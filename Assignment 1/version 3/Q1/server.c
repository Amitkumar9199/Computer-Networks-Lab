#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<time.h>


void error(const char *msg){
    perror(msg);
    exit(1);
}
            /*Server Process*/
int main(){
    int sockfd, newsockfd;/*Socket Descriptors*/
    int clilen;
    struct sockaddr_in serv_addr,cli_addr;
    int i;
    char buf[100]; /*Communication message*/
    time_t ticks;

    /* The following system call opens a socket. 
        family of protocol : AF_INET
        :SOCK_STREAM
        user application :0
    */
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        error("Cannot create socket");
    }
    /* The structure sockaddr_in is defined in <netinet/in.h> for the
        internet family of protocols. This has 3 main fields.
        sin_family : family (AF_INET for the internet family)
        sin_addr : INADDR_ANY(internet address of the server)
        sin_port : port number of server
        
        sin means socket internet
        INADDR_ANY is used for machines having single IP Address
        htons : host to network short : to convert an IPport number in host byte order to IP Port number in networkbyte order.
        */
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(20000);

    /* With the information provided in serv_addr , we cassociate
       the server with its port using the bind() system call
    */
    if(bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){
        error("Unable to bind local address");
    }
    /* The listen system call allows the process to listen on the socket for connections.
        The first argument is the socket file descriptor, and the second is the size of the backlog queue, i.e., the number of connections that can be waiting while the process is handling a particular connection. This should be set to 5, the maximum size permitted by most systems.
    */
    if(listen(sockfd,5)<0){
        close(sockfd);
        error("Listen failed");
    }
    printf("The Server is ready\n");
    /*In this program we are making an iterative server -- 
      one which handles client connections one by one. 
      i.e., no concurrency. The accept() system call returns 
      a new socket descriptor which is used for communication
      with the server. After the communication is over , the 
      process comes back to wait again on the original socket
      descriptor.
    */
    while(1){
        /* The accepth call accepts the client connection.
           It blocks the server until a client request comes.
           
           The accept() system call fills up the client's details
           in a struct sockaddr which is passed as a parameter.
           The length of the structure is noted in clilen. Note 
           that the new socket descriptor returned by the 
           accept() system call is stored in "newsocketfd"
        */
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd,(struct sockaddr *)&cli_addr,&clilen);
    
        if(newsockfd<0){
            close(sockfd);
            error("Accept failed");
        }
        /* We initialize the buffer , copy the message to it
           , and send the message to the client.
        */
        
        // strcpy(buf,"Message from server");
        ticks = time(NULL);
        strcpy(buf,ctime(&ticks));
        if(send(newsockfd,buf,strlen(buf)+1,0)!=strlen(buf)+1){
            close(newsockfd);
            close(sockfd);
            error("send() sent a different number of bytes than expected");
        }
        /* We now receive a message from the client.
          There has to be some mechanism for the receiving
          side to knnow when the entire message
          is received. Look up the return value of recv() 
          to see how you can do this.
        */
        if(recv(newsockfd,buf,100,0)<=0){
            close(newsockfd);
            close(sockfd);
            error("receive failed");
        };
        printf("Message from client : %s\n",buf);
        close(newsockfd);
    }
    close(sockfd);
    return 0;
}