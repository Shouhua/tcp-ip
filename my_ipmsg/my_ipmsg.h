
#ifndef _MY_IPMSG_H_
#define _MY_IPMSG_H_


#ifdef _cplusplus
extern "C" {
#endif


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>
#include "ipmsg.h"
#include <sys/select.h>
#include <sys/types.h>

#define NAME_LEN     (50)
#define MSG_LEN      (1000)
#define COMM_LEN     (1500)
#define IPMSG_MSG_TOKEN ":"

#define NAME "Shouhua"
#define HOST "192.168.31.164"

struct command {
    int    version;
    int    packet_no;
    char   *s_name;
    char   *s_host;
    int    comm_no;
    char   *additional;
    struct sockaddr_in peer;
//    struct command *next;
};

struct user {
    char *name;
    char *host;
    char *group;
    char *nname;   //nick name
    struct sockaddr_in addr;
    int online;
    struct user *next;
};

int msock;
int fsock;

const int on = 1;
char buff[COMM_LEN];
struct sockaddr_in peer;
int result;

void printe(const char *);
void init_msock();
void init_fsock();
void login();
void dispatch();






#ifdef _cplusplus
}
#endif
#endif
