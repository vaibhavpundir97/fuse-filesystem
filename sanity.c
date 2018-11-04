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
    r=getattr_fs("/",&st,NULL);
    printf("getattr of / done. return value:%d mode:%d nlink:%ld\n",r,st.st_mode,st.st_nlink);
    r=readdir_fs("/b",mybuf,kfill,0,NULL,0);
    printf("readdir of / done with return value:%d\n",r);
    r=open_fs("/",NULL);
    printf("open of / done with return value:%d\n",r);
    r=read_fs("/",mybuf,20,0,NULL);
    printf("read of / done with return value:%d\n",r);mybuf[20]='\0';
    printf("data output by read is:%s\n",mybuf);
//read root done
    r=getattr_fs("/abc",&st,NULL);
    printf("getattr of /abc done. return value:%d mode:%d nlink:%ld\n",r,st.st_mode,st.st_nlink);
    printf("inode of path :%d\n",pathtoinode("/a2.txt"));
    a=inode_dat(0);printf("%s :%d\n",a,getsize(0));free((void*)a);
    a=inode_dat(1);printf("%s :%d\n",a,getsize(1));free((void*)a);
    a=inode_dat(2);printf("%s :%d\n",a,getsize(2));free((void*)a);
    a=inode_dat(3);printf("%s :%d\n",a,getsize(3));free((void*)a);
    write_fs("/a.txt","wwwwwhhsvhvwwwwwwwww",6,10,NULL);
    a=inode_dat(1);printf("%s :%d\n",a,getsize(1));free((void*)a);
    printf("mkdir calling\n");
    mkdir_fs("/d",0);
    r=readdir_fs("/",mybuf,kfill,0,NULL,0);
    printf("readdir of / done with return value:%d\n",r);
    printf("sz:%d\n",getsize(0));
    unlink_fs("/a.txt");
    //printf("stat:%d",create_fs("/b/e.txt",0,NULL));
    create_fs("/e.txt",0,NULL);printf("create done\n");
    //write_fs("/a.txt","abcdefg",7,0,NULL);
    a=inode_dat(0);printf("%s :%d,e:%d\n",a,getsize(0),pathtoinode("/e.txt"));free((void*)a);
    r=readdir_fs("/",mybuf,kfill,0,NULL,0);
    write_fs("/e.txt","abcdwer",7,0,NULL);
    //destroy_fs(NULL);
    free((void*)fs);free((void*)lbuffer);//fclose(cont);
    fclose(logger);
    return 0;
}