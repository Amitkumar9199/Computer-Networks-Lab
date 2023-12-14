#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

void error(const char *msg){
    perror(msg);
    exit(1);
}                
          
#define ok 2000100  
                    /*Client Process*/
int main(){
    int sockfd; /*socket descriptors*/
    struct sockaddr_in serv_addr;
    int i;
    char *buf; /*message buffer*/
    buf = malloc(ok*sizeof(char));
    /* the following syatem call opens a socket.*/

    /* We specified INADDR_ANY when we specified the server address in the server. Since client can run on different machine , we must specify the IP addeess of the server.In this program we assume that the server isrunning on the same machine as the client. 127.0.0.1 is a special IP address for "localhost" this machine .*/
    /* if the server runs on some other machine , we must change the ip address specified below to the ip address of the machine we are running the server .*/
    /* The inet_aton() function converts the Internet host address cp from the IPv4 numbers-and-dots notation into binary form in network byte order and stores it in the structure that inp points to.*/
    FILE * fp;
    fp = fopen("output.txt", "r");
    printf("The client is ready...\n\n");
     fgets(buf,ok,fp);
    while (1)
    {
        i++;
        printf("Iteration #%d\n",i);
        printf("Enter the Expression : ");
        // scanf("%s",buf);
  	   
        // printf("%u",(int)strlen(buf));
        if(!strcmp(buf,"-1")){
            printf("Client is exiting\n");
            break;
        }
        sockfd = socket(AF_INET,SOCK_STREAM,0);
        if(sockfd<0){
            error("Unable to create socket");
        }
        
        serv_addr.sin_family = AF_INET;
        inet_aton("127.0.0.1",&serv_addr.sin_addr);
        serv_addr.sin_port = htons(20000);

        /* With the information specified in serv_addr, the connect() system call establishes a connection with the server process.*/
        if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){
            close(sockfd);
            error("Unable to connect to the server");
        }
        buf[strlen(buf)]='\0';
        char hgh[100];
        sprintf(hgh,"%d",(int)strlen(buf)+1);
        send(sockfd,hgh,strlen(hgh)+1,0);
        /* After connection , client can send or receive messages. However, the recv() will block when the server is not sending and vice-versa. Similarly, send() will block if server is not receiving and vice-versa. For non-blocking modes, refer to online manpages.*/
        int max_send = send(sockfd,buf,strlen(buf)+1,0);
        /*max_send limit is 523856*/
        // strcpy(buf,"Message from Client");
        if(max_send!=strlen(buf)+1){
            close(sockfd);
            printf("message length sent = %d\n",max_send);
            error("send() sent a different number of bytes than expected");
        }
    
        if(recv(sockfd,buf,strlen(buf)+1,0)<=0){
            close(sockfd);
            error("Unable to recieve data from server");
        }
        printf("Message from server (Expression result) : %s\n\n",buf);
        close(sockfd);
        // break;
    }
    return 0;
}
