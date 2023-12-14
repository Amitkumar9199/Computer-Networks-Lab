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
#define SZ 110   
                    /*Client Process*/
int main(){

    int sockfd; /*socket descriptors*/
    struct sockaddr_in serv_addr;
    
    int i;
    char buf[SZ]; /*message buffer*/
    char* res;

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
    size_t reslen =0, buflen = 0;
    do{
        for(int i=0;i<SZ;i++)buf[i]='\0';
        
        /*Receive messgae from server in chunks*/
        int rec_len = recv(sockfd,buf,SZ-10,0);
        if(rec_len<=0){
            /*Error Handelling*/
            free(res);
            close(sockfd);
            error("Unable to recieve data from server");
        };

        /*append the received string in the end of buffer res
            in O(length(buf)) time*/
        buflen = strlen(buf);
        res = realloc(res, reslen+buflen+1);
        strcpy(res+reslen, buf);
        reslen += buflen;
        
        /*Stop receiving from server when you get '\0'
            in the end of received message*/
        if(buf[rec_len-1]=='\0'){
            break;
        }
    }while(1);
    
    /*message to be displayed on client side*/
    printf("Date and Time : %s\n",res);

    /*free memory*/
    free(res);

    /* the following system call closes the socket.*/
    close(sockfd);
    
    return 0;
}