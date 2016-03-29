#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define PORT "3490" // the port client will be connecting to
#define MAXDATASIZE 258 // max number of bytes we can get at once
// get sockaddr, IPv4 or IPv6:

int sockfd, numbytes;
char buf[MAXDATASIZE];
struct addrinfo hints, *servinfo, *p;
int rv;
char s[INET6_ADDRSTRLEN];

//>>>>>-----------------------遊戲---------------------------
//>>>>>-----------------------遊戲---------------------------
//>>>>>-----------------------遊戲---------------------------
//>>>>>-----------------------遊戲---------------------------
//>>>>>-----------------------遊戲---------------------------



    int table[17][17];
    char row=64;
    int column=0;
    int tableR,tableC;
    int	chess=1;
    int check;
    int change=0;
    int score1=2,score2=2;
    int I,J,Z;

void sendTableToTable();
void tableTosendTable();
void printTable();
void Score();
void WGame();
    
    char sendTable[MAXDATASIZE];
    
    int size=6;

void sendTableToTable()
{
    table[0][0]=sendTable[0];
    for(I=1;I<=size;I++)
    {
        for(J=1 ;J<=size ;J++)
        {
            table[J][I]=sendTable[(I-1)*size+J];
        }
    }
}

void tableTosendTable()
{
    sendTable[0]=(char)table[0][0];
    for(I=1;I<=size;I++)
    {
        for(J=1 ;J<=size ;J++)
        {
            sendTable[(I-1)*size+J]=(char)table[J][I];
        }
    }
    
}

void printTable()
{
    printf("     ");
    
    for(I=1;I<=size;I++)
    {
        printf("%c  ",(char)(row+I));
    }
    
    printf("\n");
    
    for(I=1;I<=size;I++)
    {
        if(I<10)
        {
            printf("%d    ",column+I);
        }
        else
        {
            printf("%d   ",column+I);
        }
        for(J=1;J<=size;J++)
        {
            if(table[J][I]==1)
            {
                printf("◆  ");
            }
            else if(table[J][I]==2)
            {
                printf("◇  ");
            }
            else if(table[J][I]==0)
            {
                printf("◎  ");
            }
//            printf("%d   ",table[J][I]);
        }
        printf("\n");
    }
} 

void Score()
{
	for(I=1;I<=size;I++)
    {
        for(J=1;J<=size;J++)
        {
            if(table[I][J]==1)
            {
            	score1++;
            }
            else if(table[I][J]==2)
            {
            	score2++;
            }
        }
    }
}

int tableNumber()
{
    int k=0;
    for(I=1;I<=size;I++)
    {
        for(J=1 ;J<=size ;J++)
        {
            if(table[I][J]!=0)
            {
                k++;
            }
        }
    }
    
    return k;
}




void WGame()
{
    
    while(1)
    {
        score1=0;
        score2=0;
        if ((numbytes = recv(sockfd, sendTable, MAXDATASIZE, 0)) == -1) {
                    perror("recv");
            }
        sendTableToTable();
        Score();
        printf("\n");
        printf("玩家1分數: %d\n",score1);
        printf("玩家2分數: %d\n\n",score2);
        
        printTable();
        
        printf("目前玩家:%d",tableNumber()%2+1);
        if(tableNumber()%2+1==1)
        {
            printf("  ◆\n");
        }
        else if(tableNumber()%2+1==2)
        {
            printf("  ◇\n");
        }
        
    }
    if(score1>score2)
    {
    	printf("\n");
    	printf("玩家1:%d    玩家2:%d\n",score1,score2); 
    	printf("玩家1獲勝\n"); 
    }
    else if(score1<score2)
    {
    	printf("\n");
    	printf("玩家1:%d    玩家2:%d\n",score1,score2); 
    	printf("玩家2獲勝\n"); 
    }
    else
    {
    	printf("\n");
    	printf("玩家1:%d    玩家2:%d\n",score1,score2); 
    	printf("和局\n");
    }
     





}//end of game









void *get_in_addr(struct sockaddr *sa)
{
if (sa->sa_family == AF_INET) {
return &(((struct sockaddr_in*)sa)->sin_addr);
}
return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(int argc, char *argv[])
{
//int sockfd, numbytes;
//char buf[MAXDATASIZE];
//struct addrinfo hints, *servinfo, *p;
//int rv;
//char s[INET6_ADDRSTRLEN];
    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
        p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        close(sockfd);
        perror("client: connect");
        continue;
        }
    break;
    }
    
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
    s, sizeof s);
    printf("client: connecting to %s\n", s);
    freeaddrinfo(servinfo); // all done with this structure
    
    printf("等待觀看\n");
//    if ((numbytes = recv(sockfd, sendTable, MAXDATASIZE, 0)) == -1) {
//                    perror("recv");
//            }
    
    
    WGame();
   
    close(sockfd);
    return 0;
}

