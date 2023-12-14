/* Important Library */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

/* Function to give error msg to user, and exit */
void error(const char *msg){
    perror(msg);
    exit(1);
}

/* returns the minimum of two integers*/
int min(int a,int b){
    if(a<b)return a;
    return b;
}

#define SZ 1024

                    /*Client Process*/

int main(int argc, char* argv[]) {

    /* argc is the number of arguments passed to the program
     * argv is an array of strings containing the arguments
     * argv[0] is the name of the program
     * argv[1] is the first argument - own port
     * argv[2] is the second argument - load balancer port
    */
    if (argc < 3) {
        printf("Usage: %s <client port> <load balancer port>\n", argv[0]);
        return 1;
    }

    /* convert port numbers from string to int */
    int own_port = atoi(argv[1]);
    int lb_port = atoi(argv[2]);

    int sockfd; /* socket descriptors */
    struct sockaddr_in lb_addr;

    /* the following system call opens a socket. */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        /*Error Handelling*/
        error("Unable to create socket");
    }

    /* defining structure sockaddr_in for the internet family of protocols. */
    memset(&lb_addr, 0, sizeof(lb_addr));
    lb_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1",&lb_addr.sin_addr);
    lb_addr.sin_port = htons(lb_port);

    /* With information in lb_addr,connect() system call establishes a connection with  load balancer process. */
    if (connect(sockfd, (struct sockaddr *) &lb_addr, sizeof(lb_addr)) < 0) {
        /* Error Handelling */
        close(sockfd);
        error("Error connecting to load balancer");
    }


    /* After connection , client can send or receive messages. */

    /* send time request to load balancer */
    char buffer[SZ];
    strcpy(buffer,"Send Time");
    size_t bufferlen = strlen(buffer)+1;
    int idx=0;

    while (1){
        /* We send the data in chunks of 10 bytes or length of left string , whichever is minimum)*/
        int bytessend = send(sockfd,buffer+idx,min((int)strlen(buffer+idx)+1,10),0);
        
        if(bytessend<0){
            /*Error handelling*/
            close(sockfd);
            error("Unable to send data to load balancer.");
        }else if(bytessend==0){
            /*Error Handelling*/
            close(sockfd);
            error("Client closed connection.\n");
        }
        
        idx+=bytessend;
        if(idx>=bufferlen)break;
    }


    /* Receive Date and Time from load balancer */
    char* res=NULL;
    size_t reslen =0; bufferlen = 0;
    while(1){
        for(int i=0;i<SZ;i++)buffer[i]='\0';
        
        /*Receive messges in chunks*/
        int rec_len = recv(sockfd,buffer,10,0);

        if(rec_len<0){
            /*Error Handelling*/
            if(res)free(res);
            close(sockfd);
            error("Receive failed from load balancer.");
        }else if(rec_len==0){
            /*Error Handelling*/
            if(res)free(res);
            close(sockfd);
            error("Client closed connection.");
        }

        /*append the received string to res in O(length(buffer)) time*/
        bufferlen = strlen(buffer);
        res = realloc(res, reslen+bufferlen+1);
        strcpy(res+reslen, buffer);
        reslen += bufferlen;
        
        /*Stop receiving when you get '\0' in end */
        if(buffer[rec_len-1]=='\0')break;
    }

    /*message to be displayed on client side*/
    printf("Date and Time : %s\n",res);

    /* free the memory */
    if(res)free(res);

    /* close the socket */
    close(sockfd);

    return 0;
}
