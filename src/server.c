#include "check.h"
#include "command.h"
#include "conn.h"
#include "estring.h"
#include "hashtable.h"
#include "log.h"
#include "plist.h"
#include "serialize.h"
#include "slist.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_ARGS_COUNT 3
#define LEN_SIZE 4
#define NSTR_SIZE 4
#define STR_LEN_SIZE 4
#define RES_CODE_SIZE 4

__auto_type db = (struct HashTable){};

static volatile sig_atomic_t server_up = 1;

static void handlesig(int sig)
{
    (void)sig;
    server_up = 0;
}

void setfdnb(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) == -1)
        error("fcntl set fd status flags");
}

int sendresp(struct conn *c)
{
    size_t remain;
    ssize_t nwrite;
    while (c->wdata_sent < c->wsize) {
        remain = c->wsize - c->wdata_sent;
        nwrite = send(c->fd, &c->wdata[c->wdata_sent], remain, 0);
        if (nwrite == -1 && errno == EINTR) {
            continue;
        }
        if (nwrite == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            return -1;
        }
        if (nwrite == -1) {
            error("failed to read from connection");
            c->s = STATE_END;
            return -1;
        }
        if (nwrite == 0) {
            continue;
        }
        c->wdata_sent += nwrite;
    }
    c->s = STATE_REQ;
    c->wdata_sent = 0;
    c->wsize = 0;

    return 0;
}

void run(struct slist *cmd, int *rescode, string_t resp)
{
    if (memcmp(cmd->data[0]->data, "get", 3) == 0 && cmd->len == 2) {
        *rescode = get(&db, cmd->data[1], resp);
    }
    else if (memcmp(cmd->data[0]->data, "set", 3) == 0 && cmd->len == 3) {
        *rescode = set(&db, cmd->data[1], cmd->data[2], resp);
    }
    else if (memcmp(cmd->data[0]->data, "del", 3) == 0 && cmd->len == 2) {
        *rescode = del(&db, cmd->data[1], resp);
    }
    else if (memcmp(cmd->data[0]->data, "keys", 4) == 0 && cmd->len == 1) {
        *rescode = keys(&db, resp);
    }
    else {
        char *msg = "invalid command dumbass";
        werr(resp, ERR_UNKONWN, msg, strlen(msg));
        return;
    }

    // TODO: writing error message must be handled by the commands
    if (*rescode == RES_ERR) {
        char *msg = "failed to run the command";
        werr(resp, RES_ERR, msg, strlen(msg));
    }
}

// rdata consists of: [4 bytes of nargs] [4 bytes of str n length] [str n]
// wdata only contains the response data while [4 bytes of response len] and
// [4 bytes of response code] is set by the caller.
// So the `wlen` shows the len of response regardless the len of response code
int do_request(char *rdata, int rlen, int *rescode, string_t resp)
{
    int n;
    memcpy(&n, rdata, NSTR_SIZE);
    if (n > MAX_ARGS_COUNT) {
        return -1;
    }
    int slen;
    int pos = NSTR_SIZE;
    struct slist *cmd = new_slist(NULL, 0);
    // 1. read each str of command
    // 2. add the string to slist
    // 3. check if it's a valid command
    // 4. do something according to each command
    while (n--) {
        if (pos + STR_LEN_SIZE > rlen) {
            goto ERR;
        }
        memcpy(&slen, &rdata[pos], STR_LEN_SIZE);
        if (pos + STR_LEN_SIZE + slen > rlen) {
            goto ERR;
        }
        struct string *s = str_init();
        str_set(s, &rdata[pos + STR_LEN_SIZE], slen);
        pos += STR_LEN_SIZE + slen;
        slist_append(cmd, s);
    }
    if (pos != rlen) {
        goto ERR;
    }

    run(cmd, rescode, resp);
    slist_free(cmd);
    return 0;

ERR:
    slist_free(cmd);
    return -1;
}

int extractreq(struct conn *c)
{
    int cmdlen;
    while (c->rsize >= LEN_SIZE) {
        memcpy(&cmdlen, c->rdata, LEN_SIZE);
        if (cmdlen == 0) {
            warn("cmdlen = 0 -> close connection");
            c->s = STATE_END;
            return 0;
        }
        if (cmdlen > MAX_REQ_LEN) {
            error("invalid command length: %d", cmdlen);
            c->s = STATE_END;
            return -1;
        }
        if (c->rsize - LEN_SIZE < cmdlen) {
            // Didn't read enough data
            return 0;
        }

        int rescode;
        int wlen = 0;
        string_t resp = str_init();

        // Request is ready to parse
        int err = do_request(&c->rdata[LEN_SIZE], cmdlen, &rescode, resp);
        if (err) {
            c->s = STATE_END;

            return -1;
        }
        if (resp->len + LEN_SIZE + RES_CODE_SIZE > MAX_REQ_LEN) {
            char *msg = "response too large";
            werr(resp, ERR_TOOBIG, msg, strlen(msg));
        }
        wlen += resp->len;
        wlen += RES_CODE_SIZE; // rescode

        memcpy(c->wdata, &wlen, LEN_SIZE);
        memcpy(&c->wdata[LEN_SIZE], &rescode, RES_CODE_SIZE);
        memcpy(&c->wdata[LEN_SIZE + RES_CODE_SIZE], resp->data, resp->len);
        c->wsize = LEN_SIZE + wlen;

        int remaining = c->rsize - (LEN_SIZE + cmdlen);
        if (remaining) {
            memmove(c->rdata, &c->rdata[LEN_SIZE + cmdlen], remaining);
        }
        c->rsize = remaining;
        c->s = STATE_RES;

        str_free(resp);

        if (sendresp(c) == -1) {
            return -1;
        }
        c->s = STATE_REQ;
    }

    return 0;
}

// It's assumed that user has requested longest valid command, i.e. 4094 bytes
// In this case, it's tried to read 4 + 4096 bytes and parse as many as commands available
int handlereq(struct conn *c)
{
    ssize_t nread;
    while (c->s == STATE_REQ) {
        nread = recv(c->fd, &c->rdata[c->rsize], (sizeof(c->rdata) - c->rsize), 0);
        if (nread == -1 && errno == EINTR) {
            continue;
        }
        if (nread == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            info("nothing to read from connection");
            c->s = STATE_END;
            break;
        }
        if (nread == -1) {
            error("failed to read from connection");
            c->s = STATE_END;
            return -1;
        }
        if (nread == 0) {
            c->s = STATE_END;
            break;
        }
        c->rsize = nread;

        if (extractreq(c) == -1) {
            return -1;
        }
    }

    return 0;
}

int acceptconn(int fd, struct plist *pl, struct clist *cl)
{
    struct sockaddr_in addr;
    int addr_len = sizeof(struct sockaddr_in);
    int cfd = accept(fd, (struct sockaddr *)&addr, (socklen_t *)&addr_len);
    if (cfd < 0) {
        return -1;
    }

    setfdnb(cfd);

    char host[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, (void *)&addr.sin_addr.s_addr, host, INET_ADDRSTRLEN);
    info("Client connected [FD: %d, Address: %s, Port: %d]", cfd, host, ntohs(addr.sin_port));

    struct pollfd clientpoll = (struct pollfd){.fd = cfd, .events = POLLIN | POLLHUP};
    pl_append(pl, clientpoll);

    if (cl_new_conn(cl, cfd) == -1) {
        return -1;
    }

    return 0;
}

int conn_io(struct conn *c, int revents)
{
    if (revents & POLLIN) {
        return handlereq(c);
    }
    else if (revents & POLLOUT) {
        return sendresp(c);
    }
    else {
        error("invalid revents value to handle: %x", revents);
        return -1;
    }

    return 0;
}

int server_run()
{
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handlesig;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    check(sfd >= 0, "failed to create a socket");

    int optval = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8081);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int result = bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    check(result != -1, "failed to bind the address");

    setfdnb(sfd);

    listen(sfd, 10);

    struct plist *pl = pl_init(2);
    check(pl != NULL, "failed to init poll list");

    info("Serving TCP server on 127.0.0.1:8081");

    char bufin[1024] = {0};
    ssize_t nreadin;
    struct pollfd pt;

    struct clist *cl = cl_init(10);
    check(cl != NULL, "failed to init connection list");

    while (server_up) {
        pl_clear(pl);

        struct pollfd pfd = {.fd = STDIN_FILENO, .events = POLLIN};
        pl_append(pl, pfd);
        pfd = (struct pollfd){.fd = sfd, .events = POLLIN};
        pl_append(pl, pfd);

        for (int i = 0; i < cl->size; i++) {
            struct conn *c = cl->data[i];
            if (c == NULL)
                continue;
            struct pollfd pfd = {};
            pfd.fd = c->fd;
            pfd.events = c->s == STATE_REQ ? POLLIN : POLLOUT;
            pl_append(pl, pfd);
        }

        result = poll(pl->data, pl->len, -1);
        if (result == -1 && errno == EINTR) {
            continue;
        }
        if (result == -1) {
            perror("failed to listen for poll events");
            return EXIT_FAILURE;
        }

        info("Start reading POLLS");

        // User input
        if (pl->data[0].revents & POLLIN) {
            nreadin = read(STDIN_FILENO, bufin, 1023);
            char *nl = strchr(bufin, '\n');
            if (nl) *nl = '\0';
            info("user input: %.*s", (int)nreadin, bufin);
        }

        // Client connection
        if (pl->data[1].revents & POLLIN) {
            int cfd = acceptconn(sfd, pl, cl);
            if (cfd == -1) {
                error("failed to accept client connection");
            }
        }

        // Client requests
        for (size_t i = 2; i < pl->len; i++) {
            pt = pl->data[i];
            if (pt.revents & POLLHUP) {
                warn("closing connection for client fd: %d", pt.fd);
                close(pt.fd);
                cl_remove(cl, pt.fd);
                break;
            }
            if (pt.revents) {
                struct conn *c = cl->data[pt.fd];
                if (conn_io(c, pt.revents) == -1) {
                    error("error on handling request");
                }
                if (c->s == STATE_END) {
                    close(pt.fd);
                    cl_remove(cl, pt.fd);
                }
            }
        }
    }

    close(sfd);

    cl_free(cl);
    pl_free(pl);

    ht_clear(&db);

    return EXIT_SUCCESS;
}
