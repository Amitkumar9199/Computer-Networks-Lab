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


/*Function which reads an input string of unknown length*/
char* readinputstring(){

    /*We will take input in chunks of size<=200*/
    #define chunk 200


    /*Temporary memeory where we will keep the input string*/
    char* input = NULL;

    /*For replacing '\n' by '\0' in end of string */
    char* iterator;

    /*Final input string to return*/
    char* res;

    /*Temporary buffer to read input in chunks*/
    char tempbuf[chunk];

    /*Length of input/tempbuf string*/
    size_t inputlen=0,templen=0;
    do{
        /*if no input is taken*/
        if(fgets(tempbuf,chunk,stdin)==NULL)break;
        
        /*copy tempbuf into main input 
            string in O(length(tempbuf)) time*/
        templen=strlen(tempbuf);
        input = realloc(input,inputlen+templen+1);
        strcpy(input+inputlen,tempbuf);
        inputlen+=templen;

        /*Do it untill you get '\n' in the end*/
    }while((templen==chunk-1)&&(tempbuf[chunk-2]!='\n'));
    
    iterator=input;
    /*replacing '\n' by '\0' in end of string*/
    while(*iterator != '\n')
        iterator++;
    *iterator='\0';
    
    /*copy final string to res*/
    res=malloc(strlen(input)+1);
    strcpy(res,input);

    /*free memory used by input, which has no use now*/
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
    char tempbuf[SZ]; /*message buffer*/
    
    while (1)
    {
        /*Client side Terminal UI*/
        i++;
        printf("Iteration #%d\n",i);
        printf("Enter the Expression : ");
        
        /*Take input of a string of unknown length */
        char* buf=readinputstring(); 
        
        /*I'm exiting client only if user 
            enters "-1" as string input*/
        if(!strcmp(buf,"-1")){
            printf("Client exit.\n");
            free(buf);/*free memory*/
            break;
        }

        /* the following syatem call opens a socket.*/
        sockfd = socket(AF_INET,SOCK_STREAM,0);
        if(sockfd<0){
            /*Error Handelling*/
            free(buf);/*free memory*/
            error("Unable to create socket");
        }

        /* defining structure sockaddr_in for the 
            internet family of protocols. */    
        serv_addr.sin_family = AF_INET;
        inet_aton("127.0.0.1",&serv_addr.sin_addr);
        serv_addr.sin_port = htons(20000);

        /* With information in serv_addr,connect() system
         call establishes a connection with  server process.*/
        if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){
            /*Error Handelling*/
            close(sockfd);
            free(buf);/*free memory*/
            error("Unable to connect to the server");
        }

        /* After connection , client can send 
            or receive messages.*/
        /* Send input expression to server */
        if(send(sockfd,buf,strlen(buf)+1,0)!=strlen(buf)+1){
            /*Error Handelling*/
            close(sockfd);
            free(buf);/*free memory*/
            error("send() sent a different number of bytes than expected");
        }
        for(int i=0;i<SZ;i++)tempbuf[i]='\0';

        /*Recieve output(value of expression) from server.*/
        if(recv(sockfd,tempbuf,SZ-100,0)<=0){
            /*Error Handelling*/
            close(sockfd);
            free(buf);/*free memory*/
            error("Unable to recieve data from server");
        }

        /*message to be displayed on client side*/
        printf("Message from server (Expression result) : %s\n\n",tempbuf);
        
        /* the following system call closes the socket.*/
        close(sockfd);

        /*free memory*/
        free(buf);
    }
    return 0;
}
