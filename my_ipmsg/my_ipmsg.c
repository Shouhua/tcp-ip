/*
info:
    可以简单的实现登录, 对话, 但是还有reply的部分格式没有分解(reply >)
Author: Shouhua Peng
Date: 2/8/2015
version: 0.2
Tools: wiresharp, gcc, ipmsg(win and latest version), virtualbox and so on
 */
#include "my_ipmsg.h"

void printe(const char * info)
{
    perror(info);
    exit(EXIT_FAILURE);
}

void free_comm(struct command *comm)
{
    if(comm != NULL)
    {
        free(comm->s_name);    
        free(comm->s_host);
        free(comm->additional);
        free(comm);
        comm = NULL;
    }
}

void init_msock()
{
    int result;
    msock = socket(AF_INET, SOCK_DGRAM, 0);
    if(msock < 0)
        printe("msock socket failed");

    result = setsockopt(msock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
    if(result < 0)
        printe("msock setsockopt failed");

    struct sockaddr_in maddr;
    memset(&maddr, 0, sizeof(struct sockaddr_in));
    maddr.sin_family = AF_INET;
    maddr.sin_port = htons(IPMSG_PORT);
    maddr.sin_addr.s_addr = htonl(INADDR_ANY);

    result = bind(msock, (struct sockaddr *)&maddr, (socklen_t)sizeof(maddr));
    if(result < 0)
        printe("msock bind failed");

    //printf("init_msock success\n");
}

struct command *new_comm()
{
    struct command *comm = (struct command *)malloc(sizeof(struct command));
    if(comm == NULL)
        return NULL;
    comm->s_name = (char *)malloc(NAME_LEN*sizeof(char));
    if(comm->s_name == NULL)
    {
        free(comm);
        return NULL;
    }
    comm->s_host = (char *)malloc(NAME_LEN*sizeof(char));
    if(comm->s_host == NULL)
    {
        free(comm->s_name);
        free(comm);
        return NULL;
    }
    comm->additional = (char *)malloc(MSG_LEN*sizeof(char));
    if(comm->additional == NULL)
    {
        free(comm->s_name);
        free(comm->s_host);
        free(comm);
        return NULL;
    }
    memset(comm, 0, sizeof(comm));
    return comm;
}

void login()
{
    int msg_len;

    struct command *comm = new_comm();
    if(comm == NULL)
        printe("new_comm fialed");
    memset(comm, 0, sizeof(comm));

    //msg_len = snprintf(buff, COMM_LEN, "%d:%d:%s:%s:%u:%s", 
    //    IPMSG_VERSION, (size_t)time(NULL), NAME, HOST, (unsigned int)IPMSG_BR_ENTRY, "");

    struct sockaddr_in baddr;
    baddr.sin_family = AF_INET;
    baddr.sin_port = htons(IPMSG_PORT);
    baddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    comm->comm_no = (unsigned int)IPMSG_BR_ENTRY;
    comm->peer = baddr;
    
    comm2msg(comm);

    if(send_msg() > 0)
    {
        free_comm(comm);
        //printf("login success\n");
    }
    else
    {
        free_comm(comm);
        printe("login failed");
    }

    //int result;
    //result = sendto(msock, buff, msg_len, 0, (struct sockaddr *)&baddr, sizeof(baddr));
    //if(result < 0)
    //    printe("msock sendto failed"); 
    //printf("login success\n");
}

void send_broadcast(){}
void send_file(){}
int recv_msg()
{
    struct sockaddr_in src;
    int src_len = sizeof(src);
    memset(&src, 0, src_len);

    //char buff[COMM_LEN];

    int result;
    result = recvfrom(msock, buff, COMM_LEN, 0, (struct sockaddr *)&src, (socklen_t *)&src_len);
    if(result < 0)
        printe("recvfrom failed");
    
    peer = src;
    
    //printf("recv_msg success\n");
    return result;
}
/*
void parse_msg(struct command *comm)
{
    char *msg = buff;
    if((atoi(strtok(msg, IPMSG_MSG_TOKEN))) != IPMSG_VERSION)
    {
        fprintf(stderr, "ill legal version in ipmsg protocol\n");
        exit(EXIT_FAILURE);
    }
    comm->version = 1;
    comm->packet_no = atoi(strtok(NULL, IPMSG_MSG_TOKEN));
    strncpy(comm->s_name, strtok(NULL, IPMSG_MSG_TOKEN), NAME_LEN);
    strncpy(comm->s_host, strtok(NULL, IPMSG_MSG_TOKEN), NAME_LEN);

    comm->comm_no = atoi(strtok(NULL, IPMSG_MSG_TOKEN));

    char *str = strtok(NULL, IPMSG_MSG_TOKEN); 
    if(str)
        strncpy(comm->additional, str, MSG_LEN);
    comm->peer = peer;

    printf("parse_msg success\n");
}
*/

int parse_msg(struct command *comm)
{
    if((buff == NULL) || (comm == NULL)) 
        return -1;
    
    char *msg = buff;
    char *pos = NULL;
    int index, remain;
    
    remain = strlen(buff);
    while(index < 5)
    {
        pos = memchr(msg, ':', remain);
        if(pos == NULL)
            break;
        while(*pos == ':')
        {
            *pos = '\0';
            pos++;
        }

        switch(index)
        {
            case 0:
                comm->version = (int)strtol(msg, (char **)NULL, 10);
                break;
            case 1:
                comm->packet_no = (int)strtol(msg, (char **)NULL, 10);
                break;
            case 2:
                strncpy(comm->s_name, msg, NAME_LEN);
                break;
            case 3:
                strncpy(comm->s_host, msg, NAME_LEN);
                break;
            case 4:
                comm->comm_no = (int)strtol(msg, (char **)NULL, 10);
                break;
            default:
                break;
        }

        remain -= strlen(msg) - 1;
        msg = pos;
        index++;
    }
    
    strncpy(comm->additional, msg, MSG_LEN);
    comm->peer = peer;
    //comm->additional = msg;
    //printf("parse_msg success\n");
    return 0;
}

void recv_file(){}
/*
void msg2comm(char *buf, int size, struct command * comm)
{
    int version = atoi(strtok(buf, IPMSG_MSG_TOKEN)); 
    if(version != 1)
    {
        fprintf(stderr, "ill legal version in ipmsg protocol\n");
        exit(EXIT_FAILURE);
    }
    comm->version = 1;
    comm->packet_no = atoi(strtok(NULL, IPMSG_MSG_TOKEN));

    //comm->s_name = (char *)calloc(1, NAME_LEN);
    strncpy(comm->s_name, strtok(NULL, IPMSG_MSG_TOKEN), NAME_LEN);
    //comm->s_host = (char *)calloc(1, NAME_LEN);
    strncpy(comm->s_host, strtok(NULL, IPMSG_MSG_TOKEN), NAME_LEN);

    comm->comm_no = atoi(strtok(NULL, IPMSG_MSG_TOKEN));

    //comm->additional = (char *)calloc(1, MSG_LEN);
    strncpy(comm->additional, strtok(NULL, IPMSG_MSG_TOKEN), MSG_LEN);

    printf("msg2comm success\n");
}
*/
int comm2msg(struct command *comm)
{
    int packet_no = (unsigned int)time(NULL);
    int result;

    result = snprintf(buff, COMM_LEN, "%d:%u:%s:%s:%u:%s", 
            1, packet_no, NAME, HOST, (unsigned int)comm->comm_no, comm->additional);

    peer = comm->peer;
    //printf("comm2msg success\n");
    return result;    
}

//int send_msg(struct command *comm)
int send_msg()
{
    //int len;
    int result;
    //len = comm2msg(comm);
    //result = sendto(msock, buff, len, 0, (struct sockaddr *)&(comm->peer), sizeof(comm->peer));
    result = sendto(msock, buff, strlen(buff), 0, (struct sockaddr *)&peer, sizeof(peer));
    if(result < 0)
        printe("sendto failed");
    
    //printf("send_msg success\n");
    return result;
}

void delete_newline(char *str, char c, char r)
{
    int i = 0;
    //printf("strlen(str) = %d\n", strlen(str));
    for(;i < strlen(str); i++)
    {
        if(str[i] == c)
            str[i] = r;
        
    }
}

void dispose_msock()
{
    int result;
    int from_len;
    int comm_mode;
    int comm_opt;

    struct command *comm = new_comm();
    if(comm == NULL)
        printe("new_comm failed");
    memset(comm, 0, sizeof(comm));
  
    recv_msg();

    parse_msg(comm);
    
    comm_mode = GET_MODE(comm->comm_no);
    comm_opt  = GET_OPT(comm->comm_no);
    
    char msg[MSG_LEN];
    int comm_no;

    strncpy(msg, comm->additional, MSG_LEN);
    comm_no = (unsigned int)comm->comm_no;

    if(comm_opt & IPMSG_SENDCHECKOPT)
    {
      //  printf("IPMSG_SENDCHECKOPT\n");
      //  comm->packet_no = (unsigned int)time(NULL);
      //  struct command *comm1 = new_comm();
      //  memset(comm1, 0, sizeof(*comm1));
      //  memcpy(comm1, comm, sizeof(*comm));//容易犯错误 sizeof(comm) = 4
        
        strncpy(msg, comm->additional, MSG_LEN);
        comm_no = (unsigned int)comm->comm_no;

        comm->comm_no = (unsigned int)IPMSG_RECVMSG;
        snprintf(comm->additional, MSG_LEN, "%d", comm->packet_no);
        comm2msg(comm);
        send_msg();
     //   free_comm(comm1);
    }
    
    switch(comm_mode)
    {
        case IPMSG_ANSENTRY:
            printf("get IPMSG_ANSENTRY\n");
            //Update user
            break;
        case IPMSG_BR_ENTRY:
            printf("get IPMSG_BR_ENTRY\n");
            comm->comm_no = IPMSG_ANSENTRY;
            snprintf(comm->additional, MSG_LEN, "%s", NAME);
            comm2msg(comm);
            send_msg();
            //Update user
            break;
        case IPMSG_BR_EXIT:
            printf("get IPMSG_BR_EXIT\n");
            //Update user
            break;
        case IPMSG_SENDMSG:
            delete_newline(msg, '\n', '\a');
            printf("message %s from %s\n", msg, comm->s_name);
            break;
        default:
            break;
    }
    free_comm(comm);
}

void dispose_input()
{
    //char input[1024];
    memset(buff, 0, sizeof(buff));
    fgets(buff, sizeof(buff), stdin);
    /*
    int i;
    for(i=0; i<sizeof(buff); i++)
    {
        if(buff[i] == '\n')
        {
            buff[i] = '\0';
//            printf("there have a '\\n' in index: %d\n", i);
            break;
        }

    }
  */
    delete_newline(buff, '\n', '\a');
    struct command *comm2 = new_comm();
    if(comm2 == NULL)
    {
        printe("new_comm in dispose_input failed");    
    }
    memset(comm2, 0, sizeof(comm2));

    struct sockaddr_in baddr;
    baddr.sin_family = AF_INET;
    baddr.sin_port = htons(IPMSG_PORT);
    baddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    comm2->comm_no = (unsigned int)(IPMSG_SENDMSG | IPMSG_SENDCHECKOPT);
    snprintf(comm2->additional, MSG_LEN, "%s", buff);
    comm2->peer = baddr;
    comm2msg(comm2);
    send_msg();

    //printf("dispose_input success");
    free_comm(comm2);
}

void dispatch()
{
    int fd_max;
    fd_set rsets;
    for(;;)
    {
        FD_ZERO(&rsets);
        FD_SET(msock, &rsets);
        fd_max = msock + 1;
        
        FD_SET(STDIN_FILENO, &rsets);

        result = select(fd_max, &rsets, NULL, NULL, (struct timeval *)0);
        if(result == -1)
        {
            if(errno == EINTR)
                continue;
            else
                printe("select failed");
        }

        if(FD_ISSET(msock, &rsets))
        {
            dispose_msock();    
        }

        if(FD_ISSET(STDIN_FILENO, &rsets))
        {
            //fgets(input, sizeof(input), stdin);
            dispose_input();
        }
    }
}


int main(int argc, char *argv[])
{
    init_msock();
    login();
    dispatch();
    exit(EXIT_SUCCESS);    
}
