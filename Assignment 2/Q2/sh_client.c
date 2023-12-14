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

/* Function to remove unnecessary spaces from a string (beginning, end, and multiple spaces in between)*/
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

    /*free memory*/
    if(input)free(input);
    return res;

    #undef chunk
}

/* Function to return minimum of two integers*/
int min(int a,int b){
    if(a<b)return a;
    return b;
}

/* Some output Strings were of length more than 50, So, AG sir advised to take length > 50 */
#define SZ 200

                    /*Client Process*/
int main(){
    int sockfd; /*socket descriptors*/
    struct sockaddr_in serv_addr;

    /* the following syatem call opens a socket.*/
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        /*Error Handelling*/
        error("Unable to create socket");
    }

    /* defining structure sockaddr_in for the  internet family of protocols. */    
    serv_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1",&serv_addr.sin_addr);
    serv_addr.sin_port = htons(20000);

    /* With information in serv_addr,connect() system call establishes a connection with  server process.*/
    if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){
        /*Error Handelling*/
        close(sockfd);
        error("Unable to connect to the server");
    }
    printf("-- Client connected to server.\n");
    
    int i=0;
    char buffer[SZ]; /*message buffer*/
    char* res=NULL;
    
    /*Recieve "Login:" Prompt from server.*/
    size_t reslen =0, bufferlen = 0;
    while(1){
        
        for(int i=0;i<SZ;i++)buffer[i]='\0';

        /*receive message in chunks*/
        int rec_len = recv(sockfd,buffer,10,0);// < 50
        if(rec_len<0){
            /*Error Handelling*/
            if(res)free(res);
            close(sockfd);
            error("Unable to recieve data from server");
        }else if(rec_len==0){
            /*Error Handelling*/
            if(res)free(res);
            close(sockfd);
            error("Server closed connection.\n");
        }

        /*append the received string in the end of buffer res in O(length(bufferlen)) time*/
        bufferlen = strlen(buffer);
        res = realloc(res, reslen+bufferlen+1);
        strcpy(res + reslen, buffer);
        reslen += bufferlen;
        
        /*Stop receiving when you get '\0' in the end*/
        if(buffer[rec_len-1]=='\0'){break;}
    }

    /*message("Login:") to be displayed to user*/
    printf("%s",res);
    
    /* Read username from user*/
    char* username=NULL;
    username=readinputstring();
    
    /* Send username to server*/
    /* I'm taking username as 'kind of password', so I'm not removing extra spaces which might be present in the beginning or end of the string */
    size_t usernamelen = strlen(username)+1;
    int idx=0;
    while (1){
        /* We send the data in chunks of 10 bytes or length of left string , whichever is minimum)*/
        int bytessend = send(sockfd,username+idx,min((int)strlen(username+idx)+1,10),0);// < 50
        if(bytessend<0){
            /*Error Handelling*/
            if(res)free(res);
            if(username)free(username);
            close(sockfd);
            error("Unable to send data to server");
        }else if(bytessend==0){
            /* Error Handelling*/
            if(res)free(res);
            if(username)free(username);
            close(sockfd);
            error("Server closed Connection.\n");
        }
        idx+=bytessend;
        if(idx>=usernamelen)break;
    }

    /* free memory */
    if(username)free(username);

    /*Receive "Found"/"NotFound" username Prompt from server*/
    reslen =0, bufferlen = 0;
    while(1){
        for(int i=0;i<SZ;i++)buffer[i]='\0';

        /*receive message in chunks*/
        int rec_len = recv(sockfd,buffer,10,0);// < 50
        if(rec_len<0){
            /*Error Handelling*/
            if(res)free(res);
            close(sockfd);
            error("Unable to recieve data from server");
        }else if(rec_len==0){
            /*Error Handelling*/
            if(res)free(res);
            close(sockfd);
            error("Server closed connection.\n");
        }

        /*append the received string in the end of buffer res in O(length(bufferlen)) time*/
        bufferlen = strlen(buffer);
        res = realloc(res, reslen+bufferlen+1);
        strcpy(res + reslen, buffer);
        reslen += bufferlen;
        
        /*Stop receiving when you get '\0' in the end*/
        if(buffer[rec_len-1]=='\0'){break;}
    }

    /* If username id not found in servers's list*/
    if(strcmp(res,"NOT-FOUND")==0){
        if(res)free(res);
        close(sockfd);
        printf("Invalid username.\n");
        exit(0);
    }

    printf("Enter the shell commands to be executed.\n");
    printf("Only supports pwd , dir and cd commands.\n");
    printf("Type 'exit' to exit client.\n\n");

    /* Username is in Server's list*/
    while(1){

        /*Ask user to give a shell command*/
        printf("shell_command> ");
        char* shellcommand=NULL;
        shellcommand = readinputstring(); /*read input*/

        /* send shellcommand to server */
        size_t shellcommandlen = strlen(shellcommand)+1;
        int idx=0;
        while (1){
            /* We send the data in chunks of 10 bytes or length of left string , whichever is minimum)*/
            int bytessend = send(sockfd,shellcommand+idx,min((int)strlen(shellcommand+idx)+1,10),0);// < 50
            if(bytessend<0){
                /*Error Handelling*/
                if(res)free(res);
                if(shellcommand)free(shellcommand);
                close(sockfd);
                error("Unable to send data to server");
            }else if(bytessend==0){
                /* Error Handelling*/
                if(res)free(res);
                if(shellcommand)free(shellcommand);
                close(sockfd);
                error("Server closed Connection.\n");
            }
            idx+=bytessend;
            if(idx>=shellcommandlen)break;
        }

        /* remove spaces from shellcommand */
        char* shell_command=NULL;
        shell_command = removeSpaces(shellcommand);

        /*Free memory*/
        if(shellcommand)free(shellcommand);

        /* If user enters "exit" command, exit the program*/
        if(strcmp(shell_command,"exit")==0){
            if(shell_command)free(shell_command);
            printf("Exiting client.\n");
            break;
        }

        /*Free memory*/
        if(shell_command)free(shell_command);

        /*Recieve output of shell command from server*/
        reslen =0, bufferlen = 0;
        while(1){
            
            for(int i=0;i<SZ;i++)buffer[i]='\0';

            /*receive message in chunks*/
            int rec_len = recv(sockfd,buffer,10,0);// < 50
            if(rec_len<0){
                /*Error Handelling*/
                if(res)free(res);
                close(sockfd);
                error("Unable to recieve data from server");
            }else if(rec_len==0){
                /*Error Handelling*/
                if(res)free(res);
                close(sockfd);
                error("Server closed connection.\n");
            }

            /*append the received string in the end of buffer res
                in O(length(bufferlen)) time*/
            bufferlen = strlen(buffer);
            res = realloc(res, reslen+bufferlen+1);
            strcpy(res + reslen, buffer);
            reslen += bufferlen;
            
            /*Stop receiving when you get '\0' in the end*/
            if(buffer[rec_len-1]=='\0'){break;}
        }

        /*Print output of shell command*/
        if(strcmp(res,"$$$$")==0){
            printf("Invalid command.\n\n");
        }else if(strcmp(res,"####")==0){
            printf("Error in running command.\n\n");
        }else{
            printf("%s\n\n",res);
        }

    }

    /* free memory */
    if(res)free(res);

    /* the following system call closes the socket.*/
    close(sockfd); 
    return 0;
}
