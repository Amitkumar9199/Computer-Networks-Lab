/*Important Library*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<time.h>

/*Function to give error msg to user, and exit*/
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
            /*Server Process*/
            
int main(int argc, char *argv[])
{
    /* argc is the number of arguments passed to the program
     * argv is an array of strings containing the arguments
     * argv[0] is the name of the program
     * argv[1] is the first argument - own port
    */
    if (argc < 2)
    {
        printf("Usage: %s <server port>\n", argv[0]);
        return 1;
    }

    /* convert port number from string to int */
    int serv_port = atoi(argv[1]);


    int sockfd, newsockfd;/*Socket Descriptors*/
    int lblen;
    struct sockaddr_in serv_addr,lb_addr;
    
    /* the following system call opens a socket. */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        /*Error Handelling*/
        error("Cannot create socket");
    }

    /* defining structure sockaddr_in for the internet family of protocols. */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(serv_port);

    /* With information in serv_addr, We can associate the server with its port using the bind() system call */
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        close(sockfd);
        error("Unable to bind local address");
    }

    /* The listen() system call allows the process to listen on the socket for connections. */
    if (listen(sockfd, 5) < 0){
        close(sockfd);
        error("Listen failed.");
    }

    /*  srand is used to generate random numbers. */
    /*  seed random number generator */
    srand(time(NULL));

    /* iterative server */
    while (1)
    {
        /* The accepth call accepts the load balancer connection. It blocks the server until a load balancer request comes. */
        lblen = sizeof(lb_addr);
        newsockfd = accept(sockfd, (struct sockaddr *)&lb_addr, &lblen);
        if (newsockfd < 0){
            /*Error Handelling*/
            close(sockfd);
            error("Accept failed");
        }

        /* buffer to store the request from the load balancer */
        char* res=NULL;
        char buffer[SZ];
        size_t reslen =0, bufferlen = 0;
        while(1){
            for(int i=0;i<SZ;i++)buffer[i]='\0';
            
            /*Receive messges in chunks*/
            int rec_len = recv(newsockfd,buffer,10,0);

            if(rec_len<0){
                /*Error Handelling*/
                if(res)free(res);
                close(newsockfd);
                close(sockfd);
                error("Receive failed from load balancer.");
            }else if(rec_len==0){
                /*Error Handelling*/
                if(res)free(res);
                close(newsockfd);
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

        for(int i=0;i<SZ;i++)buffer[i]='\0';
        
        /* check if request is to send date/time or load */
        if (strcmp(res, "Send Load") == 0)
        {
            /* generate random load and store in buffer */
            int load = rand() % 100+1;
            sprintf(buffer, "%d", load);    

            /* send load to load balancer */
            bufferlen = strlen(buffer)+1;
            int idx=0;

            while (1){
                /* We send the data in chunks of 10 bytes or length of left string , whichever is minimum)*/
                int bytessend = send(newsockfd,buffer+idx,min((int)strlen(buffer+idx)+1,10),0);
                
                if(bytessend<0){
                    /*Error handelling*/
                    close(newsockfd);
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

            /* message to be printed on server side */
            printf("Load sent: %s\n", buffer);
        }
        else if (strcmp(res, "Send Time") == 0)
        {
            /*For storing Local machine time*/
            time_t ticks;
            ticks = time(NULL);
            strcpy(buffer,ctime(&ticks));

            /* send time to load balancer */
            bufferlen = strlen(buffer)+1;
            int idx=0;

            while (1){
                /* We send the data in chunks of 10 bytes or length of left string , whichever is minimum)*/
                int bytessend = send(newsockfd,buffer+idx,min((int)strlen(buffer+idx)+1,10),0);
                
                if(bytessend<0){
                    /*Error handelling*/
                    close(newsockfd);
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

            /* message to be printed on server side */
            printf("Time sent: %s", buffer);
        }
        else
        {
            printf("Invalid request received from load balancer.\n");
        }

        /* the following system call closes the socket.*/
        close(newsockfd);
    }

    /* the following system call closes the socket.*/
    close(sockfd);
    
    return 0;
}
