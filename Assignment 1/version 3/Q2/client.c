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
#define SZ 210
                    /*Client Process*/
int main(){
    int sockfd; /*socket descriptors*/
    struct sockaddr_in serv_addr;
    int i=0;
    char tempbuf[SZ];
  
    printf("The client is ready...\n\n");
    while (1)
    {
        i++;
        printf("Iteration #%d\n",i);
        printf("Enter the Expression : ");
        
        char* buf=readinputstring(); /*message buffer*/
        if(!strcmp(buf,"-1")){
            printf("Client exit.\n");
            free(buf);
            break;
        }
        sockfd = socket(AF_INET,SOCK_STREAM,0);
        if(sockfd<0){
            free(buf);
            error("Unable to create socket");
        }
        
        serv_addr.sin_family = AF_INET;
        inet_aton("127.0.0.1",&serv_addr.sin_addr);
        serv_addr.sin_port = htons(20000);

        if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){
            close(sockfd);
            free(buf);
            error("Unable to connect to the server");
        }
        if(send(sockfd,buf,strlen(buf)+1,0)!=strlen(buf)+1){
            close(sockfd);
            free(buf);
            error("send() sent a different number of bytes than expected");
        }
        for(int i=0;i<SZ;i++)tempbuf[i]='\0';
        if(recv(sockfd,tempbuf,SZ-100,0)<=0){
            close(sockfd);
            free(buf);
            error("Unable to recieve data from server");
        }
        printf("Message from server (Expression result) : %s\n\n",tempbuf);
        close(sockfd);
        free(buf);
    }
    return 0;
}
