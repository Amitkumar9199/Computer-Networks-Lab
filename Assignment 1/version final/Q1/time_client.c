/*Important Library*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

/*Function to give error msg to user, and exit*/
void error(const char *msg){
    perror(msg);
    exit(1);
}                
            
                    /*Client Process*/
int main(){

    int sockfd; /*socket descriptors*/
    struct sockaddr_in serv_addr;
    
    int i;
    char buf[100]; /*message buffer*/
    
    /* the following syatem call opens a socket.*/
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        /*Error Handelling*/
        error("Unable to create socket");
    }

    /* defining structure sockaddr_in for the internet
        family of protocols. */
    serv_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1",&serv_addr.sin_addr);
    serv_addr.sin_port = htons(20000);

    /* With information in serv_addr,connect() system call
        establishes a connection with  server process.*/
    if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){
        /*Error Handelling*/
        close(sockfd);
        error("Unable to connect to the server");
    }
    
    /* After connection , client can send 
        or receive messages.*/
    /*Receive Date and Time from server.*/
    for(int i=0;i<100;i++)buf[i]='\0';
    if(recv(sockfd,buf,100,0)<=0){
        /*Error Handelling*/
        close(sockfd);
        error("Unable to recieve data from server");
    }
    
    /*message to be displayed on client side*/
    printf("Date and Time : %s\n",buf);

    /* the following system call closes the socket.*/
    close(sockfd);
    return 0;
}
