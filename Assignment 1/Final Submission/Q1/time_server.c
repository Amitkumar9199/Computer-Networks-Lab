/*Important Library*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<time.h>

/*Function to give error msg to user, and exit*/
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
    char buf[100]; /*message buffer*/
    
    time_t ticks; /*For storing Local machine time*/

    /* The following system call opens a socket.*/
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        /*Error Handelling*/
        error("Cannot create socket");
    }
    
    /* defining structure sockaddr_in for the internet
     family of protocols. */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(20000);

    /* With the information provided in serv_addr , 
       we can associate the server with its port using 
       the bind() system call */
    if(bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){
        /*Error Handelling*/
        error("Unable to bind local address");
    }
    
    /* The listen system call allows the process to
     listen on the socket for connections.*/
    if(listen(sockfd,5)<0){
        /*Error Handelling*/
        close(sockfd);
        error("Listen failed");
    }
    
    printf("The Server is ready ... \n");
    
    /*iterative server*/
    while(1){

        /* The accepth call accepts the client connection.
           It blocks the server until a client request comes.
        */
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd,(struct sockaddr *)&cli_addr,&clilen);
        if(newsockfd<0){
            /*Error Handelling*/
            close(sockfd);
            error("Accept failed");
        }

        /* We copy date and time to buf */
        ticks = time(NULL);
        strcpy(buf,ctime(&ticks));

        /* send message(date and time) to client.*/
        if(send(newsockfd,buf,strlen(buf)+1,0)!=strlen(buf)+1){
            /*Error Handelling*/
            close(newsockfd);
            close(sockfd);
            error("send() sent a different number of bytes than expected");
        }
        
        /*Success message to be displayed on server side*/
        printf("--Date and Time sent successfully to client.\n");
        
        /* the following system call closes the socket.*/
        close(newsockfd);
    }

    /* the following system call closes the socket.*/
    close(sockfd);
    return 0;
}