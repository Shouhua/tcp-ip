/*
This file is to test icmp protocol by using raw socket.
icmp protocol draft draw:
--------------------------------------------------------------
|    type(8bits)  |   code(8bits) |       checksum(16bits)   |    
--------------------------------------------------------------
|    identification(16bits)       |       sequence(16bits)   |
--------------------------------------------------------------
|                       options                              |
--------------------------------------------------------------

*/
#include "icmp_test.h"

struct proto proto_v4 = 
    { proc_v4, send_v4, NULL, NULL, NULL, 0, IPPROTO_ICMP };
#ifdef IPV6
struct proto proto_v6 = 
    { proc_v6, send_v6, init_v6, NULL, NULL, 0, IPPROTO_ICMPV6 };
#endif

struct addrinfo *host_serv(const char *hostname, const char *service, int family, int socktype)
{
    int n;
    struct addrinfo hints, *res;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_flags = AI_CANONNAME;
    hints.ai_family = family;
    hints.ai_socktype = socktype;
    
    if((n=getaddrinfo(host, service, &hints, &res)) != 0)
        return(NULL);
    return res;
}

char *sock_ntop_host(const struct sockaddr *sa, socklen_t salen)
{
    static char str[128];
    
    struct sockaddr_in *sin = (struct sockaddr_in *)sa;
    if((inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str))) == NULL)
        return NULL;
    return str;
}

void tv_sub(struct timeval * out, struct timeval *in)
{
    if(out->tv_usec -= in->tv_usec < 0)
    {
        out->tv_sec -= 1;
        out->tv_usec +=1000000;
    }
    out->tv_sec -= in->tv_sec;
}

void sig_alrm(int signo)
{
    (*pr->fsend)();

    alarm(1);
    return;
}

uint16_t in_cksum(uint16_t *addr, int len)
{
    int nleft = len;
    uint32_t sum = 0;
    uint16_t *w = addr;
    uint16_t answer = 0;

    while(nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    if(nleft == 1)
    {
        *(unsigned char *)(&answer) = *(unsigned char *)w;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return(answer);
}


void send_v4(void)
{
    int len;
    struct icmp *icmp;

    icmp = (struct icmp *)sendbuf;
    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = 0;
    icmp->icmp_id = pid;
    icmp->icmp_seq = nsent++;
    memset(icmp->icmp_data, 0xa5, datalen);
    gettimeofday((struct icmp *)icmp->icmp_data, NULL);

    len = 8 + datalen;
    icmp->icmp_cksum = 0;
    icmp->icmp_cksum = in_cksum((u_short *)icmp, len);
    
    sendto(sockfd, sendbuf, len, 0, pr->sasend, pr->salen);
}

void proc_v4(char *ptr, ssize_t len, struct msghdr *msg, struct timeval *tvrecv)
{
    int hlen1, icmplen;
    double rtt;
    struct ip *ip;
    struct icmp *icmp;
    struct timeval *tvsend;
    
    ip = (struct ip *)ptr;
    hlen1 = ip->ip_hl << 2;
    if(ip->ip_p != IPPROTO_ICMP)
        return;

    icmp = (struct icmp *)(ptr + hlen1);
    if((icmplen = len - hlen1) < 8)
        return;
    if(icmp->icmp_type == ICMP_ECHOREPLY)
    {
        if(icmp->icmp_id != pid)
            return;
        if(icmplen < 16)
            return;
        
        tvsend = (struct timeval *)icmp->icmp_data;
        tv_sub(tvrecv, tvsend);
        rtt = tvrecv->tv_sec * 1000.0 + tvrecv->tv_usec/1000.0;
        
        printf("%d bytes from %s: seq=%u, ttl=%d, rtt=%.3f ms\n",
            icmplen, sock_ntop_host(pr->sarecv, pr->salen), icmp->icmp_seq, ip->ip_ttl, rtt);
    }
    else if(verbose)
        printf("  %d bytes from %s: type = %d, code = %d\n",
            icmplen, sock_ntop_host(pr->sarecv, pr->salen), icmp->icmp_type, icmp->icmp_code);
}

void readloop(void)
{
    int size;
    char recvbuf[BUFSIZE];
    char controlbuf[BUFSIZE];
    struct msghdr msg;
    struct iovec iov;
    ssize_t n;
    struct timeval tval;

    sockfd = socket(pr->sasend->sa_family, SOCK_RAW, pr->icmpproto);
    if(sockfd < 0)
    {
        perror("socket error");
        exit(EXIT_FAILURE);
    }
    setuid(getuid());
    
    size = 60*1024;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));

    kill(getpid(), SIGALRM);

    iov.iov_base = recvbuf;
    iov.iov_len = sizeof(recvbuf);
    msg.msg_name = pr->sarecv;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = controlbuf;
    for(;;)
    {
        msg.msg_namelen = pr->salen;
        msg.msg_controllen = sizeof(controlbuf);
        n = recvmsg(sockfd, &msg, 0);
        if(n < 0)
        {
            if(errno == EINTR)
                continue;
            else
                {
                    fprintf(stderr, "recvmsg error\n");
                    exit(EXIT_FAILURE);
                }
        }
        
        gettimeofday(&tval, NULL);
        (*pr->fproc)(recvbuf, n, &msg, &tval);
    }
}

int datalen = 56;

int main(int argc, char *argv[])
{
    int c;
    struct addrinfo *ai;
    char *h;
    opterr = 0;
    while((c = getopt(argc, argv, "v")) != -1)
    {
        switch(c)
        {
            case 'v':
                verbose++;
                break;
            case '?':
                fprintf(stderr, "unrecognized option: %c\n", c);
                exit(-1);
        }
    }

    if(optind != argc - 1)
    {
        fprintf(stderr, "usage: ping [ -v ] <hostname>\n");
        exit(-1);
    }

    host = argv[optind];

    pid = getpid() && 0xffff;
    signal(SIGALRM, sig_alrm);
    
    ai = host_serv(host, NULL, 0, 0);

    h = sock_ntop_host(ai->ai_addr, ai->ai_addrlen);

    printf("PING %s (%s): %d data bytes\n", 
        ai->ai_canonname ? ai->ai_canonname : h, h, datalen);
    if(ai->ai_family == AF_INET)
    {
        pr = &proto_v4;    
    }
    else if(ai->ai_family == AF_INET6)
    {
        printf("ipv6 not support temporarily...\n");    
    }
    else
    {
        fprintf(stderr, "unknown address family %d", ai->ai_family);
        exit(EXIT_FAILURE);
    }

    pr->sasend = ai->ai_addr;
    pr->sarecv = calloc(1, ai->ai_addrlen);
    pr->salen = ai->ai_addrlen;

    readloop();

    exit(0);    
}
