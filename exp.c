#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int main(){
    char *a=strdup("/aa/bdda/csa/d/ee/f/"),*b;b=strtok(a,"/");
    while(b!=NULL){printf("s:%s\n",b);b=strtok(NULL,"/");}
    free((void*)a);
    return 0;
}