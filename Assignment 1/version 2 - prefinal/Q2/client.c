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

char* readinputstring(){
    #define chunk 200
    
    char* input = NULL;
    char* iterator;
    char* res;
    char tempbuf[chunk];
    size_t inputlen=0,templen=0;
    do{
        if(fgets(tempbuf,chunk,stdin)==NULL)break;
        templen=strlen(tempbuf);
        input = realloc(input,inputlen+templen+1);
        strcpy(input+inputlen,tempbuf);
        inputlen+=templen;
    }while((templen==chunk-1)&&(tempbuf[chunk-2]!='\n'));
    iterator=input;
    while(*iterator != '\n')
        iterator++;
    *iterator='\0';
    res=malloc(strlen(input)+1);
    strcpy(res,input);
    free(input);
    return res;
    
    #undef chunk
}
            
                    /*Client Process*/
int main(){
    int sockfd; /*socket descriptors*/
    struct sockaddr_in serv_addr;
    int i;
  
    printf("The client is ready...\n\n");
    while (1)
    {
        i++;
        printf("Iteration #%d\n",i);
        printf("Enter the Expression : ");
        
        char* buf=readinputstring(); /*message buffer*/
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

        if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){
            close(sockfd);
            error("Unable to connect to the server");
        }

        char hgh[100];
        sprintf(hgh,"%d",(int)strlen(buf)+1);
        send(sockfd,hgh,strlen(hgh)+1,0);
        recv(sockfd,hgh,100,0);
        if(send(sockfd,buf,strlen(buf)+1,0)!=strlen(buf)+1){
            close(sockfd);
            error("send() sent a different number of bytes than expected");
        }
    
        if(recv(sockfd,buf,100,0)<=0){
            close(sockfd);
            error("Unable to recieve data from server");
        }
        printf("Message from server (Expression result) : %s\n\n",buf);
        close(sockfd);

        free(buf);
    }
    return 0;
}
