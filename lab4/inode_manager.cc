#include "inode_manager.h"

// disk layer -----------------------------------------

disk::disk()
{
  bzero(blocks, sizeof(blocks));
}

void
disk::read_block(blockid_t id, char *buf)
{

  /*
   *your lab1 code goes here.
   *if id is smaller than 0 or larger than BLOCK_NUM 
   *or buf is null, just return.
   *put the content of target block into buf.
   *hint: use memcpy
  */
   if(id < 0 || id > BLOCK_NUM || buf == NULL)
      return;
   unsigned char* block = blocks[id];
   memcpy(buf,block,BLOCK_SIZE);            //将block号为id的内容copy到buf中
   return;  
}

void
disk::write_block(blockid_t id, const char *buf)
{
  /*
   *your lab1 code goes here.
   *hint: just like read_block
  */
  if(id < 0 || id > BLOCK_NUM || buf == NULL)
      return;
  unsigned char* block = blocks[id];
  memcpy(block,buf,BLOCK_SIZE);         //将buf内容copy到block中
  return;  
}

// block layer -----------------------------------------

// Allocate a free disk block.
blockid_t
block_manager::alloc_block()
{
  /*
   * your lab1 code goes here.
   * note: you should mark the corresponding bit in block bitmap when alloc.
   * you need to think about which block you can start to be allocated.

   *hint: use macro IBLOCK and BBLOCK.
          use bit operation.
          remind yourself of the layout of disk.
   */
  blockid_t begin_id = IBLOCK(INODE_NUM, sb.nblocks) + 1;
  blockid_t id,b_id;
  char* buf = (char*)malloc(sizeof(char) * BLOCK_SIZE);
  for(id = begin_id;id < BLOCK_NUM; id++)     //从data部分往后寻找空的block，从头往后遍历
  {
     b_id = BBLOCK(id);
     int byte_cnt = (id % BPB)/8;
     int offset = (id % BPB) % 8;   
     read_block(b_id,buf);
     char byte = *((char*)buf + byte_cnt);
     //int boolean = (byte >> (7 - offset)) & 0x1;       //从bitmap中获取bit，如果是0，则block未被使用，可以分配，修改bit为1，返回id
     int boolean = (byte) &( 0x1 << (7-offset));
     if(boolean == 0)        
     {
        *((char*)buf + byte_cnt) = byte | (0x1 << (7-offset));
        write_block(b_id,buf); 
        free(buf);
        return id;
     }
  }
  free(buf);
  return -1;
}

void
block_manager::free_block(uint32_t id)
{
  /* 
   * your lab1 code goes here.
   * note: you should unmark the corresponding bit in the block bitmap when free.
   */
  if(id < 0 || id > BLOCK_NUM)    
     return;
  blockid_t b_id = BBLOCK(id);
  int byte_cnt = (id % BPB)/8;
  int offset = (id % BPB) % 8;
  char* buf = (char*)malloc(sizeof(char) * BLOCK_SIZE);
  read_block(b_id,buf);
  char byte = *((char*)buf + byte_cnt);
  *((char*)buf + byte_cnt) = byte &(~(0x1 << (7-offset)));       
  write_block(b_id,buf);      //将bitmap中对应的bit修改为0
  free(buf);
}  

// The layout of disk should be like this:
// |<-sb->|<-free block bitmap->|<-inode table->|<-data->|
block_manager::block_manager()
{
  d = new disk();

  // format the disk
  sb.size = BLOCK_SIZE * BLOCK_NUM;
  sb.nblocks = BLOCK_NUM;
  sb.ninodes = INODE_NUM;

}

void
block_manager::read_block(uint32_t id, char *buf)
{
  d->read_block(id, buf);
}

void
block_manager::write_block(uint32_t id, const char *buf)
{
  d->write_block(id, buf);
}

// inode layer -----------------------------------------

inode_manager::inode_manager()
{
  bm = new block_manager();
  uint32_t root_dir = alloc_inode(extent_protocol::T_DIR);
  if (root_dir != 1) {
    printf("\tim: error! alloc first inode %d, should be 1\n", root_dir);
    exit(0);
  }
}

/* Create a new file.
 * Return its inum. */
uint32_t
inode_manager::alloc_inode(uint32_t type)
{
  /* 
   * your lab1 code goes here.
   * note: the normal inode block should begin from the 2nd inode block.
   * the 1st is used for root_dir, see inode_manager::inode_manager().   
   * if you get some heap memory, do not forget to free it.
   */
   uint32_t inum;
   struct inode *ino;
   for(inum = 1; inum <= INODE_NUM;inum++)
   {
       ino = get_inode(inum);     //从头往后遍历inode表寻找未被分配的inode
       if(ino == NULL)
       {
           ino = (struct inode*)malloc(sizeof(struct inode));   //初始化inode，同时put_inode
           int tm = time(0);
           ino->type = type;
           ino->size = 0;   
           ino->atime = tm;
           ino->ctime = tm;
           ino->mtime = tm;
           for(int i = 0;i <= NDIRECT;i++)
               ino->blocks[i] = 0;
           put_inode(inum,ino);
           free(ino);
           return inum;     //返回inode number
       } 
       free(ino);   
   }
   return 0;  
}

void
inode_manager::free_inode(uint32_t inum)
{
  /* 
   * your lab1 code goes here.
   * note: you need to check if the inode is already a freed one;
   * if not, clear it, and remember to write back to disk.
   * do not forget to free memory if necessary.
   */
   inode* ino = get_inode(inum);     //将inode的type和size都修改为0
   if(ino == NULL)
      return;
   /*ino->type = 0;
   ino->size = 0;*/
   bzero(ino, sizeof(inode));
   put_inode(inum,ino);
   free(ino);
}
/* Return an inode structure by inum, NULL otherwise.
 * Caller should release the memory. */
struct inode* 
inode_manager::get_inode(uint32_t inum)
{
  struct inode *ino, *ino_disk;
  char buf[BLOCK_SIZE];

  printf("\tim: get_inode %d\n", inum);

  if (inum < 0 || inum >= INODE_NUM) {
    printf("\tim: inum out of range\n");
    return NULL;
  }

  bm->read_block(IBLOCK(inum, bm->sb.nblocks), buf);
  // printf("%s:%d\n", __FILE__, __LINE__);

  ino_disk = (struct inode*)buf + inum%IPB;
  if (ino_disk->type == 0) {
    printf("\tim: inode not exist\n");
    return NULL;
  }

  ino = (struct inode*)malloc(sizeof(struct inode));
  *ino = *ino_disk;

  return ino;
}

void
inode_manager::put_inode(uint32_t inum, struct inode *ino)
{
  char buf[BLOCK_SIZE];
  struct inode *ino_disk;

  printf("\tim: put_inode %d\n", inum);
  if (ino == NULL)
    return;

  bm->read_block(IBLOCK(inum, bm->sb.nblocks), buf);
  ino_disk = (struct inode*)buf + inum%IPB;
  *ino_disk = *ino;
  bm->write_block(IBLOCK(inum, bm->sb.nblocks), buf);
}

#define MIN(a,b) ((a)<(b) ? (a) : (b))

/* Get all the data of a file by inum. 
 * Return alloced data, should be freed by caller. */
void
inode_manager::read_file(uint32_t inum, char **buf_out, int *size)
{
  /*
   * your lab1 code goes here.
   * note: read blocks related to inode number inum,
   * and copy them to buf_out
   */
   if(buf_out == NULL)             //如果bufout和size是null，为其分配空间
      buf_out = (char**)malloc(sizeof(char*));
   if(size == NULL)
      size = (int*)malloc(sizeof(int));
   struct inode* ino = get_inode(inum);    //获得对应的inode
   if(ino == NULL)
       return;
   int read_sz = ino->size;
   char* buf;
   *size = read_sz;
   if(*size == 0)    //如果文件的size是0，返回""
   {
     /* buf = (char*)malloc(sizeof(char));
      *buf ='\0';*/
      buf = NULL;
      *buf_out = buf;
      int tm = time(0);
      ino->atime = tm;
      return;   
   }
   buf = (char*)malloc(sizeof(char) * read_sz);     
  
   int block_cnt = (read_sz - 1)/BLOCK_SIZE;    //根据size获得文件占用到第几个块
   if(block_cnt < NDIRECT)   //如果小于NDIRECT
   {  
      blockid_t block_id; 
      for(int i = 0 ; i < block_cnt ; i++)   //遍历使用的块，copy每个块的内容，加入buf中
      {
         block_id = ino->blocks[i]; 
         bm->read_block(block_id,buf+i*BLOCK_SIZE);
      }
      int remain_sz = read_sz - block_cnt * BLOCK_SIZE;   //最后一个块单独读取，不读取多余的内容
      char* buf2 = (char*)malloc(sizeof(char) * BLOCK_SIZE + 1);
      bm->read_block(ino->blocks[block_cnt],buf2);
      memcpy(buf+block_cnt*BLOCK_SIZE,buf2,remain_sz); 
      free(buf2);     
   }
   else    //如果大于等于NDIRECT，说明用到了间接块
   {
      blockid_t block_id; 
      for(int i = 0 ; i < NDIRECT ; i++)    //小于NDIRECT的块依然按照之前的处理
      {
         block_id = ino->blocks[i]; 
         bm->read_block(block_id,buf+i*BLOCK_SIZE);
      }
      char* buf_idir = (char*)malloc(sizeof(char) * BLOCK_SIZE + 1);   //获取间接块，找到对应的块，读取其中的block number，其余与之前相同
      bm->read_block(ino->blocks[NDIRECT],buf_idir);
      for(int i = NDIRECT ; i < block_cnt ; i++)
      {
         block_id = *((uint32_t*) buf_idir + (i - NDIRECT));
         bm->read_block(block_id,buf+i*BLOCK_SIZE);
      }
      int remain_sz = read_sz - block_cnt * BLOCK_SIZE;
      char* buf2 = (char*)malloc(sizeof(char) * BLOCK_SIZE + 1);
      bm->read_block(*((uint32_t*) buf_idir + (block_cnt - NDIRECT)),buf2);
      memcpy(buf+block_cnt*BLOCK_SIZE,buf2,remain_sz); 
      free(buf_idir);
      free(buf2);      
   }   
      *buf_out = buf; 
      int tm = time(0);
      ino->atime = tm;
      free(ino);
}

/* alloc/free blocks if needed */
void
inode_manager::write_file(uint32_t inum, const char *buf, int size)
{
  /*
   * your lab1 code goes here.
   * note: write buf to blocks of inode inum.
   * you need to consider the situation when the size of buf 
   * is larger or smaller than the size of original inode.
   * you should free some blocks if necessary.
   */
   if(size < 0 || size > (int)MAXFILE*BLOCK_SIZE || buf == NULL)    //如果size或者buf不符合要求，返回
     return;
   struct inode* ino = get_inode(inum);   //获得inode
   if(ino == NULL)
      return;
   int origin_size = ino->size;
   int old_block_num, new_block_num;     //根据新旧的size获得新旧各用到第几个块，-1表示size为0未用块
   old_block_num = (origin_size - 1)/BLOCK_SIZE;
   new_block_num = (size - 1)/BLOCK_SIZE;
   if(origin_size == 0)
       old_block_num = -1;
   if(size == 0)
       new_block_num = -1;
   if(new_block_num > old_block_num)    //将多的块free，少的块alloc，一次分配完全
   {
      blockid_t block_id;
      for(int i = old_block_num+1;i<new_block_num + 1;i++)
      {
           block_id = bm->alloc_block();
           if(i == NDIRECT)
           {
               blockid_t new_id = bm->alloc_block();
               ino->blocks[NDIRECT] = new_id;
           }
           if(i >= NDIRECT)
           {
              char* buf_idir = (char*)malloc(sizeof(char) * BLOCK_SIZE + 1);
              bm->read_block(ino->blocks[NDIRECT],buf_idir);
              *((uint32_t*) buf_idir + (i - NDIRECT)) = block_id;
              bm->write_block(ino->blocks[NDIRECT],buf_idir);
              free(buf_idir);
           }
           else
           {
              ino->blocks[i] = block_id;
           }
      }
   }
   else if(new_block_num < old_block_num)
   {
      for(int i = new_block_num;i>old_block_num;i--)
      {
         if(i >= NDIRECT)
         {
             char* buf_idir = (char*)malloc(sizeof(char) * BLOCK_SIZE + 1);
             bm->read_block(ino->blocks[NDIRECT],buf_idir);
             blockid_t free_id = *((uint32_t*) buf_idir + (i - NDIRECT));
             *((uint32_t*) buf_idir + (i - NDIRECT)) = 0;
             bm->write_block(ino->blocks[NDIRECT],buf_idir);
             bm->free_block(free_id);
             free(buf_idir);
         }
         if( i <= NDIRECT)
         {
            blockid_t free_id = ino->blocks[i];
            ino->blocks[i] = 0;
            bm->free_block(free_id);
         }
      }
   }
   char* block_buf = (char*)malloc(sizeof(char) * BLOCK_SIZE + 1);
   for(int i = 0;i< new_block_num+1;i++)       //将buf分成一个个block的大小，分别写入各个块中，DIRECT和INDIRECT的情况与read_file相同
   {
      
      memcpy(block_buf,buf + i* BLOCK_SIZE, BLOCK_SIZE);
      if(i < NDIRECT)
      {
          bm->write_block(ino->blocks[i],block_buf);
      }
      else
      {
         char* buf_idir = (char*)malloc(sizeof(char) * BLOCK_SIZE + 1);
         bm->read_block(ino->blocks[NDIRECT],buf_idir);
         blockid_t block_id = *((uint32_t*) buf_idir + (i - NDIRECT)) ;
         bm->write_block(block_id,block_buf);
         free(buf_idir);
      }
   }
   free(block_buf);
   ino->size = size; 
   int tm = time(0);
   ino->atime = tm;
   ino->mtime = tm;
   ino->ctime = tm;
   put_inode(inum,ino);
   free(ino);
}

void
inode_manager::getattr(uint32_t inum, extent_protocol::attr &a)
{
  /*
   * your lab1 code goes here.
   * note: get the attributes of inode inum.
   * you can refer to "struct attr" in extent_protocol.h
   */

   struct inode* ino = get_inode(inum);    //获得inode
   if(ino == NULL)
      return;
   a.type = ino->type;    //获得相关的attribute
   a.atime = ino->atime;
   a.mtime = ino->mtime;
   a.ctime = ino->ctime;
   a.size = ino->size;  
}

void
inode_manager::remove_file(uint32_t inum)
{
  /*
   * your lab1 code goes here
   * note: you need to consider about both the data block and inode of the file
   * do not forget to free memory if necessary.
   */
  inode* ino = get_inode(inum);    //获得相应的inode
  if(ino == NULL)
     return;
  int size = ino->size;
  blockid_t block_num = (size - 1)/BLOCK_SIZE;
  for(int i = block_num;i>= 0;i--)    //将所有的block都free
  {
         if(i >= NDIRECT)
         {
             char* buf_idir = (char*)malloc(sizeof(char) * BLOCK_SIZE + 1);
             bm->read_block(ino->blocks[NDIRECT],buf_idir);
             blockid_t free_id = *((uint32_t*) buf_idir + (i - NDIRECT));
             *((uint32_t*) buf_idir + (i - NDIRECT)) = 0;
             bm->write_block(ino->blocks[NDIRECT],buf_idir);
             bm->free_block(free_id);
             free(buf_idir);
         }
         if( i <= NDIRECT)
         {
            blockid_t free_id = ino->blocks[i];
            ino->blocks[i] = 0;
            bm->free_block(free_id);
         }
   }
   put_inode(inum,ino);   //写回inode表
   free_inode(inum);   //将inode表中的inode type改为0，size改为0，写回inode表
   free(ino);
}
