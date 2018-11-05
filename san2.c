#define FUSE_USE_VERSION 31

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<fcntl.h>
#include<stddef.h>
#include<assert.h>
#include<fuse3/fuse.h>
#include"mfs.h"

///reads the present state only
int kfill (void *buf, const char *name,
				const struct stat *stbuf, off_t off,
				enum fuse_fill_dir_flags flags){
                    printf("bufferfill:%s\n",name);
                }

void rinit(){fs=malloc(sizeof(char)*(1024*1024*4));cont=fopen("cont.txt","rb");
fread((void*)fs,1,4*1024*1024,cont);fclose(cont);logger=fopen("logger.txt","w");
lbuffer=malloc(sizeof(char)*(1024*1024));loffset=lbuffer;
}

int main(){int r;char mybuf[4096],*a;struct stat st;
    rinit();
    printf("init done\n");
    readdir_fs("/",mybuf,kfill,0,NULL,0);
    rename_fs("/a.txt","/a2.txt",0);
    readdir_fs("/",mybuf,kfill,0,NULL,0);
    free((void*)fs);free((void*)lbuffer);//fclose(cont);
    fclose(logger);
    return 0;
}