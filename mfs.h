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
    //we will fill other fields later
    char none[104];
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
    return (64*(i->n)+i->filld);
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
    if(strcmp("/",path)==0)return 0;path=strdup(path);
    a=strtok((char*)path,"/");fl=0;
    while(a!=NULL){printf("searching for %s\n",a);printf("p:%p\n",&fs->ind[fl].type);
        if(fs->ind[fl].type!=1){free((void*)path);printf("not directory\n");return -1;}
        c=inode_dat(fl);printf("path list of fl:%s\n",c);
        while(1){
        for(i=0;a[i]!='\0';i++)if(a[i]!=c[i])break;
        if(a[i]=='\0'&&c[i]==' '){sscanf(c+i,"%d",&fl);a=strtok(NULL,"/");break;}
        while((c[i]!='\0')&&c[i]!=',')i++;
        if(c[i]=='\0'){free((void*)path);free((void*)c);return -1;}
        free((void*)c);
        }
    }
    free((void*)path);
    return fl;
}


// init function -----------------------------------------

static void init_fs(){
    //to initially init filesystem
    int fz=4*1024*1024;struct spblk *sb;struct inode *id;struct dnode *dn;char *b;int k;
    fs=(struct fs_str*)malloc(sizeof(char)*fz);  //allocate memory
    lbuffer=(char*)malloc(sizeof(char)*1024*1024);
    loffset=lbuffer+sprintf(lbuffer,"init called ");
    memset((void*)fs,0,fz);                 loffset=lbuffer; //set memory to zero
    cont=fopen("cont.txt","rb");logger=fopen("log2.txt","wb");
    if(cont==NULL){                         //if file not found create fs
        sb=&fs->sb;
        cont=fopen("cont.txt","wb");
        strcpy(sb->name,"myfs");strcpy(sb->format,"myformat");
        sb->inodefilled=sb->dnodefilled=1;
        sb->inodetotal=2032;sb->dnodetotal=14336;
        b=fs->ibit;b[0]=b[1]=-1;b[2]=1<<7;   //set to filled as first two blocks are filled
        b=fs->dbit;b[0]=1<<7;        //only one datanode (root directory) is filled
        id=&fs->ind[0];
        id->dat[0]=0;
        id->n=1;            //total filled
        id->filld=8;        //size of the dir file
        id->type=1;                 //assuming 1 for directories
        id->links=2;
        id->uid=id->gid=0;id->perm=-1; //dont know what to fill
        dn=&fs->dnd[0];printf("pi:%p\n",&id->type);
        strcpy(dn->d,". 0,.. 0");
        //fwrite((void*)fs,1,fz,cont);//fclose(cont);
        //fflush(cont);
        fseek(cont,0,SEEK_SET);
    }
    else {              //if file already present then read previous contents
        fread((void*)fs,1,fz,cont);fclose(cont);cont=fopen("cont.txt","wb");//fseek(cont,0,SEEK_SET);
    }//return NULL;
}


//main functions section-------------------------------------------------------------


static void destroy_fs(void *pd){//cont=fopen("cont2.txt","wb");
    int fz=4*1024*1024;fwrite((void*)fs,1,fz,cont);fclose(cont);
    //fwrite((void*)"abcdefghijklsb",1,10,logger);
    fwrite((void*)lbuffer,1,loffset-lbuffer,logger);
    fclose(logger);
    free((void*)fs);free((void*)lbuffer);
}

static int getattr_fs(const char *path,struct stat *st,struct fuse_file_info *fi){
    int i,j,k;
    struct inode *e;
    memset(st, 0, sizeof(struct stat));
    i=sprintf(loffset,"getattr called on %s ",path);loffset+=i; //return -ENOENT;
    i=pathtoinode(path);
    if(i==-1)return -ENOENT;
    i=sprintf(loffset,"got it");loffset+=i;
    e=&fs->ind[i];
    st->st_nlink=e->links;
    if(e->type==1)st->st_mode=S_IFDIR | 0755;
    else {st->st_mode=S_IFREG | 0444;st->st_size=getsize(i);}
    return 0;
}

static int readdir_fs(const char *path, void *buf, fuse_fill_dir_t filler,
             off_t offset, struct fuse_file_info *fi,
			 enum fuse_readdir_flags flags){
    int i,j,k;char *c,*a;
    struct inode *e;
    i=sprintf(loffset,"readdir called on %s ",path);loffset+=i;//return -ENOENT;
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
    }
    return 0;
}

static int open_fs(const char *path, struct fuse_file_info *fi){
    int i;
    i=sprintf(loffset,"open called on %s ",path);loffset+=i;return -ENOENT;
    i=pathtoinode(path);
    if(i==-1)return -ENOENT;
    return 0;
}

static int read_fs(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi){
    int i,j,k,s;struct inode *e;char *d;
    i=sprintf(loffset,"read called on %s ",path);loffset+=i;return -ENOENT;
    i=pathtoinode(path);
    if(i==-1)return -ENOENT;
    s=getsize(i);
    if(offset>=s)return 0;
    d=inode_dat(i);
    memcpy(buf,d+offset,size);
    free((void*)d);         // not at all care of optimizations
}