/*
static void *init_fs(struct fuse_conn_info *conn,struct fuse_config *cfg);
static int getattr_fs(const char *path,struct stat *stbuf,struct fuse_file_info *fi);
static int readdir_fs(const char *path,void *buf,fuse_fill_dir_t filler,off_t offset,\
    struct fuse_file_info *fi,enum fuse_readdir_flags flags);
static int open_fs(const char *path,struct fuse_file_info *fi);
static int read_fs(consta char *path,struct fuse_file_info *fi);
static void destroy_fs(void *private_data);
*/


//structure section--------------------------------------------------------

//structure 4mb , 2kb blocks,2kb supernode,2kb bitmap(inode+dnode)
// 256 bytes inode and dnode therefore 8 inode in each block
// 254 blocks for inode and rest for datanode


struct fs_str *fs;
FILE *cont;
FILE *logger;
char *lbuffer,*loffset;

struct spblk{
    char name[100];
    char format[100];
    int inodefilled;
    int inodetotal;
    int dnodefilled;
    int dnodetotal;
    //other things
    char none[1832];
};
struct inode{
    short int dat[64];
    short int n;short int filld;
    int links;
    int uid;
    int gid;
    int perm;
    int type;
    short int parent;
    //we will fill other fields later
    char none[102];
};

struct dnode{
    char d[256];
};

struct fs_str{
    struct spblk sb;
    char ibit[256];
    char dbit[1792];
    struct inode ind[2032];
    struct dnode dnd[14336];
};


//utility functions section -----------------------------------------------------


int getsize(int indn){
// function to get size of an inode
    struct inode *i;i=fs->ind+indn;
    if(i->n>64)return 64*256+getsize((int)i->filld);
    return (64*(i->n-1)+i->filld);
}

char *inode_dat(int inod){
//function to get data of inode to char array
struct inode *a;char *d;int i,j,k,sz;a=fs->ind+inod;
sz=getsize(inod);
d=(char*)malloc(sizeof(char)*(sz+1));
k=0;
while(1){
if(a->n>64){
    for(i=0;i<64;i++,k+=256)memcpy(d+k,(fs->dnd[a->dat[i]].d),256);a=fs->ind+a->filld;
}
else{
    for(i=0;i<(a->n-1);i++,k+=256)memcpy(d+k,(fs->dnd[a->dat[i]].d),256);
    memcpy(d+k,(fs->dnd[a->dat[i]].d),a->filld);
    break;
}
}d[sz]='\0';return d;
}



int pathtoinode(const char *path){ char *a,*b,*c;int i,fl;
//function to convert path to inode number return -1 if not present
//still needs some revision
	//i=sprintf(loffset,"path to inode called on %s\n",path);loffset+=i;
    if(strcmp("/",path)==0)return 0;path=strdup(path);
    a=strtok((char*)path,"/");fl=0;
    while(a!=NULL){
        if(fs->ind[fl].type!=1){free((void*)path);return -1;}
        b=c=inode_dat(fl);
        while(1){
        for(i=0;a[i]!='\0'&&c[i]!='\0';i++)if(a[i]!=c[i])break;
        if(a[i]=='\0'&&c[i]==' '){sscanf(c+i+1,"%d",&fl);a=strtok(NULL,"/");break;}
        while((c[i]!='\0')&&(c[i]!=','))i++;
        if(c[i]=='\0'){free((void*)path);free((void*)b);return -1;}
        else i++;c+=i;
        }free((void*)b);
    }
    free((void*)path);
    return fl;
}

void rmbit(int a,int t){
    char *b;printf("rm:%d t:%d\n",a,t);
    if(t==0){
        b=fs->dbit;b[a/8]&=~(1<<(a%8));
        return;
    }
    if(t==1){
        b=fs->ibit+2;b[a/8]&=~(1<<(a%8));
        return;
    }
}

int getfree(int t){
    char *b;int i,j;
    if(t==0){
        b=fs->dbit;for(i=0;i<1792;i++)if(b[i]!=-1)break;
        if(i==1792)return -1;
        for(j=0;j<8;j++)if(b[i]&(1<<j))break;b[i]|=(1<<j);return i*8+7;
    }
    if(t==1){
        b=fs->ibit+2;for(i=2;i<256;i++)if(b[i]!=-1)break;
        if(i==1792)return -1;
        for(j=0;j<8;j++)if(b[i]&(1<<j))break;b[i]|=(1<<j);return i*8+7;
    }
}

void deldat(int ino){
    struct inode *e;int i;
    e=&fs->ind[ino];printf("a:%d n:%d\n",ino,e->n);
    if(e->n>64){deldat(e->filld);e->n=64;rmbit(e->filld,1);}
    for(i=0;i<e->n;i++)rmbit(e->dat[i],0);
    e->n=0;
}

void inode_write(int ino,char *d,int sz){
    struct inode *e,*f;int i,j,k;char *a;
    //deldat(ino);
    e=fs->ind+ino;e->n=0;
    if(sz>(64*256)){e->filld=getfree(1);e->n=65;
    f=fs->ind+e->filld;f->links=1;f->parent=-1;f->filld=0;f->n=0;
    inode_write(e->filld,d+256*64,sz-256*64);sz=256*64;
    }
    j=sz/256;k=sz%256;printf("wr:%d %d %d\n",j,k,sz);
    for(i=0;i<j;i++){e->dat[i]=getfree(0);memcpy((void*)fs->dnd[e->dat[i]].d,d+256*i,256);}
    e->dat[j]=getfree(0);memcpy(fs->dnd[e->dat[j]].d,d+256*j,k);if(e->n==0){e->n=j+1;e->filld=k;}
}

// init function -----------------------------------------

static void addextra(){
    struct inode *id;struct dnode *dn;
    id=fs->ind+0;dn=fs->dnd+0;strcpy(dn->d,". 0,.. 0,a.txt 1,b 2");id->filld=20;
    id=fs->ind+1;dn=fs->dnd+1;strcpy(dn->d,"my first data");id->filld=13;id->parent=0;
    id->n=1;id->dat[0]=1;id->links=1;id->uid=id->gid=0;id->perm=-1;id->type=0;
    id=fs->ind+2;dn=fs->dnd+2;strcpy(dn->d,". 2,.. 2,a2.txt 3");id->filld=17;id->parent=0;
    id->n=1;id->dat[0]=2;id->links=2;id->uid=id->gid=0;id->perm=-1;id->type=1;
    id=fs->ind+3;dn=fs->dnd+3;strcpy(dn->d,"second data");id->filld=11;id->parent=2;
    id->n=1;id->dat[0]=3;id->links=1;id->uid=id->gid=0;id->perm=-1;id->type=0;
    fs->dbit[0]=15;fs->ibit[2]=15;
}


static void init_fs(){
    //to initially init filesystem
    int fz=4*1024*1024;struct spblk *sb;struct inode *id;struct dnode *dn;char *b;int k;
    fs=(struct fs_str*)malloc(sizeof(char)*fz);  //allocate memory
    lbuffer=(char*)malloc(sizeof(char)*1024*1024);
    loffset=lbuffer+sprintf(lbuffer,"init called\n");
    memset((void*)fs,0,fz);                 loffset=lbuffer; //set memory to zero
    cont=fopen("cont.txt","rb");logger=fopen("log2.txt","wb");
    if(1||cont==NULL){                         //if file not found create fs
        sb=&fs->sb;
        cont=fopen("cont.txt","wb");
        strcpy(sb->name,"myfs");strcpy(sb->format,"myformat");
        sb->inodefilled=sb->dnodefilled=1;
        sb->inodetotal=2032;sb->dnodetotal=14336;
        b=fs->ibit;b[0]=b[1]=-1;b[2]=1;   //set to filled as first two blocks are filled
        b=fs->dbit;b[0]=1;        //only one datanode (root directory) is filled
        id=&fs->ind[0];
        id->dat[0]=0;
        id->n=1;id->parent=-1;           //total filled
        id->filld=8;        //size of the dir file
        id->type=1;                 //assuming 1 for directories
        id->links=2;
        id->uid=id->gid=0;id->perm=-1; //dont know what to fill
        dn=&fs->dnd[0];
        strcpy(dn->d,". 0,.. 0");
        addextra();
    }
    else {              //if file already present then read previous contents
        fread((void*)fs,1,fz,cont);fclose(cont);//cont=fopen("cont.txt","wb");//fseek(cont,0,SEEK_SET);
    }
}


//main functions section-------------------------------------------------------------


static void destroy_fs(void *pd){
    int fz=4*1024*1024;fwrite((void*)fs,1,fz,cont);fclose(cont);
    fwrite((void*)lbuffer,1,loffset-lbuffer,logger);
    fclose(logger);
    free((void*)fs);free((void*)lbuffer);
}

static int getattr_fs(const char *path,struct stat *st,struct fuse_file_info *fi){
    int i,j,k;
    struct inode *e;
    i=sprintf(loffset,"getattr called on %s\n",path);loffset+=i;
    memset((void*)st, 0, sizeof(struct stat));
    i=pathtoinode(path);
    if(i==-1)return -ENOENT;
    e=&fs->ind[i];
    if(e->type==1){st->st_nlink=2;st->st_mode=S_IFDIR | 0755;}
    else {st->st_mode=S_IFREG | 0444;st->st_size=getsize(i);}
    
    return 0;
}

static int readdir_fs(const char *path, void *buf, fuse_fill_dir_t filler,
             off_t offset, struct fuse_file_info *fi,
			 enum fuse_readdir_flags flags){
    int i,j,k;char *c,*a;
    struct inode *e;
    i=sprintf(loffset,"readdir called on %s\n",path);loffset+=i;
    i=pathtoinode(path);
    if(i==-1)return -ENOENT;
    e=&fs->ind[i];
    if(e->type!=1)return -ENOENT;
    c=inode_dat(i);
    a=strtok(c,",");
    while(a!=NULL){
        j=0;while(a[j]!=' ')j++;
        a[j]='\0';filler(buf,a,NULL,0,0);
        a=strtok(NULL,",");
    }free((void*)c);
    return 0;
}

static int open_fs(const char *path, struct fuse_file_info *fi){
    int i;
    i=sprintf(loffset,"open called on %s\n",path);loffset+=i;//return -ENOENT;
    i=pathtoinode(path);
    if(i==-1)return -ENOENT;
    return 0;
}

static int read_fs(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi){
    int i,j,k,s;struct inode *e;char *d;
    i=sprintf(loffset,"read called on %s size:%ld offset %ld\n",path,size,offset);loffset+=i;//return -ENOENT;
    i=pathtoinode(path);
    if(i==-1)return -ENOENT;
    s=getsize(i);
    if(offset>=s)return 0;
    d=inode_dat(i);
    if(size>s)size=s;
    memcpy(buf,d+offset,size);
    free((void*)d);         // not at all care of optimizations
    return size;
}

static int write_fs(const char *path,const char *dat,size_t size,off_t offset,
                struct fuse_file_info *fi){
    int i,j,k,s;struct inode *e;char *d;
    i=sprintf(loffset,"read called on %s size:%ld offset %ld\n",path,size,offset);loffset+=i;
    i=pathtoinode(path);
    printf("init done\n");
    if(i==-1)return -ENOENT;
    j=getsize(i);
    d=inode_dat(i);
    if((offset+size)>j)d=(char*)realloc((void*)d,sizeof(char)*(j=(offset+size)));
    for(k=0;k<size;k++)d[k+offset]=dat[k];
    printf("funcdone %d,%d\n",i,fs->ind[i].n);
    deldat(i);
    printf("deldone\n");
    inode_write(i,d,j);
    free((void*)d);
    return size;
}

static int mkdir_fs(const char *path, mode_t what){
    int i,j,k,x,sz;struct inode *e;char *a,*d;
    i=sprintf(loffset,"mkdir called on %s\n",path);loffset+=i;
    i=pathtoinode(path);if(i!=-1)return -ENOENT;
    a=strdup(path);j=k=1;while(a[j]!='\0'){if(a[j]=='/')k=j;j++;}
    if(a[k]!='/')i=pathtoinode("/");else{a[k]='\0';k++;i=pathtoinode(a);}
    if(i==-1){free((void*)a);return -ENOENT;}
    sz=getsize(i);//printf("nf:%s\n",a);
    d=inode_dat(i);d=realloc(d,sz+strlen(a+k)+32);x=getfree(1);
    e=fs->ind+x;e->type=1;e->n=0;e->filld=0;e->links=2;e->parent=i;
    j=sprintf(d+sz,",%s %d",a+k,x);printf("d:%s x:%d sz:%d j:%d i:%d\n",d,x,sz,j,i);deldat(i);inode_write(i,d,sz+j);free((void*)a);
}

/*static int unlink_fs(const char *path){
    int i,p,k;struct inode *e;char *a,*b;
    i=sprintf(loffset,"unlink called on %s!",path);loffset+=i;
    return -ENOENT;
    i=pathtoinode(path);
    if(i==-1)return -ENOENT;
    e=fs->ind+i;p=e->parent;
    if(e->type==1)return -ENOENT;
    a=inode_dat(p);b=strtok(a,",");
    while(b!=NULL){
        while((*b++)!=' ');sscanf(b,"%d",&k);
        if(k==i)
    }
}*/

