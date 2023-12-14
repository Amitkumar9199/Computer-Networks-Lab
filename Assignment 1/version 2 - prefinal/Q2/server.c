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
float cal(char *buf,int *i,int len){
    float res=0;
    char op='+';
    while((*i) < len){
        if (buf[*i]==' '){
            (*i)++;
            continue;
        }
        if( buf[*i]=='.' || ( buf[*i]>='0' && buf[*i]<='9' ) ){
            float val=atof(buf + *i);
            if(op=='+')res+=val;
            else if(op=='-')res-=val;
            else if(op=='*')res*=val;
            else if(op=='/')res/=val;
            while( ((*i) < len) && ( buf[*i]=='.' || ( buf[*i]>='0' && buf[*i]<='9' ) ) ){
                (*i)++;
            }
            continue;
        }
        if(buf[*i]=='+'||buf[*i]=='-'||buf[*i]=='*'||buf[*i]=='/'){
            op=buf[*i];
            (*i)++;
            continue;
        }
        if(buf[*i]=='('){
            (*i)++;
            float val=cal(buf,i,len);
            if(op=='+')res+=val;
            else if(op=='-')res-=val;
            else if(op=='*')res*=val;
            else if(op=='/')res/=val;
            continue;
        }
        if(buf[*i]==')'){
            (*i)++;
            return res;
        }
        (*i)++;
    }
    return res;
}
#define ok 200000
                    /*Server Process*/
int main(){
    int sockfd, newsockfd;/*Socket Descriptors*/
    int clilen;
    struct sockaddr_in serv_addr,cli_addr;
    int i;
    char tempbuf[ok]; /*Communication message*/
    time_t ticks;
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        error("Cannot create socket");
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(20000);

    if(bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){
        error("Unable to bind local address");
    }
    if(listen(sockfd,5)<0){
        close(sockfd);
        error("Listen failed");
    }
    char *buf;
    printf("The Server is ready...\n");
    while(1){
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd,(struct sockaddr *)&cli_addr,&clilen);
    
        if(newsockfd<0){
            close(sockfd);
            error("Accept failed");
        }
        
        
        recv(newsockfd,tempbuf,ok,0);
        send(newsockfd,tempbuf,strlen(tempbuf)+1,0);
        int fg=atoi(tempbuf);
        size_t buflen =0, tempbuflen = 0;
        for(int i=0;i<ok;i++)tempbuf[i]='\0';
        do{
            for(int i=0;i<ok;i++)tempbuf[i]='\0';
            int rec_len = recv(newsockfd,tempbuf,100,0);
            if(rec_len<=0){
                close(newsockfd);
                close(sockfd);
                error("receive failed");
            };
            tempbuflen = strlen(tempbuf);
            buf = realloc(buf, buflen+tempbuflen+1);
            strcpy(buf+buflen, tempbuf);
            buflen += tempbuflen;
            printf("buflen = %ld, fg = %d\n",buflen,(int)fg);
            if(fg==(int)buflen+1){
                printf("null\n");break;
            }
        }while(1);
        printf("Message from client (Expression) : %s\n",buf);
        int j=0;
        float ans=cal(buf,&j,strlen(buf));
        gcvt(ans,5,buf); // method 2
        if(send(newsockfd,buf,strlen(buf)+1,0)!=strlen(buf)+1){
            close(newsockfd);
            close(sockfd);
            error("send() sent a different number of bytes than expected");
        }

        close(newsockfd);
    }
    free(buf);
    close(sockfd);
    return 0;
}