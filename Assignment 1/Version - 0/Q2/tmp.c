#include<stdio.h>
#include<stdlib.h>
#include<string.h>

float cal(char *buf,int *i,int len){
    float res=0;
    char op='+';
    while(*i < len){
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
            while( *i < len && ( buf[*i]=='.' || ( buf[*i]>='0' && buf[*i]<='9' ) ) ){
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
    }
    return res;
}



int main(){
    char buf[100];
    for(int i=0;i<100;i++)buf[i]='\0';
    while (1){
        // fgets(buff,100,stdin); // method - 1 : reads till 100 charcters or newline
        scanf("%[^\n]%*c",buf); // method - 2 : string matching : take all input except \n 
        if(!strcmp(buf,"-1"))break;
        float res=0;
        char op='+';
        int i=0;
        int len=strlen(buf);
        printf("%s has value : ",buf);
        res=cal(buf,&i,len);
        printf("%f\n",res);
        printf("len=%d,i=%d\n",len,i);

        /* We initialize the buffer , copy the message to it, and send the message to the client.*/
        /* gcvt() | Convert float value to string in C
           This function is used to convert a floating point number to string.
           Syntax : gcvt (float value, int ndigits, char * buf);
        */
        {// method 1
            char buf[100];
            gcvt(res,5,buf);
            printf("Sending : %s",buf);
        }
        {// method 2
            char buf[100];
            sprintf(buf,"%f",res);
            printf("Sending : %s",buf);
        }
    }
    return 0;
}
