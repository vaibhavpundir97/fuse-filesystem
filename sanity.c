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
int kfill (void *buf, const char *name,
				const struct stat *stbuf, off_t off,
				enum fuse_fill_dir_flags flags){
                    printf("bufferfill:%s\n",name);
                }
int main(){int r;char mybuf[4096],*a;struct stat st;
    init_fs();
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
    destroy_fs(NULL);
    //free((void*)fs);//fclose(cont);fclose(logger);
    return 0;
}