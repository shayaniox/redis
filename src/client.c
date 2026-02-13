#include "client.h"
#include "check.h"
#include "log.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int client_run(int argc, char *argv[])
{
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    check(sfd != -1, "failed to create socket");

    unsigned int ip = 0;
    ip |= 127 << 24;
    ip |= 0 << 16;
    ip |= 0 << 8;
    ip |= 1 << 0;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8081);
    addr.sin_addr.s_addr = htonl(ip);

    char addrbuf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, (void *)&addr.sin_addr.s_addr, addrbuf, INET_ADDRSTRLEN);

    info("Connecting to: %s:%d", addrbuf, ntohs(addr.sin_port));

    int result = connect(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    check(result != -1, "failed to connect to address");

    // command len: 4 bytes
    // n str: 4 bytes
    // str len: 4 bytes
    // actual string: str len value

    size_t totallen = 4; // strn size
    for (int i = 0; i < argc; i++) {
        totallen += 4;
        totallen += strlen(argv[i]);
    }

    char *req = malloc(4 + totallen + 4); // 4 bytes for cmdlen = 0
    memcpy(&req[0], &totallen, 4);
    memcpy(&req[4], &argc, 4);

    int pos = 4 + 4;
    int len;
    for (int i = 0; i < argc; i++) {
        len = strlen(argv[i]);
        memcpy(&req[pos], &len, 4);
        memcpy(&req[pos + 4], argv[i], len);
        pos += 4 + len;
    }

    // cmdlen = 0
    *(int *)&req[4 + totallen] = 0;

    result = send(sfd, req, 4 + totallen + 4, 0);
    check(result != -1, "failed to send request");

    free(req);

    ssize_t numread;
    int buflen = 1024;
    char buf[buflen];

    while (1) {
        numread = recv(sfd, buf, buflen, 0);
        if (numread == -1) {
            error("failed to receive the server response");
            break;
        }
        if (numread == 0) {
            info("connection closed");
            break;
        }
        info("Numread: %ld", numread);
        info("Command len: %d", *(int *)buf - 4);
        info("Result code: %d", *(int *)&buf[4]);
        info("Received: {%.*s}", (int)numread - (4 + 4), &buf[4 + 4]);
    }

    close(sfd);

    return EXIT_SUCCESS;
}
