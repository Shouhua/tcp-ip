/*
    This file is test the ipmsg protocol and this is a draft, i'll update later...
this version is mainly to test login and receive reply from another endian.
    Author: Shouhua Peng
    Date: 3/2/2015
    version: 0.1
    Tools: wiresharp, gcc, ipmsg(win and latest version), virtualbox and so on
*/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/time.h>
#include "ipmsg.h"
#include <string.h>

#define DEFAULT_PORT (2425)
#define NAMELEN      (50)
#define MSGLEN       (1000)
#define COMMAND_LEN  (1500)

typedef struct command
{
    size_t version;
    size_t packet_no;
    char   sender_name[NAMELEN];
    char   sender_host[NAMELEN];
    size_t command_no;
    char   additional[MSGLEN];
    struct sockaddr_in peer;
    struct command *next;
}COMMAND;

int login()
{
    int udp_sock;
    const int on = 1;
    struct sockaddr_in me;
    char buf[COMMAND_LEN];

    if((udp_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    me.sin_family = AF_INET;
    me.sin_port = htons(DEFAULT_PORT);
    me.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if(setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0)
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    if(bind(udp_sock, (struct sockaddr *)&me, sizeof(me)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
/*
    COMMAND com;
    com.packet_no = (size_t)time(NULL);
    com.version = 1;
    com.sender_name = "shouhua";
    com.sender_host = "192.168.31.164";
    com.command_no = IPMSG_BR_ENTRY;
*/  
    int msg_len;
    msg_len = snprintf(buf, sizeof(buf), "%d:%d:%s:%s:%d:%s", IPMSG_VERSION, (size_t)time(NULL), "shouhua", "192.168.31.164", IPMSG_BR_ENTRY, "");
    
    struct sockaddr_in to;
    to.sin_family = AF_INET;
    to.sin_port = htons(DEFAULT_PORT);
    to.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    int result;
    result = sendto(udp_sock, buf, strlen(buf), 0, (struct sockaddr *)&to, sizeof(to));
    if(result < 0)
    {
        printf("result = %d\n", result);
        perror("sendto failed");
        exit(EXIT_FAILURE);
    }
    
    printf("result = %d\n", result);
    
    
    int len = sizeof(to);
    while(1)
    {
        result = recvfrom(udp_sock, buf, sizeof(buf), 0, (struct sockaddr *)&to, (socklen_t *)&len);
        if(result < 0)
        {
            perror("recvfrom failed");
            exit(EXIT_FAILURE);
        }
        printf("result = %d\n", result);
        printf("buf = %s\n", buf);
    }
}


int main(int argc, char *argv[])
{
    
    login();

    exit(EXIT_SUCCESS);    
}
