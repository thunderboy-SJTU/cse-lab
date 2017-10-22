#include "inode_manager.h"


/*<<<<<<< HEAD
=======
// disk layer -----------------------------------------

disk::disk()
{
  bzero(blocks, sizeof(blocks));
}

void
disk::read_block(blockid_t id, char *buf)
{*/

  /*
   *your lab1 code goes here.
   *if id is smaller than 0 or larger than BLOCK_NUM 
   *or buf is null, just return.
   *put the content of target block into buf.
   *hint: use memcpy
  */
   /*if(id < 0 || id > BLOCK_NUM || buf == NULL)
      return;
   unsigned char* block = blocks[id];
   memcpy(buf,block,BLOCK_SIZE);            //将block号为id的内容copy到buf中
   return;  
}*/

/*void
disk::write_block(blockid_t id, const char *buf)
{*/
  /*
   *your lab1 code goes here.
   *hint: just like read_block
  */
  /*if(id < 0 || id > BLOCK_NUM || buf == NULL)
      return;
  unsigned char* block = blocks[id];
  memcpy(block,buf,BLOCK_SIZE);         //将buf内容copy到block中
  return;  
}

>>>>>>> lab4*/
// block layer -----------------------------------------

// Allocate a free disk block.


int changeId(int block_id)
{
     if(block_id > 65535)
        return -1;
     int id = block_id;
     char* idAddr = (char*)&id;
     char byte = *(idAddr);
     char second = *(idAddr+1);
     char three = (byte >> 7) & 0x1;
     char five = (byte >> 6) & 0x1;
     char six = (byte >> 5) & 0x1;
     char seven = (byte >> 4) &0x1;
     char one = seven ^ five ^ three;
     char two = seven ^ six ^ three;
     char four = seven ^ six ^ five;
     char eight = one ^ two ^ three ^ four ^ five ^ six ^ seven;
     char byte1 = (one << 7) | (two << 6) | (three << 5) | (four << 4) |(five << 3) | (six << 2) | (seven << 1) | eight;
     three = (byte >> 3) & 0x1;
     five = (byte >> 2) & 0x1;
     six = (byte >> 1) & 0x1;
     seven = byte & 0x1;
     one = seven ^ five ^ three;
     two = seven ^ six ^ three;
     four = seven ^ six ^ five;
     eight = one ^ two ^ three ^ four ^ five ^ six ^ seven;
     char byte2 = (one << 7) | (two << 6) | (three << 5) | (four << 4) |(five << 3) | (six << 2) | (seven << 1) | eight;
     *idAddr = byte1;
     *(idAddr+1) = byte2;
     three = (second >> 7) & 0x1;
     five = (second >> 6) & 0x1;
     six = (second >> 5) & 0x1;
     seven = (second >> 4) & 0x1;
     one = seven ^ five ^ three;
     two = seven ^ six ^ three;
     four = seven ^ six ^ five;
     eight = one ^ two ^ three ^ four ^ five ^ six ^ seven;
     byte1 = (one << 7) | (two << 6) | (three << 5) | (four << 4) |(five << 3) | (six << 2) | (seven << 1) | eight;
     three = (second >> 3) & 0x1;
     five = (second >> 2) & 0x1;
     six = (second >> 1) & 0x1;
     seven = second & 0x1;
     one = seven ^ five ^ three;
     two = seven ^ six ^ three;
     four = seven ^ six ^ five;
     eight = one ^ two ^ three ^ four ^ five ^ six ^ seven;
     byte2 = (one << 7) | (two << 6) | (three << 5) | (four << 4) |(five << 3) | (six << 2) | (seven << 1) | eight;
     *(idAddr+2) = byte1;
     *(idAddr+3) = byte2;
     return id;
}

int RealId(int readId)
{
      char byte;
      bool ifone = true,iftwo = true,iffour = true;
      int id = readId;
      char* idAddr = (char*)&id;
      char a[4];
      a[0] = *(idAddr);
      a[1] = *(idAddr+1);
      a[2] = *(idAddr+2);
      a[3] = *(idAddr+3);
      for(int i = 0;i<2;i++)
      {
         ifone = true;
         iftwo = true;
         iffour = true;
         char one = (a[2*i] >> 7) & 0x1;
         char two = (a[2*i] >> 6) & 0x1;
         char three = (a[2*i] >> 5) & 0x1;
         char four = (a[2*i] >> 4) & 0x1;
         char five = (a[2*i] >> 3) & 0x1;
         char six = (a[2*i] >> 2) & 0x1;
         char seven = (a[2*i] >> 1) & 0x1;
         char eight = a[2*i] & 0x1;
         if(((seven ^ five ^ three)&0x1) != one)
            ifone = false;
         if(((seven ^ six ^ three)&0x1) != two)
            iftwo = false;
         if(((seven ^ six ^ five)&0x1)!= four)
            iffour = false;
         if(((one ^ two ^ three ^ four ^ five ^ six ^ seven) == eight) && !(ifone && iftwo && iffour))
         {
            return -1;
         }
         if((!ifone) && (!iftwo) && (!iffour))
            seven ^= 0x1;
         else if((!ifone) && (!iftwo))
            three ^= 0x1;
         else if((!ifone) && (!iffour))
            five ^= 0x1;
         else if((!iftwo) && (!iffour))
            six ^= 0x1;
         ifone = true;
         iftwo = true;
         iffour = true;
         char one2 = (a[2*i+1] >> 7) & 0x1;
         char two2 = (a[2*i+1] >> 6) & 0x1;
         char three2 = (a[2*i+1] >> 5) & 0x1;
         char four2 = (a[2*i+1] >> 4) & 0x1;
         char five2 = (a[2*i+1] >> 3) & 0x1;
         char six2 = (a[2*i+1] >> 2) & 0x1;
         char seven2 = (a[2*i+1] >> 1) & 0x1;
         char eight2 = a[2*i+1] & 0x1;
         if(((seven2 ^ five2 ^ three2)&0x1)!= one2)
            ifone = false;
         if(((seven2 ^ six2 ^ three2)&0x1) != two2)
            iftwo = false;
         if(((seven2 ^ six2 ^ five2)&0x1) != four2)
            iffour = false;
         if(((one2 ^ two2 ^ three2 ^ four2 ^ five2 ^ six2 ^ seven2) == eight2) && !(ifone && iftwo && iffour))
         {
            return -1;
         }
         if((!ifone) && (!iftwo) && (!iffour))
            seven2 ^= 0x1;
         else if((!ifone) && (!iftwo))
            three2 ^= 0x1;
         else if((!ifone) && (!iffour))
            five2 ^= 0x1;
         else if((!iftwo) && (!iffour))
            six2 ^= 0x1;
         byte = (three << 7) | (five << 6) | (six << 5) | (seven << 4) | (three2 << 3) | (five2 << 2) | (six2 << 1) | seven2;
         *(idAddr+i) = byte;
      }
      *(idAddr+2) = 0;
      *(idAddr+3) = 0;
      return id;
}


/*void checkBlock(blockid_t block_id, block_manager* bm)         //这些都不知道错在哪儿了，按照RS（255,253）写的，难过 T T
{
  int pp[9] = {1, 0, 1, 1, 1, 0, 0, 0, 1}; 
  int alpha[256];
  int index[256];
  int gp[3];
  char m[255];
  char s0 = 0,s1 = 0 ,s2 = 0,s3 = 0;
  int mask = 1;
  index[0] = -1;
  alpha[8] = 0;
  for(int i = 0;i < 8;i++)
  {
     alpha[i] = mask;
     index[alpha[i]] = i;
     if(pp[i] != 0)
       alpha[8] ^= mask;
     mask = mask << 1;     
  }
  index[alpha[8]] = 8;
  mask = mask >> 1;
  for(int i = 9; i < 255;i++)
  {
    if(alpha[i-1] >= mask)
       alpha[i] = alpha[8]^((alpha[i-1]^mask)<<1);
    else
       alpha[i] = alpha[i-1] << 1;
    index[alpha[i]] = i;
  }
  gp[0] = 1;
  gp[1] = index[alpha[1]^1];
  gp[2] = 0;
  
  char* buf = (char*)malloc(sizeof(char) * BLOCK_SIZE);
  bm->read_block(block_id,buf);
  m[1] = buf[BLOCK_SIZE-6];
  m[0] = buf[BLOCK_SIZE-5];

  for(int i = 0; i <253;i++)
     m[254-i] = buf[i];
  for(int i = 0; i <255;i++)
  {
     s0 ^= m[i];
     s1 ^= alpha[(index[m[i]]+i)%255];
  }
  if(s0 != 0 || s1 != 0)
  {    
  int tmp = index[s1] - index[s0];
  if(tmp < 0)
     tmp += 255;
  if(tmp == 0 || tmp == 1)
     buf[BLOCK_SIZE-5-tmp] = m[tmp] ^ s0;
  else
     buf[254-tmp] = m[tmp]^s0;
  }
  
  m[1] = buf[BLOCK_SIZE-4];
  m[0] = buf[BLOCK_SIZE-3];
  for(int i = 0; i <253;i++)
     m[254-i] = buf[253+i];
  for(int i = 0; i <255;i++)
  {
     s2 ^= m[i];
     s3 ^= alpha[(index[m[i]]+i)%255];
  }
  if(s2 != 0 || s3 != 0)
  {    
  int tmp = index[s3] - index[s2];
  if(tmp < 0)
     tmp += 255;
  if(tmp == 0 || tmp == 1)
     buf[BLOCK_SIZE-3-tmp] = m[tmp] ^ s2;
  else
     buf[507-tmp] = m[tmp]^s2;
  }
  if(s0 == 0 && s1 == 0 && s2 == 0 && s3 == 0)
  {
     free(buf);
     return;
  }
  bm->write_block(block_id,buf); 
  free(buf);
}*/

/*void writeRS(blockid_t block_id, block_manager* bm)
{
  int pp[9] = {1, 0, 1, 1, 1, 0, 0, 0, 1}; 
  int alpha[256];
  int index[256];
  int gp[3];
  char m[255];
  char code[2] = {0,0};
  int s0 = 0,s1 = 0;
  int mask = 1;
  index[0] = -1;
  alpha[8] = 0;
  for(int i = 0;i < 8;i++)
  {
     alpha[i] = mask;
     index[alpha[i]] = i;
     if(pp[i] != 0)
       alpha[8] ^= mask;
     mask = mask << 1;     
  }
  index[alpha[8]] = 8;
  mask = mask >> 1;
  for(int i = 9; i < 255;i++)
  {
    if(alpha[i-1] >= mask)
       alpha[i] = alpha[8]^((alpha[i-1]^mask)<<1);
    else
       alpha[i] = alpha[i-1] << 1;
    index[alpha[i]] = i;
  }
  gp[0] = 1;
  gp[1] = index[alpha[1]^1];
  gp[2] = 0;
  
  char* buf = (char*)malloc(sizeof(char) * BLOCK_SIZE);
  bm->read_block(block_id,buf);
  for(int i = 0; i <253;i++)
     m[254-i] = buf[i];
  for(int i = 254; i>=2;i--) 
  {
     int feedback = index[m[i]^code[1]];
     if(feedback == -1)
     {
            if (gp[1] != -1)  
                code[1] = code[0]^alpha[(gp[1]+feedback)%255];    
            else  
                code[1] = code[0] ;  
            code[0] = alpha[(gp[0]+feedback)%255];
     }  
     else  
     {  
                code[1] = code[0]; 
                code[0] = 0; 
     } 
  } 
  buf[BLOCK_SIZE-6] = code[1];
  buf[BLOCK_SIZE-5] = code[0];
  
  code[0] = 0;
  code[1] = 0;
  for(int i = 0; i <253;i++)
     m[254-i] = buf[253+i];
  for(int i = 254; i>=2;i--) 
  {
     int feedback = index[m[i]^code[1]];
     if(feedback == -1)
     {
            if (gp[1] != -1)  
                code[1] = code[0]^alpha[(gp[1]+feedback)%255];    
            else  
                code[1] = code[0] ;  
            code[0] = alpha[(gp[0]+feedback)%255];
     }  
     else  
     {  
                code[1] = code[0]; 
                code[0] = 0; 
     } 
  } 
  buf[BLOCK_SIZE-4] = code[1];
  buf[BLOCK_SIZE-3] = code[0];
  buf[BLOCK_SIZE-2] = 0;
  buf[BLOCK_SIZE-1] = 0;
  bm->write_block(block_id,buf);  
  free(buf);
}*/
 


/* 两个RS(255,253)，实现一个block里前后255各纠1个byte（浪费两个byte）*/    
unsigned char getFirstCode(char* buf)
{
   unsigned char poly = 0x1d; //{1, 0, 1, 1, 1, 0, 0, 0, 1}本原表达式
   unsigned char code = 0;
   for(int i=0;i<253;i++)
   {
     char byte =buf[i];
     for(int j=0;j<8;j++)
     {
       if(code & 0x80) 
          code = (code <<1)^poly;
       else 
          code <<= 1;
       if(byte & 0x80) 
          code ^=poly;
       byte <<= 1;
     }
   }
   return code;
}

unsigned char getSecondCode(char* buf)
{
  unsigned char code = 0;
  for(int i=0;i<253;i++) 
    code ^= buf[i];
  return code;
}

void writeRSBuf(char* buf)
{
   buf[BLOCK_SIZE-6]=getFirstCode(buf);
   buf[BLOCK_SIZE-5]=getSecondCode(buf);
   buf[BLOCK_SIZE-4] = getFirstCode(buf + 253);
   buf[BLOCK_SIZE-3] = getSecondCode(buf + 253);
}
void writeRS(blockid_t block_id, block_manager* bm)
{
   
   char* buf = (char*)malloc(sizeof(char) * BLOCK_SIZE);
   bm->read_block(block_id,buf);
   writeRSBuf(buf);
   bm->write_block(block_id,buf);  
   free(buf);
}

int checkBuf(char* buf)
{
   unsigned char code1,code2;
   unsigned char poly = 0x1d;
   int flag = 0;
   unsigned char check1 = buf[BLOCK_SIZE-6]^(code1 = getFirstCode(buf));
   unsigned char check2 = buf[BLOCK_SIZE-5]^(code2 = getSecondCode(buf));
   if((!check1)&&(!check2))   
        flag = 1;
   else if((check1)&&(!check2))              
   {
     buf[BLOCK_SIZE-6] = code1;
   }
   else if((!check1)&&(check2))              
   {
     buf[BLOCK_SIZE-5] = code2;
   }
   else 
   {
     int i = 0;
     while(check1!=check2)
     {
       if(check1&0x80) 
          check1=(check1<<1)^poly;
       else 
          check1<<=1;
       i++;
     }
     i=(i>>3) + ((i&7)<<5) - 2;
     buf[i]^=check2;

   }   
   check1=buf[BLOCK_SIZE-4]^( code1 = getFirstCode(buf + 253));
   check2=buf[BLOCK_SIZE-3]^( code2 = getSecondCode(buf + 253));
   if((!check1)&&(!check2))
   {  
        if(flag == 1)
          flag = 2;
   }
   else if((check1)&&(!check2))              
   {
     buf[BLOCK_SIZE-4] = code1;
   }
   else if((!check1)&&(check2))              
   {
     buf[BLOCK_SIZE-3] = code2;
   }
   else 
   {
     int i = 0;
     while(check1!=check2)
     {
       if(check1&0x80) 
         check1=(check1<<1)^poly;
       else 
         check1<<=1;
       i++;
     }
     i=(i>>3) + ((i&7)<<5) - 2;
     buf[i + 253] ^= check2;
   }
   return flag;
}
  
void checkBlock(blockid_t block_id, block_manager* bm)
{
   
   char* buf = (char*)malloc(sizeof(char) * BLOCK_SIZE);
   bm->read_block(block_id,buf);
   int flag = checkBuf(buf);
   if(flag != 2)
     bm->write_block(block_id,buf);  
   free(buf); 
}


   
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
  /*blockid_t begin_id = IBLOCK(INODE_NUM, sb.nblocks) + 1;
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
  return -1;*/
  blockid_t begin_id = IBLOCK(INODE_NUM, sb.nblocks) + 1;
  for(blockid_t id = begin_id; id < BLOCK_NUM; id++)
  {
     if(using_blocks.find(id) == using_blocks.end() || using_blocks[id] == 0)
     {
         using_blocks[id] = 1;  
         return id;
     }
  }
  return -1;
}

void
block_manager::free_block(uint32_t id)
{
  /* 
   * your lab1 code goes here.
   * note: you should unmark the corresponding bit in the block bitmap when free.
   */
  /*if(id < 0 || id > BLOCK_NUM)    
     return;
  blockid_t b_id = BBLOCK(id);
  int byte_cnt = (id % BPB)/8;
  int offset = (id % BPB) % 8;
  char* buf = (char*)malloc(sizeof(char) * BLOCK_SIZE);
  read_block(b_id,buf);
  char byte = *((char*)buf + byte_cnt);
  *((char*)buf + byte_cnt) = byte &(~(0x1 << (7-offset)));       
  write_block(b_id,buf);      //将bitmap中对应的bit修改为0
  free(buf);*/
  std::map<uint32_t, int>::iterator iter = using_blocks.find(id);
  if(iter != using_blocks.end())
  {
     if(using_blocks[id] == 1){
        using_blocks[id] = 0;
     }
  }
  return;
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
   * do not forget to free memory if nececheck2ary.
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
  checkBlock(IBLOCK(inum, bm->sb.nblocks),bm);  //changed
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
  writeRS(IBLOCK(inum, bm->sb.nblocks), bm);   //changed
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



   int offset = 6;


   int read_sz = ino->size*2;  //changed
   //int read_sz = ino->size;

   char* buf;
   char* tmpbuf;
   *size = ino->size;
   if(*size == 0)    //如果文件的size是0，返回""
   {
      buf = NULL;
      *buf_out = buf;
      int tm = time(0);
      ino->atime = tm;
      return;   
   }

   int block_cnt = (read_sz - 1)/(BLOCK_SIZE - offset);
   //int block_cnt = (read_sz - 1)/BLOCK_SIZE;    //根据size获得文件占用到第几个块

   read_sz = (block_cnt + 1) * BLOCK_SIZE;
   buf = (char*)malloc(sizeof(char) * read_sz/2);    //changed  
   tmpbuf = (char*)malloc(sizeof(char) * read_sz);   //changed 
   //tmpbuf = (char*)malloc(sizeof(char) * (block_cnt + 1) * BLOCK_SIZE);  
  
   if(block_cnt < NDIRECT)   //如果小于NDIRECT
   {  
      blockid_t block_id; 
      for(int i = 0 ; i <= block_cnt ; i++)   //遍历使用的块，copy每个块的内容，加入buf中
      {
         block_id = ino->blocks[i]; 
         bm->read_block(block_id,tmpbuf+i*BLOCK_SIZE);  //changed
      }    
   }
   else    //如果大于等于NDIRECT，说明用到了间接块
   {
      blockid_t block_id; 
      for(int i = 0 ; i < NDIRECT ; i++)    //小于NDIRECT的块依然按照之前的处理
      {
         block_id = ino->blocks[i]; 
         bm->read_block(block_id,tmpbuf+i*BLOCK_SIZE); //changed
      }
      char* buf_idir = (char*)malloc(sizeof(char) * BLOCK_SIZE + 1);   //获取间接块，找到对应的块，读取其中的block number，其余与之前相同
      bm->read_block(ino->blocks[NDIRECT],buf_idir);
      for(int i = NDIRECT ; i <= block_cnt ; i++)
      {
         block_id = *((uint32_t*) buf_idir + (i - NDIRECT));
         block_id = RealId(block_id);    //changed
         if(block_id == -1)
         {
            checkBuf(buf_idir);
            block_id = *((uint32_t*) buf_idir + (i - NDIRECT));
            block_id = RealId(block_id);
         }
         bm->read_block(block_id,tmpbuf+i*BLOCK_SIZE);
         block_id = changeId(block_id);
         *((uint32_t*) buf_idir + (i - NDIRECT)) = block_id;       
      }
      bm->write_block(ino->blocks[NDIRECT],buf_idir);
      free(buf_idir);    
   }

   int flag = 0;
   for(int j = 0; j < block_cnt+ 1; j++)
   {   
     for(int i = 0; i < (BLOCK_SIZE - 6)/2;i++)
     {
      if(j*(BLOCK_SIZE-6)/2+i >= ino->size)
      { 
          flag = 2;
          break;
      }
      char byte;
      bool ifone = true,iftwo = true,iffour = true;
      char byte1 = tmpbuf[j*BLOCK_SIZE + 2*i];
      char one = (byte1 >> 7) & 0x1;
      char two = (byte1 >> 6) & 0x1;
      char three = (byte1 >> 5) & 0x1;
      char four = (byte1 >> 4) & 0x1;
      char five = (byte1 >> 3) & 0x1;
      char six = (byte1 >> 2) & 0x1;
      char seven = (byte1 >> 1) & 0x1;
      char eight = byte1 & 0x1;
      
      if(((seven ^ five ^ three)&0x1) != one)
         ifone = false;
      if(((seven ^ six ^ three)&0x1) != two)
         iftwo = false;
      if(((seven ^ six ^ five)&0x1)!= four)
         iffour = false;
      if(((one ^ two ^ three ^ four ^ five^ six ^ seven) == eight) && !(ifone && iftwo && iffour))
      {
         flag = 1;
         break;
      }
      if((!ifone) && (!iftwo) && (!iffour))
         seven ^= 0x1;
      else if((!ifone) && (!iftwo))
         three ^= 0x1;
      else if((!ifone) && (!iffour))
         five ^= 0x1;
      else if((!iftwo) && (!iffour))
         six ^= 0x1;
      ifone = true;
      iftwo = true;
      iffour = true;
      char byte2 = tmpbuf[j*BLOCK_SIZE +2*i+1];
      char one2 = (byte2 >> 7) & 0x1;
      char two2 = (byte2 >> 6) & 0x1;
      char three2 = (byte2 >> 5) & 0x1;
      char four2 = (byte2 >> 4) & 0x1;
      char five2 = (byte2 >> 3) & 0x1;
      char six2 = (byte2 >> 2) & 0x1;
      char seven2 = (byte2 >> 1) & 0x1;
      char eight2 = byte2 & 0x1;
      if(((seven2 ^ five2 ^ three2)&0x1)!= one2)
         ifone = false;
      if(((seven2 ^ six2 ^ three2)&0x1) != two2)
         iftwo = false;
      if(((seven2 ^ six2 ^ five2)&0x1) != four2)
         iffour = false;
      if(((one2 ^ two2 ^ three2 ^ four2 ^ five2 ^ six2 ^ seven2) == eight2) && !(ifone && iftwo && iffour))
      {
         flag = 1;
         break;
      }
      if((!ifone) && (!iftwo) && (!iffour))
         seven2 ^= 0x1;
      else if((!ifone) && (!iftwo))
         three2 ^= 0x1;
      else if((!ifone) && (!iffour))
         five2 ^= 0x1;
      else if((!iftwo) && (!iffour))
         six2 ^= 0x1;      
      byte = (three << 7) | (five << 6) | (six << 5) | (seven << 4) | (three2 << 3) | (five2 << 2) | (six2 << 1) | seven2;
      buf[j*(BLOCK_SIZE-6)/2+i] = byte;
     } 
     if(flag == 1)
     {
        checkBuf(tmpbuf+j*BLOCK_SIZE);
        j--;
        flag = 0;
     }
    else if(flag == 2)
     {
       flag = 0;
       break; 
     } 
 }    
      write_file(inum,buf,ino->size);
      *buf_out = buf; 
      int tm = time(0);
      ino->atime = tm;
      free(tmpbuf);
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
   * you should free some blocks if nececheck2ary.
   */
   if(size < 0 || size*2 > (int)MAXFILE*BLOCK_SIZE || buf == NULL)    //如果size或者buf不符合要求，返回  changed
     return;
   struct inode* ino = get_inode(inum);   //获得inode
   if(ino == NULL)
      return;    
   int origin_size = ino->size;
   int old_block_num, new_block_num;     //根据新旧的size获得新旧各用到第几个块，-1表示size为0未用块
   /*old_block_num = (origin_size*2 - 1)/BLOCK_SIZE; //changed
   new_block_num = (size*2 - 1)/BLOCK_SIZE;     //changed*/

   int offset = 6;
   old_block_num = (origin_size*2 - 1)/(BLOCK_SIZE -offset); //changed
   new_block_num = (size*2 - 1)/(BLOCK_SIZE-offset); 

   if(new_block_num > 158)
       return;

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
              char* buf_idir = (char*)malloc(sizeof(char) * BLOCK_SIZE + 1);  //changed
              bm->read_block(ino->blocks[NDIRECT],buf_idir);
              int id = changeId(block_id);
              *((uint32_t*) buf_idir + (i - NDIRECT)) = id;
              writeRSBuf(buf_idir);
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
             blockid_t free_id = RealId(*((uint32_t*) buf_idir + (i - NDIRECT)));
             if(free_id == -1)
             {
               checkBuf(buf_idir);
               free_id = RealId(*((uint32_t*) buf_idir + (i - NDIRECT)));
             }
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
  // char* tmpbuf = (char*)malloc(sizeof(char) * BLOCK_SIZE/2 + 1);  //changed
   char* tmpbuf = (char*)malloc(sizeof(char) * (BLOCK_SIZE-offset)/2 + 1); 
   for(int i = 0;i< new_block_num+1;i++)       //将buf分成一个个block的大小，分别写入各个块中，DIRECT和INDIRECT的情况与read_file相同
   {      
      //memcpy(tmpbuf,buf + i* BLOCK_SIZE/2, BLOCK_SIZE/2);
      memcpy(tmpbuf,buf + i* (BLOCK_SIZE - offset)/2, (BLOCK_SIZE - offset)/2);     
      for(int j = 0;j < (BLOCK_SIZE - offset)/2;j++)
     //for(int j = 0;j < BLOCK_SIZE/2;j++)
      {
          char byte = *(tmpbuf+j);
          char three = (byte >> 7) & 0x1;
          char five = (byte >> 6) & 0x1;
          char six = (byte >> 5) & 0x1;
          char seven = (byte >> 4) &0x1;
          char one = seven ^ five ^ three;
          char two = seven ^ six ^ three;
          char four = seven ^ six ^ five;
          char eight = one ^ two ^ three ^ four ^ five ^ six ^ seven;
          char byte1 = (one << 7) | (two << 6) | (three << 5) | (four << 4) |(five << 3) | (six << 2) | (seven << 1) | eight;
          three = (byte >> 3) & 0x1;
          five = (byte >> 2) & 0x1;
          six = (byte >> 1) & 0x1;
          seven = byte & 0x1;
          one = seven ^ five ^ three;
          two = seven ^ six ^ three;
          four = seven ^ six ^ five;
          eight = one ^ two ^ three ^ four ^ five ^ six ^ seven;
          char byte2 = (one << 7) | (two << 6) | (three << 5) | (four << 4) |(five << 3) | (six << 2) | (seven << 1) | eight;
          block_buf[2*j] = byte1;
          block_buf[2*j+1] = byte2;
      }

      writeRSBuf(block_buf);
      if(i < NDIRECT)
      {
          bm->write_block(ino->blocks[i],block_buf);
      }
      else
      {
         char* buf_idir = (char*)malloc(sizeof(char) * BLOCK_SIZE + 1);
         bm->read_block(ino->blocks[NDIRECT],buf_idir);
         blockid_t block_id = RealId(*((uint32_t*) buf_idir + (i - NDIRECT))) ;
         if(block_id == -1)
         {
             checkBuf(buf_idir);
             block_id = RealId(*((uint32_t*) buf_idir + (i - NDIRECT)));
         }
         bm->write_block(block_id,block_buf);
         block_id = changeId(block_id);
         *((uint32_t*) buf_idir + (i - NDIRECT)) = block_id;
         bm->write_block(ino->blocks[NDIRECT],buf_idir);
         free(buf_idir);
      }
      
   }
   free(tmpbuf);
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
             blockid_t free_id = RealId(*((uint32_t*) buf_idir + (i - NDIRECT)));
             if(free_id == -1)
             {
               checkBuf(buf_idir);
               free_id = RealId(*((uint32_t*) buf_idir + (i - NDIRECT)));
             }
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
