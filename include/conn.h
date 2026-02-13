#ifndef _CONN_H
#define _CONN_H

#define MAX_REQ_LEN 4092

enum state {
    STATE_REQ = 0,
    STATE_RES = 1,
    STATE_END = 2,
};

struct conn {
    int fd;
    int rsize;
    int wsize;
    char rdata[4 + MAX_REQ_LEN];
    char wdata[4 + MAX_REQ_LEN];
    int wdata_sent;
    enum state s;
};

struct conn *newconn(int fd);
void freeconn(struct conn *c);

struct clist {
    struct conn **data;
    int size;
};

struct clist *cl_init(int size);
int cl_new_conn(struct clist *l, int num);
int cl_remove(struct clist *l, int fd);
void cl_free(struct clist *l);

#endif /* ifndef _CONN_H */
