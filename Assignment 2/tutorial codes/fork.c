#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

int main(){
    // int id=fork();
    // printf("hi id = %d\n",id);
    // printf("hi id = %d\n",id);
    // int tid=fork();
    // // id of child process is zero
    // printf("hi id = %d , tid = %d\n",id,tid);
    // if(id==0){
    //     printf("Hello from child process\n");
    // }else{
    //     printf("Hello from parent process\n");
    // }
    
    // if(fork()&&fork())
    //     fork();
    // printf("Hello\n");
    if(fork()||fork())
        fork();
    printf("Hello\n");

    return 0;
}