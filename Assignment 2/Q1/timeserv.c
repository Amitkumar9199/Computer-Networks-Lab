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
#define SZ 1024

            /*Server Process*/
int main(){
    
    int sockfd;/*Socket Descriptors*/
    struct sockaddr_in serv_addr,cli_addr;
    
    /* The following system call creates a socket.*/
    sockfd = socket(AF_INET,SOCK_DGRAM,0);
    if(sockfd<0){
        /*Error Handelling*/
        perror("Cannot create socket");
        exit(EXIT_FAILURE);
    }

    /* defining structure sockaddr_in for the internet family of protocols. */    
    memset(&serv_addr,0,sizeof(serv_addr));
    memset(&cli_addr,0,sizeof(cli_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(8181);

    /* With the information provided in serv_addr , 
       we can associate the server with its port using  the bind() system call */
    if(bind(sockfd,(const struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){
        /*Error Handelling*/
        perror("Unable to bind local address");
        exit(EXIT_FAILURE); 
    }
    
    printf("-- Server is running on port 8181\n");

    int n;
    char buf[SZ]; /*message buffer*/
    time_t ticks; /*For storing Local machine time*/
    socklen_t len;

    /*iterative server*/
    while(1){
        
        /* Receive msg from client */
        len = sizeof(cli_addr);
        for(int i=0;i<SZ;i++)buf[i]='\0';

        /*  recvfrom() returns the length of the message on success, or -1 if an error occurred. */
        n = recvfrom(sockfd,(char*)buf,SZ,0,(struct sockaddr*)&cli_addr,&len);

        if(n<0){
            /*Error Handelling*/
            error("Unable to receive data from socket");
        }else if(n==0){
            /*Error Handelling*/
            error("Connection closed by client");
        }

        /*message to be displayed on server side*/
        buf[n]='\0';
        printf("Client says : %s\n",buf);

        /* We copy date and time to buf */
        ticks = time(NULL); // get current time
        for(int i=0;i<SZ;i++)buf[i]='\0';
        strcpy(buf,ctime(&ticks));

        /* send message(date and time) to client.*/
        /*  sendto() returns the number of characters sent, or -1 if an error occurred. */
        if( sendto(sockfd,(const char*)buf,strlen(buf),0,(const struct sockaddr*)&cli_addr,sizeof(cli_addr)) < 0 ){
            /*Error Handelling*/
            error("Unable to send data to socket\n");
        }

        /*Success message to be displayed on server side*/
        printf("--Date and Time sent successfully to client.\n\n");

    }

    /* the following system call closes the socket.*/
    close(sockfd);
    return 0;
}