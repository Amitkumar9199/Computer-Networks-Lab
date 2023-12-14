/*Important Library*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdbool.h>
#include <dirent.h> /* for directory operations */

/*Function to give error msg to user, and exit*/
void error(const char *msg){
    perror(msg);
    exit(1);
}

/* Function to remove unnecessary spaces from a  string (beginning, end, and multiple spaces in between)*/
char* removeSpaces(char* str){

    char* res = (char*)malloc(strlen(str)+1);
    int i=0,j=0;

    /* remove spaces from beginning*/
    while(str[i]==' '){i++;}

    /* remove spaces from  between*/
    while(str[i]!='\0'){ 
        if(str[i]!=' '){
            res[j]=str[i];
            j++;
        }else{
            res[j]=' ';
            j++;
            while(str[i]==' '){
                i++;
            }
            i--;
        }
        i++;
    }

    /* remove spaces from end*/
    while((j-1>=0) && (res[j-1]==' '))j--; 
    
    /* add '\0' at end*/
    res[j]='\0'; 
    
    /* copy res to res2*/
    char *res2 = (char*)malloc(strlen(res)+1);
    strcpy(res2,res);
    
    /* free res*/
    if(res)free(res);

    return res2;
}

/* returns the minimum of two integers*/
int min(int a,int b){
    if(a<b)return a;
    return b;
}

/* Some output Strings were of length more than 50, So, AG sir advised to take length > 50 */
#define SZ 200

                    /*Server Process*/
int main(){
    int sockfd, newsockfd;/*Socket Descriptors*/
    int clilen;
    struct sockaddr_in serv_addr,cli_addr;
    
    /* The following system call opens a socket.*/
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        /*Error Handelling*/
        error("Cannot create socket");
    }

    /* defining structure sockaddr_in for the internet family of protocols. */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(20000);

    /* With the information provided in serv_addr ,we can associate the server with its port using  the bind() system call */
    if(bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){
        /*Error Handelling*/
        close(sockfd);
        error("Unable to bind local address");
    }

    /* The listen system call allows the process to listen on the socket for connections.*/
    if(listen(sockfd,5)<0){
        /*Error Handelling*/
        close(sockfd);
        error("Listen failed");
    }
    printf("-- The Server is ready...\n");


    /*Concurrent server -- one which forks to accept multiple client connections concurrently. As soon as the server accepts a connection from a client, it forks a child which communicates with the client, while the parent becomes free to accept a new connection. To facilitate this, the accept() system call returns a new socket descriptor which can be used by the child. The parent continues with the original socket descriptor.*/
    while(1){

        /* The accepth call accepts the client connection. It blocks the server until a client request comes. */
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd,(struct sockaddr *)&cli_addr,&clilen);
        if(newsockfd<0){
            /*Error Handelling*/
            close(sockfd);
            error("Accept failed");
        }
        printf("-- New client connected from %s : %d\n",inet_ntoa(cli_addr.sin_addr),ntohs(cli_addr.sin_port));

        /* Having successfully accepted a client connection, the server now forks. The paretn closes the new socket descriptor and goes back to the top of the loop to accept another connection. The child continues to process the client request. */
        if(fork()==0){
            /*Child Process*/

            /* Close the old socket since all communications will be through the new socket. */
            close(sockfd);

            char buffer[SZ];

            /*The server sends the string “LOGIN:” to the client*/
            for(int i=0;i<SZ;i++)buffer[i]='\0';
            strcpy(buffer,"LOGIN:");
            size_t bufferlen = strlen(buffer)+1;
            int idx=0;
            while (1){
                /* We send the data in chunks of 10 bytes or length of left string , whichever is minimum)*/
                int bytessend = send(newsockfd,buffer+idx,min((int)strlen(buffer+idx)+1,10),0);// < 50
                if(bytessend<0){
                    /*Error Handelling*/
                    close(newsockfd);
                    error("Unable to send data to server");
                }else if(bytessend==0){
                    /*Error Handelling*/
                    close(newsockfd);
                    error("Client closed connection.\n");
                }
                idx+=bytessend;
                if(idx>=bufferlen)break;
            }

            /*Receive the username from the client.*/
            char* res=NULL;
            size_t reslen =0; bufferlen = 0;
            while(1){
                for(int i=0;i<SZ;i++)buffer[i]='\0';
                
                /*Receive messges in chunks*/
                int rec_len = recv(newsockfd,buffer,10,0);// < 50
                if(rec_len<0){
                    /*Error Handelling*/
                    if(res)free(res);
                    close(newsockfd);
                    error("receive failed");
                }else if(rec_len==0){
                    /*Error Handelling*/
                    if(res)free(res);
                    close(newsockfd);
                    error("Client closed connection.\n");
                }

                /*append the received string to res in O(length(buffer)) time*/
                bufferlen = strlen(buffer);
                res = realloc(res, reslen+bufferlen+1);
                strcpy(res+reslen, buffer);
                reslen += bufferlen;
                
                /*Stop receiving when you get '\0' in end */
                if(buffer[rec_len-1]=='\0')break;
            }

            /* Username received from client */
            printf("Username : %s\n",res);

            /* Server checks if username is in users.txt file */
            /*Open file*/
            FILE *fp = fopen("users.txt","r");
            if(fp==NULL){
                /*Error Handelling*/
                if(res)free(res);
                close(newsockfd);
                error("Unable to open file");
            }
            char* name = NULL;
            size_t namelen = 0; ssize_t read; bool found = 0;
            /*Read names in file*/            
            while ((read = getline(&name, &namelen, fp)) != -1) {
                if(name[strlen(name)-1]=='\n'){
                    name[strlen(name)-1]='\0';
                }
                if(strcmp(name,res)==0){
                    found = 1;
                    break;
                }
            }

            /* close file*/
            fclose(fp);
            
            /* free pointer*/
            if(name)free(name);
            
            for(int i=0;i<SZ;i++)buffer[i]='\0';
            if(found){
                printf("Username FOUND in file\n");
                strcpy(buffer,"FOUND");
            }else{
                printf("Username NOT-FOUND in file\n");
                strcpy(buffer,"NOT-FOUND");
            }

            /*Send the result(FOUND,NOT-FOUND) to client*/
            bufferlen = strlen(buffer)+1;
            idx=0;
            while (1){
                /* We send the data in chunks of 10 bytes or length of left string , whichever is minimum)*/
                int bytessend = send(newsockfd,buffer+idx,min((int)strlen(buffer+idx)+1,10),0);// < 50
                if(bytessend<0){
                    /*Error handelling*/
                    if(res)free(res);
                    close(newsockfd);
                    error("Unable to send data to server");
                }else if(bytessend==0){
                    /*Error Handelling*/
                    if(res)free(res);
                    close(newsockfd);
                    error("Client closed connection.\n");
                }
                idx+=bytessend;
                if(idx>=bufferlen)break;
            }

            /*  if username id found then answer client's queries */
            while(found){

                /* Receive Shell Command from client */
                reslen =0; bufferlen = 0;
                while(1){
                    for(int i=0;i<SZ;i++)buffer[i]='\0';
                    
                    /*Receive messges in chunks*/
                    int rec_len = recv(newsockfd,buffer,10,0);// < 50
                    if(rec_len<0){
                        /*Error Handelling*/
                        if(res)free(res);
                        close(newsockfd);
                        error("receive failed");
                    }else if(rec_len==0){
                        /*Error Handelling*/
                        if(res)free(res);
                        close(newsockfd);
                        error("Client closed connection.\n");
                    }

                    /*append the received string to res in O(length(buffer)) time*/
                    bufferlen = strlen(buffer);
                    res = realloc(res, reslen+bufferlen+1);
                    strcpy(res+reslen, buffer);
                    reslen += bufferlen;
                    
                    /*Stop receiving when you get '\0' in end */
                    if(buffer[rec_len-1]=='\0'){
                        break;
                    }

                }

                char *shell_command=NULL;
                shell_command = removeSpaces(res); /* remove unwanted spaces from command */
                printf("Shell Command : %s\n",shell_command);

                /* Execute the shell command */

                /* if command is exit then break the loop, client wants to disconnect */
                if(strcmp(shell_command,"exit")==0){
                    printf("Client exited\n");
                    break;
                }
                
                /* if command is "pwd" then get the current working directory */
                else if(strcmp(shell_command,"pwd")==0){

                    /* get current working directory */
                    char* pwd = getcwd(NULL,0);
                    
                    if(pwd==NULL){
                        /*Error Handelling*/
                        if(res)free(res);
                        if(shell_command)free(shell_command);
                        close(newsockfd);
                        error("Unable to get current working directory");
                    }
                    
                    for(int i=0;i<SZ;i++)buffer[i]='\0';
                    strcpy(buffer,pwd);

                    /* free memory */
                    if(pwd)free(pwd);
                }


                /* if command is "dir" then get the list of files in current directory */
                else if(strcmp(shell_command,"dir")==0){
                    /* d is the directory pointer */
                    DIR *d;

                    /* dirr is the directory entry pointer */
                    struct dirent *dirr;

                    /* open current directory */
                    d = opendir(".");

                    /* if directory is opened successfully */
                    if (d){
                        int i=0;
                        
                        /* read all the entries in the directory */
                        while ((dirr = readdir(d)) != NULL){
                            /* ignore . and .. */
                            // if(strcmp(dirr->d_name,".")==0 || strcmp(dirr->d_name,"..")==0)continue;
                            
                            /* append the file name to buffer */
                            if(i==0){
                                strcpy(buffer,dirr->d_name);
                            }else{
                                strcat(buffer," ");
                                strcat(buffer,dirr->d_name);
                            }
                            i++;
                        }

                        /* close the directory */
                        closedir(d);
                    }
                    else{ /* if directory is not opened successfully */
                        /*Error Handelling*/
                        if(res)free(res);
                        if(shell_command)free(shell_command);
                        close(newsockfd);
                        error("Unable to open directory");
                    }
                }

                /* if command is "dir directory_name" then get the list of files in directory */
                else if(strncmp(shell_command,"dir ",4)==0){
                    /*  get the directory name */
                    char* dir = shell_command+4;

                    /*  d is the directory pointer */
                    DIR *d;

                    /* dirr is the directory entry pointer */
                    struct dirent *dirr;

                    /* open the directory */
                    d = opendir(dir);
                    
                    /* if directory is opened successfully */
                    if (d){
                        int i=0;
                    
                        /* read all the entries in the directory */
                        while ((dirr = readdir(d)) != NULL){
                            /* ignore . and .. */
                            // if(strcmp(dirr->d_name,".")==0 || strcmp(dirr->d_name,"..")==0)continue;
                            
                            /* append the file name to buffer */
                            if(i==0){
                                strcpy(buffer,dirr->d_name);
                            }else{
                                strcat(buffer," ");
                                strcat(buffer,dirr->d_name);
                            }
                            i++;
                        }
                    
                        /* close the directory */
                        closedir(d);

                    }else{/* if directory is not opened successfully */                        
                        int n = strlen(dir)-1;
                        while(n>=0 && dir[n]!='/')n--;
                        
                        /* check if it was the file in any given directory */
                        if(n>=0){
                            /* get the file name */
                            char* file2=dir+n+1;

                            /* get the directory name */
                            dir[n]='\0';
                            
                            /* open the directory */
                            d = opendir(dir);
                            
                            if (d){ /* if directory is opened successfully */
                                
                                strcpy(buffer,"No such file exist in ");
                                strcat(buffer,dir);
                                strcat(buffer," directory.");
                                
                                /* check ,if such file exist */
                                while ((dirr = readdir(d)) != NULL){
                                    /* copy 'file location' name if exist */
                                    if(strcmp(dirr->d_name,file2)==0){
                                        strcpy(buffer,dir);
                                        strcat(buffer,"/");
                                        strcat(buffer,dirr->d_name);
                                        break;
                                    }
                                }
                                closedir(d);
                            }else{
                                strcpy(buffer,"####");
                            }
                        }
                        /* check if it was the file in in current directory */
                        else{
                            d = opendir(".");
                            strcpy(buffer,"####");
                            
                            /* read all the entries in the directory, and copy file name if exist */
                            if (d){
                                /* check ,if file exist */
                                while ((dirr = readdir(d)) != NULL){
                                    /* copy file name , if exist */
                                    if(strcmp(dirr->d_name,dir)==0){
                                        strcpy(buffer,dirr->d_name);
                                        break;
                                    }
                                }
                                closedir(d);
                            }
                            else{ /* if directory is not opened successfully */
                                /*Error Handelling*/
                                if(res)free(res);
                                if(shell_command)free(shell_command);
                                close(newsockfd);
                                error("Unable to open directory");
                            }

                        }
                    }
                }

                /* if command is "cd" or "cd ~" then change the current directory to 'home/user_name' directory */
                else if(strcmp(shell_command,"cd")==0 || strcmp(shell_command,"cd ~")==0){
                    /* get the home directory */
                    char* dir = getenv("HOME");

                    /* change the current directory */
                    int ret = chdir(dir);

                    /* if directory is changed successfully */
                    if(ret==0){
                        strcpy(buffer,"Directory changed to '");
                        strcat(buffer,dir);
                        strcat(buffer,"' successfully.");
                    }else{
                        /* if directory is not changed successfully */
                        strcpy(buffer,"####");
                    }
                }
                /* if command is "cd directory_name" then change the current directory */
                else if(strncmp(shell_command,"cd ",3)==0){
                    /* get the directory name */
                    char* dir = shell_command+3;

                    /* change the current directory */
                    int ret = chdir(dir);

                    /* if directory is changed successfully */
                    if(ret==0){
                        strcpy(buffer,"Directory changed to '");
                        strcat(buffer,dir);
                        strcat(buffer,"' successfully.");
                    }else{
                        /* if directory is not changed successfully */
                        strcpy(buffer,"####");
                    }
                }
                /* if command is invalid */
                else{
                    strcpy(buffer,"$$$$");
                }

                /* free memory */
                if(shell_command)free(shell_command);

                /* Send the result to client */
                bufferlen = strlen(buffer)+1;
                idx=0;
                while (1){
                    /* We send the data in chunks of 10 bytes or length of left string , whichever is minimum)*/
                    int bytessend = send(newsockfd,buffer+idx,min((int)strlen(buffer+idx)+1,10),0);// < 50
                    if(bytessend<0){
                        /*Error handelling*/
                        if(res)free(res);
                        close(newsockfd);
                        error("Unable to send data to server");
                    }else if(bytessend==0){
                        /*Error Handelling*/
                        if(res)free(res);
                        close(newsockfd);
                        error("Client closed connection.\n");
                    }
                    idx+=bytessend;
                    if(idx>=bufferlen)break;
                }
            }

            /*free memory.*/
            if(res)free(res);

            /*close the socket.*/ 
            close(newsockfd);
            
            /* exit the child process */
            exit(0);
        }

        /* the following system call closes the socket.*/
        close(newsockfd);
    }

    /* the following system call closes the socket.*/
    close(sockfd);
    return 0;
}
