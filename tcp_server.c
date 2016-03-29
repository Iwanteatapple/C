/*
** server.c -- a stream socket server demo
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#define PORT "3490" // the port users will be connecting to
#define BACKLOG 10 // how many pending connections queue will hold
#define MAXDATASIZE 258

char sendTable[MAXDATASIZE];

void sigchld_handler(int s)
{
while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
if (sa->sa_family == AF_INET) {
return &(((struct sockaddr_in*)sa)->sin_addr);
}
return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    fd_set fds,newfds; 
    
    int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv ,i,j;
    int fdmax;
    int numbytes;
   
    FD_ZERO(&fds); //每次循环都要清空集合，否则不能检测描述符变化
    FD_ZERO(&newfds); //每次循环都要清空集合，否则不能检测描述符变化
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
    }
    
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
        p->ai_protocol)) == -1) {
        perror("server: socket");
        continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
        sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        close(sockfd);
        perror("server: bind");
        continue;
        }
        break;
    }
    
    if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    return 2;
    }
    
    freeaddrinfo(servinfo); // all done with this structure
    if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
    }
    
    FD_SET(sockfd,&fds); //添加描述符
    fdmax = sockfd;
   
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
    }
    
    
    
    printf("server: waiting for connections...\n");
    
    
    while(1) 
    { // main accept() loop
        newfds = fds;
        
        if(select(fdmax+1,&newfds,NULL,NULL,NULL)==-1)   //select使用
        {
            perror("select");
            exit(4);
        }// end switch 
        
        for(i=0;i <= fdmax ;i++)
        {
            
            if (FD_ISSET(i, &newfds)) { // 我們找到一個！！
                if (i == sockfd) {
                // handle new connections
                    sin_size = sizeof their_addr;
                    new_fd = accept(sockfd,
                    (struct sockaddr *)&their_addr,
                    &sin_size);

                    if (new_fd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(new_fd, &fds); // 新增到 master set
                        if (new_fd > fdmax) { // 持續追蹤最大的 fd
                            fdmax = new_fd;
                        }
                      printf("selectserver: new connection from %s on "
                      "socket %d\n",
                      inet_ntop(their_addr.ss_family,
                        get_in_addr((struct sockaddr*)&their_addr),
                        s, INET6_ADDRSTRLEN),
                        new_fd);
                    }
                }
                else{
                    // 處理來自 client 的資料
                    if ((numbytes = recv(i, sendTable, MAXDATASIZE, 0)) <= 0) {
                    // got error or connection closed by client
                        if (numbytes == 0) {
                          // 關閉連線
                          printf("selectserver: socket %d hung up\n", i);
                        } else {
                          perror("recv");
                        }
                    close(i); // bye!
                    FD_CLR(i, &fds); // 從 master set 中移除

                    } else {
                        // 我們從 client 收到一些資料
                        for(j = 0; j <= fdmax; j++) {
                          // 送給大家！
                          if (FD_ISSET(j, &fds)) {
                            // 不用送給 listener 跟我們自己
                            if (j != sockfd && j != i) {
                              if (send(j, sendTable, MAXDATASIZE, 0) == -1) {
                                perror("send");
                              }
                            }
                          }
                        }
                    }
                }
            }
        }
    }
    
    
    return 0;
}