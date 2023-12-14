#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

void error(const char *msg){
    perror(msg);
    exit(1);
    /*The exit (0) shows the successful 
    termination of the program and the 
    exit(1) shows the abnormal termination 
    of the program.*/
}

#define SERVER_PORT 12345

int main(int argc , char *argv[]){
    /*File Description (FD) Entry 
    (or Sort Description (SD) */
    int sockfd, newsockfd ; /*Socket descriptors*/
    int clilen;
    struct sockaddr_in serv_addr,cli_addr;

    int i;
    char buf[100]; /*message buffer*/

    /*************************************************/
    /* Create an AF_INET stream socket to receive   */
    /* incoming connections on                       */
    /*************************************************/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0)
        error("ERROR opening socket");
    
    /*************************************************/
    /* Bind the socket                               */
    /*************************************************/
    serv_addr.sin_family = AF_INET; // Socket INternet
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(20000);// 	host to network short // to convert an IP port number in host byte order to the IP port number in network byte order.
    
    if(bind(sockfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){
        error("Unable to bind local address");
    }
    
    /*************************************************/
    /* Set the listen back log                       */
    /*************************************************/
    if(listen(sockfd,5)<0){
        close(sockfd);
        error("Listen failed");
    }

    printf("The Server is ready\n");

    while (1){
        /* The accept() system call accepts a client connection.
            It blocks server untill a client request comes.
        */
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd,(struct sockfd *)&cli_addr,&clilen);
        if(newsockfd<0){
            error("Accept failed");
        }
        /*We initialize the buffer , copy the message to it, and send the message to the client.*/
        strcpy(buf,"Message from server");
        send(newsockfd,buf,strlen(buf)+1,0);
        recv(newsockfd,buf,100,0);
        printf("Message from client: %s\n",buf);
        close(newsockfd);
    }
    return 0;
}