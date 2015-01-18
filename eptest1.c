/*
This file is to test epoll to monitor the click button of user, and output the string.
*/
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>

int main(int argc, char *argv[])
{
    int nread, count;
    char buffer[3];
    int epfd, nfds;
    struct epoll_event ev, events[5];
    epfd = epoll_create(1);
    ev.data.fd = STDIN_FILENO;
    ev.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);
    for(;;)
    {
        nfds = epoll_wait(epfd, events, 5, -1);
        for(int i=0; i<nfds; i++)
        {
            if(events[i].data.fd == STDIN_FILENO)
                {
                    ioctl(0, FIONREAD, nread);
                    if(nread == 0)
                        exit(0);
                    while(nread > 0)
                    {
                        count = read(0, buffer, 3);
                        if(count < 0 && errno != EAGAIN)
                            {
                                perror("read error!\n");
                                break;
                            }
                        printf("%s", buffer);
                        memset(&buffer, 0, sizeof(buffer));
                        nread = nread - 3;
                    }
                }
        }
    }
    return 0;    
}
