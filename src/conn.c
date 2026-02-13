#include "conn.h"
#include "log.h"
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct conn *newconn(int fd)
{
    struct conn *c = calloc(1, sizeof(struct conn));
    if (!c)
        return NULL;

    c->fd = fd;
    c->rsize = 0;
    c->wsize = 0;
    c->wdata_sent = 0;

    return c;
}

static int cl_expand(struct clist *cl, size_t size)
{
    cl->data = reallocarray(cl->data, size, sizeof(struct conn *));
    if (cl->data == NULL) {
        return -1;
    }
    cl->size = size;

    return 0;
}

void freeconn(struct conn *c)
{
    free(c);
}

struct clist *cl_init(int init_size)
{
    struct clist *cl = calloc(sizeof(struct clist), 1);
    if (!cl) {
        return NULL;
    }
    cl->data = calloc(sizeof(struct conn *), init_size);
    if (!cl->data) {
        return NULL;
    }
    cl->size = init_size;

    return cl;
}

int cl_new_conn(struct clist *cl, int fd)
{
    if (fd > cl->size + 1) {
        info("before expand: %p", cl->data);
        if (cl_expand(cl, fd + 1) == -1) {
            perror("error on expanding connection list");
        }
        info("after expand: %p", cl->data);
    }
    struct conn *c = newconn(fd);
    if (c == NULL) {
        return -1;
    }
    cl->data[fd] = c;

    return 0;
}

int cl_remove(struct clist *cl, int fd)
{
    if (fd > cl->size) {
        errno = EINVAL;
        return -1;
    }
    freeconn(cl->data[fd]);
    cl->data[fd] = NULL;

    return 0;
}

void cl_free(struct clist *cl)
{
    if (!cl)
        return;
    for (int i = 0; i < cl->size; i++)
        if (cl->data[i] != NULL)
            freeconn(cl->data[i]);
    free(cl->data);
    free(cl);
}
