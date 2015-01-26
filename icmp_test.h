#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h> //struct addrinfo
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>


#define BUFSIZE 1500

char sendbuf[BUFSIZE];

int datalen;
char *host;
int nsent;
pid_t pid;
int sockfd;
int verbose;

void init_v6(void);
void proc_v4(char *, ssize_t, struct msghdr *, struct timeval *);
void proc_v6(char *, ssize_t, struct msghdr *, struct timeval *);
void send_v4(void);
void send_v6(void);
void readloop(void);
void sig_alarm(int);
void tv_sub(struct timeval *, struct timeval *);

struct proto{
    void (*fproc)(char *, ssize_t, struct msghdr *, struct timeval *);
    void (*fsend)(void);
    void (*finit)(void);
    struct sockaddr *sasend;
    struct sockaddr *sarecv;
    socklen_t salen;
    int icmpproto;
} *pr;

#ifdef IPV6

#include <netinet/ip6.h>
#include <netinet/icmp6.h>

#endif
