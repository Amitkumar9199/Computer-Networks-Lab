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

                    /*Server Process*/
int main(){
    int sockfd, newsockfd;/*Socket Descriptors*/
    int clilen;
    struct sockaddr_in serv_addr,cli_addr;
    int i;
    char buf[100]; /*Communication message*/
    time_t ticks;

    /* The following system call opens a socket.  family of protocol : AF_INET :SOCK_STREAM user application :0*/
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        error("Cannot create socket");
    }
    /* The structure sockaddr_in is defined in <netinet/in.h> for theinternet family of protocols. This has 3 main fields.sin_family : family (AF_INET for the internet family)sin_addr : INADDR_ANY(internet address of the server)sin_port : port number of serversin means socket internetINADDR_ANY is used for machines having single IP Addresshtons : host to network short : to convert an IPport number in host byte order to IP Port number in networkbyte order.*/
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(20000);

    /* With the information provided in serv_addr , we cassociate the server with its port using the bind() system call*/
    if(bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){
        error("Unable to bind local address");
    }
    /* The listen system call allows the process to listen on the socket for connections. The first argument is the socket file descriptor, and the second is the size of the backlog queue, i.e., the number of connections that can be waiting while the process is handling a particular connection. This should be set to 5, the maximum size permitted by most systems.*/
    if(listen(sockfd,5)<0){
        close(sockfd);
        error("Listen failed");
    }
    printf("The Server is ready...\n");
    /*In this program we are making an iterative server -- one which handles client connections one by one. i.e., no concurrency. The accept() system call returns a new socket descriptor which is used for communicationwith the server. After the communication is over , the process comes back to wait again on the original socketdescriptor.*/
    while(1){
        /* The accepth call accepts the client connection.It blocks the server until a client request comes.The accept() system call fills up the client's detailsin a struct sockaddr which is passed as a parameter.The length of the structure is noted in clilen. Note that the new socket descriptor returned by the accept() system call is stored in "newsocketfd"*/
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd,(struct sockaddr *)&cli_addr,&clilen);
    
        if(newsockfd<0){
            close(sockfd);
            error("Accept failed");
        }
        
        /* We now receive a message from the client.There has to be some mechanism for the receivingside to knnow when the entire messageis received. Look up the return value of recv() to see how you can do this.*/
        int rec_len = recv(newsockfd,buf,100,0);
        if(rec_len<=0){
            close(newsockfd);
            close(sockfd);
            error("receive failed");
        };
        printf("Message from client (Expression) : %s\n",buf);
        int j=0;
        float ans=cal(buf,&j,strlen(buf));
        /* We initialize the buffer , copy the message to it, and send the message to the client.*/
        // sprintf(buf,"%f",res); // method 1
        gcvt(ans,5,buf); // method 2
        if(send(newsockfd,buf,strlen(buf)+1,0)!=strlen(buf)+1){
            close(newsockfd);
            close(sockfd);
            error("send() sent a different number of bytes than expected");
        }

        close(newsockfd);
    }
    close(sockfd);
    return 0;
}