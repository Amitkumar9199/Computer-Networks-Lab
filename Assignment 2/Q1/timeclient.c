/*Important Library*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<poll.h>


/*Function to give error msg to user, and exit*/
void error(const char *msg){
    perror(msg);
    exit(1);
}               

#define SZ 1024
#define time_out_msecs 3000 /* 3 seconds client timeout */

                    /*Client Process*/
int main(){

    int sockfd; /*socket descriptors*/
    struct sockaddr_in serv_addr;

    /* the following system call creates a socket.*/
    sockfd = socket(AF_INET,SOCK_DGRAM,0);
    if(sockfd<0){
        /*Error Handelling*/
        perror("Unable to create socket");
        exit(EXIT_FAILURE); 
    }

    /* defining structure sockaddr_in for the internet family of protocols. */
    memset(&serv_addr,0,sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1",&serv_addr.sin_addr);
    serv_addr.sin_port = htons(8181);

    int n;
    socklen_t len;
    char* hello="Give me date and time of your machine."; /* message to be sent to server */
    char buffer[SZ]; /* buffer to store the received data */

    /*client tries a maximum of 5 times*/
    /* Loop runs at max 5 times */
    int counter = 1;
    while (counter <= 5) {
        
        /* Send connection msg to server */
        /*  sendto() returns the number of bytes sent, or -1 if an error occurred. */
        if( sendto(sockfd,(const char*)hello,strlen(hello),0,(const struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0){
            /*Error Handelling*/
            error("Unable to send data to socket\n");
        }
        printf("-- Connection message sent from client.\n");
  
        /* Polling for giving timeout of 3 seconds to UDP client to recv msg from server.*/
        
        /* fdset is a structure of type pollfd */
        struct pollfd fdset;
        
        /* fdset.fd is the socket descriptor */
        fdset.fd = sockfd;

        /* fdset.events is the event to be monitored */
        fdset.events = POLLIN;

        /* poll() returns 0 if timeout, 1 if input available, -1 if error. */
        int ret = poll(&fdset, 1, time_out_msecs);
        
        if (ret < 0) { // error
            error("poll Failure");
        }
        else if (ret == 0) { // timeout
            printf("poll timeout : counter = %d\n\n",counter);
        }
        else { // data received

            /* Check if the socket has received data */
            if (fdset.revents & POLLIN) {

                /* Receive msg from server */
                len = sizeof(serv_addr);
                for(int i=0;i<SZ;i++)buffer[i]='\0';

                /*  recvfrom() returns the number of bytes received, or -1 if an error occurred. */
                n = recvfrom(sockfd,(char*)buffer,SZ,0,(struct sockaddr*)&serv_addr,&len);
                
                if(n<0){
                    /*Error Handelling*/
                    error("Unable to receive data from socket\n");
                }else if(n==0){
                    /*Error Handelling*/
                    error("serer closed connection.\n");
                }

                /*message to be displayed on client side*/
                buffer[n]='\0';
                printf("Date and Time : %s\n",buffer);
                
                break; // break out of while loop
            }
        }
        counter++; // increment counter
    }
    
    /*  Client tries 5 times, and server is not responding. */
    if(counter > 5) printf("-- Server is not responding.\n\n");

    /* the following system call closes the socket.*/
    close(sockfd);
    
    return 0;
}