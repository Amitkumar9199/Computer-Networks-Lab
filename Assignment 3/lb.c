/* Important Library */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include <sys/time.h>
#include<poll.h>

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
#define TIME_OUT 5000

                    /*Load balancer Process*/
int main(int argc, char *argv[])
{
    /* argc is the number of arguments passed to the program
     * argv is an array of strings containing the arguments
     * argv[0] is the name of the program
     * argv[1] is the first argument - own port
     * argv[2] is the second argument - server 1 port
     * argv[3] is the third argument - server 2 port
    */
    if (argc < 4){
        printf("Usage: %s <own port> <serv_1_port> <serv_2_port>\n", argv[0]);
        return 1;
    }

    
    /* Socket Descriptors */
    int sockfd, serv_1_sockfd, serv_2_sockfd,cli_sockfd;

    /* Structure to store server address */
    struct sockaddr_in lb_addr, serv_1_addr, serv_2_addr, cli_addr;
    int clilen;
    /* Port number */
    int port, serv_1_port, serv_2_port;
    
    char buffer[SZ];

    /* fdset is a structure of type pollfd */
    struct pollfd fds[2];

    /* finding port number from command line arguments */
    port = atoi(argv[1]);
    serv_1_port = atoi(argv[2]);
    serv_2_port = atoi(argv[3]);

    /* the following system call opens a socket. */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd <0){
        /*Error Handelling*/
        error("Cannot create socket");
    }

    /* defining structure sockaddr_in for the internet family of protocols. */
    /* for load balancer */
    lb_addr.sin_family = AF_INET;
    lb_addr.sin_addr.s_addr = INADDR_ANY;
    lb_addr.sin_port = htons(port);

    /* With information in serv_addr, We can associate the server with its port using the bind() system call */
    if (bind(sockfd, (struct sockaddr *)&lb_addr, sizeof(lb_addr)) < 0){
        /*Error Handelling*/
        close(sockfd);
        error("Unable to bind local address");
    }

    /* The listen() system call allows the process to listen on the socket for connections. */
    if (listen(sockfd, 5) < 0){
        /*Error Handelling*/
        close(sockfd);
        error("Listen failed.");
    }


    /* defining structure sockaddr_in for the internet family of protocols. */
    /* for server 1 */
    memset(&serv_1_addr, 0, sizeof(serv_1_addr));
    serv_1_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1",&serv_1_addr.sin_addr);
    serv_1_addr.sin_port = htons(serv_1_port);
    
    
    /* defining structure sockaddr_in for the internet family of protocols. */
    /* for server 2 */
    memset(&serv_2_addr, 0, sizeof(serv_2_addr));
    serv_2_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1",&serv_2_addr.sin_addr);
    serv_2_addr.sin_port = htons(serv_2_port);

    
    /* fdset.fd is the socket descriptor */
    fds[0].fd = sockfd;

    /* fdset.events is the event to be monitored */
    fds[0].events = POLLIN;
    fds[1].fd = -1;

    /* Table to store load of server 1 and server 2 */
    int serv_1_load[SZ], serv_2_load[SZ] ; 
    serv_1_load[0]=0; serv_2_load[0]=0;
    int serv_1_idx=-1, serv_2_idx=-1;

    /* 0 seconds so that it takes initial load from server 1 and server 2 */
    double timeout = 0; 

    int counter =0;
    while (1)
    {
        /* start time */
        struct timeval start, end;
        gettimeofday(&start, NULL);
        if(counter)printf("\nWaiting for : %f ms\n", timeout);
        
        /* poll() returns 0 if timeout, 1 if input available, -1 if error. */
        int ret = poll(fds, 2, timeout);
    
        if (ret == -1){// error
            close(sockfd);
            error("poll Failure");
        }
        else if (ret == 0){ // timeout
            
            /* increase circular index */
            serv_1_idx = (serv_1_idx+1)%SZ;
            serv_2_idx = (serv_2_idx+1)%SZ;

            /* if timeout , then open connection to server 1 */
            
            /* create socket */
            serv_1_sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (serv_1_sockfd < 0){
                /*Error Handelling*/
                close(sockfd);
                error("Cannot create socket");
            }

            /* connect to server 1 */
            if (connect(serv_1_sockfd, (struct sockaddr *)&serv_1_addr, sizeof(serv_1_addr)) < 0){
                /*Error Handelling*/
                close(serv_1_sockfd);
                close(sockfd);
                error("Unable to connect to server 1");
            }

            /* send load request to server 1 */
            for(int i=0;i<SZ;i++)buffer[i]='\0';
            strcpy(buffer, "Send Load");
            size_t bufferlen = strlen(buffer)+1;
            int idx=0;

            while (1){
                /* We send the data in chunks of 10 bytes or length of left string , whichever is minimum)*/
                int bytessend = send(serv_1_sockfd,buffer+idx,min((int)strlen(buffer+idx)+1,10),0);
                
                if(bytessend<0){
                    /*Error handelling*/
                    close(serv_1_sockfd);
                    close(sockfd);
                    error("Unable to send data to load balancer.");
                }else if(bytessend==0){
                    /*Error Handelling*/
                    close(serv_1_sockfd);
                    close(sockfd);
                    error("Client closed connection.\n");
                }
                
                idx+=bytessend;
                if(idx>=bufferlen)break;
            }


            /* Receive the load from the server */
            char* res=NULL;
            size_t reslen =0; bufferlen = 0;
            while(1){
                for(int i=0;i<SZ;i++)buffer[i]='\0';
                
                /*Receive messges in chunks*/
                int rec_len = recv(serv_1_sockfd,buffer,10,0);

                if(rec_len<0){
                    /*Error Handelling*/
                    if(res)free(res);
                    close(serv_1_sockfd);
                    close(sockfd);
                    error("Receive failed from load balancer.");
                }else if(rec_len==0){
                    /*Error Handelling*/
                    if(res)free(res);
                    close(serv_1_sockfd);
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

            /* convert the load to integer */
            serv_1_load[serv_1_idx] = atoi(res);

            /* free the memory */
            if(res)free(res);

            /* close the socket */
            close(serv_1_sockfd);

            /* message on terminal */
            printf("Load received from server S1 {%s , %d} : %d\n",inet_ntoa(serv_1_addr.sin_addr),ntohs(serv_1_addr.sin_port),serv_1_load[serv_1_idx]);
            
            
            /* if timeout , then open connection to server 1 */

            /* create socket */
            serv_2_sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (serv_2_sockfd < 0){
                /*Error Handelling*/
                close(sockfd);
                error("Cannot create socket");
            }

            /* connect to server 2 */
            if (connect(serv_2_sockfd, (struct sockaddr *)&serv_2_addr, sizeof(serv_2_addr)) < 0){
                /*Error Handelling*/
                close(serv_2_sockfd);
                close(sockfd);
                error("Unable to connect to server 2");
            }

            /* send load request to server 2 */
            for(int i=0;i<SZ;i++)buffer[i]='\0';
            strcpy(buffer, "Send Load");
            bufferlen = strlen(buffer)+1;
            idx=0;

            while (1){
                /* We send the data in chunks of 10 bytes or length of left string , whichever is minimum)*/
                int bytessend = send(serv_2_sockfd,buffer+idx,min((int)strlen(buffer+idx)+1,10),0);
                
                if(bytessend<0){
                    /*Error handelling*/
                    close(serv_2_sockfd);
                    close(sockfd);
                    error("Unable to send data to load balancer.");
                }else if(bytessend==0){
                    /*Error Handelling*/
                    close(serv_2_sockfd);
                    close(sockfd);
                    error("Client closed connection.\n");
                }
                
                idx+=bytessend;
                if(idx>=bufferlen)break;
            }

            /* Receive the load from the server 2*/
            res=NULL;
            reslen =0, bufferlen = 0;
            while(1){
                for(int i=0;i<SZ;i++)buffer[i]='\0';
                
                /*Receive messges in chunks*/
                int rec_len = recv(serv_2_sockfd,buffer,10,0);

                if(rec_len<0){
                    /*Error Handelling*/
                    if(res)free(res);
                    close(serv_2_sockfd);
                    close(sockfd);
                    error("Receive failed from load balancer.");
                }else if(rec_len==0){
                    /*Error Handelling*/
                    if(res)free(res);
                    close(serv_2_sockfd);
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
            /*  convert the load to integer */
            serv_2_load[serv_2_idx] = atoi(res);

            /* free the memory */
            if(res)free(res);

            /* close the socket */
            close(serv_2_sockfd);

            /* message on terminal */
            printf("Load received from server S2 {%s , %d} : %d\n",inet_ntoa(serv_2_addr.sin_addr),ntohs(serv_2_addr.sin_port),serv_2_load[serv_2_idx]);
            
            /* update timeout */
            timeout = TIME_OUT;
        }
        else{ // input from client

            /* Check if the socket has received data */
            if (fds[0].revents & POLLIN)
            {
                /* Accept the connection from client*/
                clilen = sizeof(cli_addr);
                cli_sockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
                if (cli_sockfd < 0){
                    /* Error Handelling */
                    close(sockfd);
                    error("Accept failed");
                }
                
                /* create a child process to handle the client */
                /* fork */
                int pid = fork();
                if (pid < 0){
                    /* Error Handelling */
                    close(cli_sockfd);
                    close(sockfd);
                    error("Fork failed");
                }
                else if (pid == 0){ // child
                    
                    /* close the listening socket of parent */
                    close(sockfd);

                    /*  receive the request from client */
                    char* res=NULL;
                    size_t reslen =0, bufferlen = 0;
                    while(1){
                        for(int i=0;i<SZ;i++)buffer[i]='\0';
                        
                        /*Receive messges in chunks*/
                        int rec_len = recv(cli_sockfd,buffer,10,0);

                        if(rec_len<0){
                            /*Error Handelling*/
                            if(res)free(res);
                            close(cli_sockfd);
                            error("Receive failed from client.");
                        }else if(rec_len==0){
                            /*Error Handelling*/
                            if(res)free(res);
                            close(cli_sockfd);
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
                    /* copy the request to buffer */
                    for(int i=0;i<SZ;i++)buffer[i]='\0';
                    strcpy(buffer,res);
                    if(res)free(res);


                    /* check the min load server and send the request to that server */
                    int serverSock, serverPort;
                    if (serv_1_load[serv_1_idx] < serv_2_load[serv_2_idx])
                    {
                        /* create socket */
                        serverSock = socket(AF_INET, SOCK_STREAM, 0);
                        if (serverSock <0)
                        {
                            /* Error Handelling */
                            close(cli_sockfd);
                            error("Could not create socket");
                        }

                        /* connect to server 1 */
                        if (connect(serverSock, (struct sockaddr *)&serv_1_addr, sizeof(serv_1_addr)) < 0)
                        {
                            /* Error Handelling */
                            close(serverSock);
                            close(cli_sockfd);
                            error("Connect failed");
                        }
                        /* update the server port */
                        serverPort = serv_1_port;

                        /* print the message on terminal */
                        printf("Sending client request to server S1 {%s , %d} \n",inet_ntoa(serv_1_addr.sin_addr),ntohs(serv_1_addr.sin_port));
                    }
                    else
                    {
                        /* create socket */
                        serverSock = socket(AF_INET, SOCK_STREAM, 0);
                        if (serverSock < 0)
                        {
                            /* Error Handelling */
                            close(cli_sockfd);
                            error("Could not create socket");
                        }
                        if (connect(serverSock, (struct sockaddr *)&serv_2_addr, sizeof(serv_2_addr)) < 0)
                        {
                            /* Error Handelling */
                            close(serverSock);
                            close(cli_sockfd);
                            error("Connect failed");
                        }
                        /* update the server port */
                        serverPort = serv_2_port;

                        /* print the message on terminal */
                        printf("Sending client request to server S2 {%s , %d} \n",inet_ntoa(serv_2_addr.sin_addr),ntohs(serv_2_addr.sin_port));
                    }

                    /* send the request to server */
                    bufferlen = strlen(buffer)+1;
                    int idx=0;

                    while (1){
                        /* We send the data in chunks of 10 bytes or length of left string , whichever is minimum)*/
                        int bytessend = send(serverSock,buffer+idx,min((int)strlen(buffer+idx)+1,10),0);
                        
                        if(bytessend<0){
                            /*Error handelling*/
                            close(cli_sockfd);
                            close(serverSock);
                            error("Unable to send data to server.");
                        }else if(bytessend==0){
                            /*Error Handelling*/
                            close(cli_sockfd);
                            close(serverSock);
                            error("Client closed connection.\n");
                        }

                        idx+=bytessend;
                        if(idx>=bufferlen)break;
                    }

                    /* receive the response from server */
                    res=NULL;
                    reslen =0, bufferlen = 0;
                    while(1){
                        for(int i=0;i<SZ;i++)buffer[i]='\0';
                        
                        /*Receive messges in chunks*/
                        int rec_len = recv(serverSock,buffer,10,0);

                        if(rec_len<0){
                            /*Error Handelling*/
                            if(res)free(res);
                            close(cli_sockfd);
                            close(serverSock);
                            error("Receive failed from server.");
                        }else if(rec_len==0){
                            /*Error Handelling*/
                            if(res)free(res);
                            close(cli_sockfd);
                            close(serverSock);
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

                    /* send the response(Date and Time) to client */
                    for(int i=0;i<SZ;i++)buffer[i]='\0';
                    strcpy(buffer,res);
                    if(res)free(res);

                    /* send the request to server */
                    bufferlen = strlen(buffer)+1;
                    idx=0;

                    while (1){
                        /* We send the data in chunks of 10 bytes or length of left string , whichever is minimum)*/
                        int bytessend = send(cli_sockfd,buffer+idx,min((int)strlen(buffer+idx)+1,10),0);
                        
                        if(bytessend<0){
                            /*Error handelling*/
                            close(cli_sockfd);
                            close(serverSock);
                            error("Unable to send data to client.");
                        }else if(bytessend==0){
                            /*Error Handelling*/
                            close(cli_sockfd);
                            close(serverSock);
                            error("Client closed connection.\n");
                        }

                        idx+=bytessend;
                        if(idx>=bufferlen)break;
                    }
                    /*  close the socket */
                    close(cli_sockfd);
                    close(serverSock);

                    // printf("%d\n",serverPort);
                    /* exit the child process */
                    exit(0);
                }
            }
            /* parent process */
            
            /* end time */
            gettimeofday(&end, NULL);
            double time_elapsed_in_ms = (end.tv_sec - start.tv_sec) * 1000.0;
            time_elapsed_in_ms += (end.tv_usec - start.tv_usec) / 1000.0;
            
            /*  update the timeout */
            timeout = timeout - time_elapsed_in_ms;
            if (timeout < 0){
                timeout = 0;
            }
        }
        counter++;
    }

    /* return */
    return 0;
}