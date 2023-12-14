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

/*recursive function to calculate the value of expression*/
float cal(char *buf,int *i,int len){
    /*Stores final output*/
    float res=0;

    /*Stores the operator which is going to be used next*/
    char op='+';

    /*Untill whole string is read*/
    while((*i) < len){

        /*Leave space char*/
        if (buf[*i]==' '){
            (*i)++;
            continue;
        }

        /*Read float value and do the operation*/
        if( buf[*i]=='.' || ( buf[*i]>='0' && buf[*i]<='9' ) ){
            /*atof fn converts string arguments to floating*/
            float val=atof(buf + *i);
            
            /*update res*/
            if(op=='+')res+=val;
            else if(op=='-')res-=val;
            else if(op=='*')res*=val;
            else if(op=='/')res/=val;
            while( ((*i) < len) && ( buf[*i]=='.' || ( buf[*i]>='0' && buf[*i]<='9' ) ) ){
                (*i)++;
            }
            continue;
        }

        /*Read new operator and update op*/
        if(buf[*i]=='+'||buf[*i]=='-'||buf[*i]=='*'||buf[*i]=='/'){
            op=buf[*i];
            (*i)++;
            continue;
        }

        /*If we get a '(' call cal function again, with updated index*/
        /*Get value inside '(' and ')' and update res;*/
        if(buf[*i]=='('){
            (*i)++;
            float val=cal(buf,i,len);
            if(op=='+')res+=val;
            else if(op=='-')res-=val;
            else if(op=='*')res*=val;
            else if(op=='/')res/=val;
            continue;
        }

        /*return the res value, since the value inside () bracket is calculated*/
        if(buf[*i]==')'){
            (*i)++;
            return res;
        }
        (*i)++;
    }

    /*return result*/
    return res;
}


#define SZ 110

                    /*Server Process*/
int main(){
    int sockfd, newsockfd;/*Socket Descriptors*/
    int clilen;

    struct sockaddr_in serv_addr,cli_addr;
    int i;
    char tempbuf[SZ]; /*message buffer*/
    
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
        close(sockfd);
        error("Unable to bind local address");
    }

    /* The listen system call allows the process to
     listen on the socket for connections.*/
    if(listen(sockfd,5)<0){
        /*Error Handelling*/
        close(sockfd);
        error("Listen failed");
    }

    char *buf;/*To reiceive data from client*/
    printf("The Server is ready...\n");

    /*iterative server*/
    while(1){

        /* The accepth call accepts the client connection.
           It blocks the server until a client request comes.
        */
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd,(struct sockaddr *)&cli_addr,&clilen);
        if(newsockfd<0){
            /*Error Handelling*/
            free(buf);
            close(sockfd);
            error("Accept failed");
        }

        /*The following system call receive the message
         from the client.*/
        size_t buflen =0, tempbuflen = 0;
        do{
            for(int i=0;i<SZ;i++)tempbuf[i]='\0';
            
            /*Receive messgae from client in chunk of size 100*/
            int rec_len = recv(newsockfd,tempbuf,SZ-10,0);
            if(rec_len<=0){
                /*Error Handelling*/
                free(buf);
                close(newsockfd);
                close(sockfd);
                error("receive failed");
            };

            /*append the received string in the end of buffer buf
              in O(length(tempbuf)) time*/
            tempbuflen = strlen(tempbuf);
            buf = realloc(buf, buflen+tempbuflen+1);
            strcpy(buf+buflen, tempbuf);
            buflen += tempbuflen;
            
            /*Stop receiving from client when you get '\0'
             in the end of received message*/
            if(tempbuf[rec_len-1]=='\0'){
                break;
            }
        }while(1);

        /*Success message to be displayed on server side*/
        printf("Message from client is received.\n");
        printf("Message from client (Expression) : %s\n\n",buf);
        
        /*Calculaing value of the expression*/
        int j=0;
        float ans=cal(buf,&j,strlen(buf));

        /*convert float value to string*/
        sprintf(buf,"%f",ans);

        /*send the calculated value to client*/
        if(send(newsockfd,buf,strlen(buf)+1,0)!=strlen(buf)+1){
            /*Error Handelling*/
            free(buf);
            close(newsockfd);
            close(sockfd);
            error("send() sent a different number of bytes than expected");
        }

        /* the following system call closes the socket.*/
        close(newsockfd);
    }

    /*free memory.*/
    free(buf);

    /* the following system call closes the socket.*/
    close(sockfd);
    return 0;
}
