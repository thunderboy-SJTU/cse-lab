// yfs client.  implements FS operations using extent and lock server
#include "yfs_client.h"
#include "extent_client.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

yfs_client::yfs_client(std::string extent_dst, std::string lock_dst)
{
  ec = new extent_client(extent_dst);
  lc = new lock_client(lock_dst);
  if (ec->put(1, "") != extent_protocol::OK)
      printf("error init root dir\n"); // XYB: init root dir
}

yfs_client::inum
yfs_client::n2i(std::string n)
{
    std::istringstream ist(n);
    unsigned long long finum;
    ist >> finum;
    return finum;
}

std::string
yfs_client::filename(inum inum)
{
    std::ostringstream ost;
    ost << inum;
    return ost.str();
}

bool
yfs_client::isfile(inum inum)
{
    lc->acquire(inum);
    extent_protocol::attr a;

    if (ec->getattr(inum, a) != extent_protocol::OK) {
        printf("error getting attr\n");
        lc->release(inum);
        return false;
    }

    if (a.type == extent_protocol::T_FILE) {
        printf("isfile: %lld is a file\n", inum);
        lc->release(inum);
        return true;
    } 

    //printf("isfile: %lld is a dir\n", inum);
    lc->release(inum);
    return false;
}
/** Your code here for Lab...
 * You may need to add routines such as
 * readlink, issymlink here to implement symbolic link.
 * 
 * */

bool
yfs_client::isdir(inum inum)             //修改，适应symbolic link
{
    // Oops! is this still correct when you implement symlink?
    //return ! isfile(inum);
    lc->acquire(inum);
    extent_protocol::attr a;

    if (ec->getattr(inum, a) != extent_protocol::OK) {
        printf("error getting attr\n");
        lc->release(inum);
        return false;
    }

    if (a.type == extent_protocol::T_DIR) {
        printf("isfile: %lld is a dir\n", inum);
        lc->release(inum);
        return true;
    } 

    lc->release(inum);
    return false;
}

bool
yfs_client::issymlink(inum inum)    //判断是否为symbolic link类型
{
    lc->acquire(inum);
    extent_protocol::attr a;
    
    if (ec->getattr(inum, a) != OK) 
    {
        printf("error getting attr\n");
        lc->release(inum);
        return false;
    }

    if (a.type == extent_protocol::T_SYMLINK) 
    {
        printf("isfile: %lld is a symlink\n", inum);
        lc->release(inum);
        return true;
    } 
    lc->release(inum);
    return false;
}

int
yfs_client::getfile(inum inum, fileinfo &fin)
{
    lc->acquire(inum);
    int r = OK;

    printf("getfile %016llx\n", inum);
    extent_protocol::attr a;
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        r = IOERR;
        goto release;
    }

    fin.atime = a.atime;
    fin.mtime = a.mtime;
    fin.ctime = a.ctime;
    fin.size = a.size;
    printf("getfile %016llx -> sz %llu\n", inum, fin.size);

release:
    lc->release(inum);
    return r;
}

int
yfs_client::getdir(inum inum, dirinfo &din)
{
    lc->acquire(inum);
    int r = OK;

    printf("getdir %016llx\n", inum);
    extent_protocol::attr a;
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        r = IOERR;
        goto release;
    }
    din.atime = a.atime;
    din.mtime = a.mtime;
    din.ctime = a.ctime;

release:
    lc->release(inum);
    return r;
}


#define EXT_RPC(xx) do { \
    if ((xx) != extent_protocol::OK) { \
        printf("EXT_RPC Error: %s:%d \n", __FILE__, __LINE__); \
        r = IOERR; \
        goto release; \
    } \
} while (0)


// Only support set size of attr
int
yfs_client::setattr(inum ino, size_t size)
{
    lc->acquire(ino);
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: get the content of inode ino, and modify its content
     * according to the size (<, =, or >) content length.
     */

    std::string file_buf;
    r = ec->get(ino,file_buf);
    if(r!= OK)
    {  
       lc->release(ino);
       return r;
    }
    file_buf.resize(size,0);
    r = ec->put(ino,file_buf); 
    lc->release(ino); 
    return r;
}

int
yfs_client::create(inum parent, const char *name, mode_t mode, inum &ino_out)     //未用mode
{
    int r = OK;
    /*
     * your lab2 code goes here.
     * note: lookup is what you need to check if file exist;
     * after create file or dir, you must remember to modify the parent infomation.
     */
     lc->acquire(parent);
     bool found = 0;
     inum ino;
     std::string file_buf,tmp,file_name;
     r = ec->get(parent,file_buf);
     if(r != OK)
     {
        lc->release(parent);
        return r;
     }
    lookup_without_lock(parent,name,found,ino);      //判断是否已存在
    if(found == true)
    {
        lc->release(parent);
        return EXIST;
    }
     r = ec->create(extent_protocol::T_FILE,ino_out);
     if(r != OK)
     {
        lc->release(parent);
        return r;
     }
     tmp = name;
     file_buf += tmp+"/"+filename(ino_out)+"/";   //dirent添加到文件夹末尾，具体格式为 file_name/inode_num/ (本来打算用“ ”来做分隔符，但文件名支持空格，用“/”来代替）
     r = ec->put(parent,file_buf);
     lc->release(parent);
     return r;
}

int
yfs_client::mkdir(inum parent, const char *name, mode_t mode, inum &ino_out)
{
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: lookup is what you need to check if directory exist;
     * after create file or dir, you must remember to modify the parent infomation.
     */
     lc->acquire(parent);
     std::string file_buf,tmp,file_name;
     bool found = 0;
     inum ino;
     if(strcmp(name,".") == 0 || strcmp(name,"..") == 0)     //不能名为.或..
     {
         lc->release(parent);
         return EXIST;
     }
     r = ec->get(parent,file_buf);
     if(r != OK)
     {
        lc->release(parent);
        return r;
     }
     lookup_without_lock(parent,name,found,ino);   //查看文件或文件夹是否存在
     if(found == true)
     {
        lc->release(parent);
        return EXIST;
     }
     r = ec->create(extent_protocol::T_DIR,ino_out);
     if(r != OK)
     {
        lc->release(parent);
        return r;
     }
     tmp = name;
     file_buf += tmp+"/"+filename(ino_out)+"/";   //dirent添加到上一级文件夹末尾
     r = ec->put(parent,file_buf);
     lc->release(parent);
     return r;
}


int
yfs_client::lookup_without_lock(inum parent, const char *name, bool &found, inum &ino_out)
{

    int r = OK;    
    /*
     * your lab2 code goes here.
     * note: lookup file from parent dir according to name;
     * you should design the format of directory content.
     */
    std::string file_buf,file_name,file_inum;   //读取文件夹中direntry的内容，如果有相同的文件名，更新found和ino_out
    int begin = 0,end;
    r = ec->get(parent,file_buf);
    if(r != OK)
    {
        found = false;
        return NOENT;
    }
    while((size_t)begin <= file_buf.size())
    {
       end = file_buf.find('/',begin);
       if(end == -1)
       {
          return NOENT;
       }
       file_name = file_buf.substr(begin,end-begin);
       begin = end + 1;
       end = file_buf.find('/',begin);
       file_inum = file_buf.substr(begin,end-begin);
       if(strcmp(file_name.c_str(),name) == 0)
       {
         found = true;
         ino_out = n2i(file_inum);
         return OK;
       }
       begin = end + 1;
    }   
    found = false;
    return NOENT;
}

int
yfs_client::lookup(inum parent, const char *name, bool &found, inum &ino_out)
{

    int r = OK;    
    /*
     * your lab2 code goes here.
     * note: lookup file from parent dir according to name;
     * you should design the format of directory content.
     */
    lc->acquire(parent);
    std::string file_buf,file_name,file_inum;   //读取文件夹中direntry的内容，如果有相同的文件名，更新found和ino_out
    int begin = 0,end;
    r = ec->get(parent,file_buf);
    if(r != OK)
    {
        found = false;
        lc->release(parent);
        return NOENT;
    }
    while((size_t)begin <= file_buf.size())
    {
       end = file_buf.find('/',begin);
       if(end == -1)
       {
          lc->release(parent);
          return NOENT;
       }
       file_name = file_buf.substr(begin,end-begin);
       begin = end + 1;
       end = file_buf.find('/',begin);
       file_inum = file_buf.substr(begin,end-begin);
       if(strcmp(file_name.c_str(),name) == 0)
       {
         found = true;
         ino_out = n2i(file_inum);
         lc->release(parent);
         return OK;
       }
       begin = end + 1;
    }   
    found = false;
    lc->release(parent);
    return NOENT;
}



int
yfs_client::readdir(inum dir, std::list<dirent> &list)   
{
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: you should parse the dirctory content using your defined format,
     * and push the dirents to the list.
     */
    lc->acquire(dir);
    std::string file_buf,file_name,file_inum;
    inum ino_out;
    int begin = 0, end;
    r = ec->get(dir,file_buf);
    if(r != OK)
    {
        lc->release(dir);
        return r;
    }
    while((size_t)begin <= file_buf.size())
    {
       end = file_buf.find('/',begin);
       if(end == -1)
       {
          lc->release(dir);
          return NOENT;
       }
       file_name = file_buf.substr(begin,end-begin);
       begin = end + 1;
       end = file_buf.find('/',begin);
       file_inum = file_buf.substr(begin,end-begin);
       ino_out = n2i(file_inum);
       dirent dir_entry = {file_name,ino_out};    //遍历文件夹中的entry，保存在list中
       list.push_back(dir_entry);
       begin = end + 1;
    }
    lc->release(dir);
    return r;  
}

int
yfs_client::read(inum ino, size_t size, off_t off, std::string &data)
{
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: read using ec->get().
     */
    lc->acquire(ino);
    std::string file_buf;
    r = ec->get(ino,file_buf);
    if(r != OK)
    {
       lc->release(ino);
       return r;
    }
    if(off > (off_t)file_buf.length())            //如果offset大于文件长度，返回空字符串
        data ="";
    else
    {
        if(off + size > file_buf.length())   //如果offset+size大于文件长度，读到文件末尾为止
            size = file_buf.length()-off;
        data = file_buf.substr(off,size);       
    }
    lc->release(ino);
    return r;
}

int
yfs_client::write(inum ino, size_t size, off_t off, const char *data,
        size_t &bytes_written)
{
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: write using ec->put().
     * when off > length of original file, fill the holes with '\0'.
     */
    lc->acquire(ino);
    std::string file_buf;
    r = ec->get(ino,file_buf);
    if(r != OK)
    {
       lc->release(ino);
       return r;
    }
    if(off + size > file_buf.length())  //扩大字符串长度
        file_buf.resize(off + size, 0);
    
    //file_buf.replace(off,size,data,0,size);   //用这个无法通过e测试，求解惑
    for(size_t i = 0; i < size; i++)     //用一个个字符赋值的方式修改文件，用replace函数在blob file处会出现错误
    {
      file_buf[off + i] = data[i];
    }
    r = ec->put(ino,file_buf);
    bytes_written = size;
    lc->release(ino);
    return r;
}

int yfs_client::unlink(inum parent,const char *name)
{
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: you should remove the file using ec->remove,
     * and update the parent directory content.
     */
     lc->acquire(parent);
     std::string file_buf;
     std::string tmp,file_name;
     int begin = 0,begin_inum,end;
     inum file_inum;
     if(strcmp(name,".") == 0 || strcmp(name,"..") == 0)  //无法unlink.或..
     {
         lc->release(parent);
         return IOERR;
     }
     r = ec->get(parent,file_buf);
     if(r != OK)
     {
        lc->release(parent);
        return r;
     }
      while((size_t)begin <= file_buf.size())
     {
       end = file_buf.find('/',begin);
       if(end == -1)
         break;
       file_name = file_buf.substr(begin,end-begin);
       if(strcmp(file_name.c_str(),name) == 0)
       {
           // std::string str1,str2;
            begin_inum = end + 1;
            end = file_buf.find('/',begin_inum);
            file_inum = n2i(file_buf.substr(begin_inum,end-begin));
            /*str1 = file_buf.substr(0,begin);
            if((size_t)end == file_buf.size()-1)
                  str2 = "";
            else 
                  str2 = file_buf.substr(end+1,file_buf.size()-end-1);
            str1.append(str2);*/
            file_buf.replace(begin,end-begin+1,"");  //如果找到同名文件或文件夹，则将entry替换成空字符串
            r = ec->put(parent,file_buf);
            //r = ec->put(parent,str1);
            r = ec->remove(file_inum);
            lc->release(parent);
            return r;
       }
       begin = end + 1;
       end = file_buf.find('/',begin); 
       begin = end + 1;
     }
     lc->release(parent);
     return NOENT;
}

int yfs_client::symlink(inum parent, const char *name, const char *link, inum &ino_out)
{
  int r = OK;
  lc->acquire(parent);
  bool found = 0;
  inum ino;
  std::string file_buf,tmp,file_name;
  r = ec->get(parent,file_buf);
  if(r != OK)
  {
      lc->release(parent);
      return r;
  }
  lookup_without_lock(parent,name,found,ino);
  if(found == true)
  {
     lc->release(parent);
     return EXIST;
  }
  r = ec->create(extent_protocol::T_SYMLINK,ino_out);
  if(r != OK)
  {
      lc->release(parent);
      return r;
  }
  tmp = name;
  file_buf += tmp+"/"+filename(ino_out)+"/";
  r = ec->put(parent,file_buf);
  if(r != OK)
  {
      lc->release(parent);
      return r;
  }
  r = ec->put(ino_out,link);   //在文件中写入link，实现symlink
  lc->release(parent);
  return r;
}

int yfs_client::readlink(inum ino, std::string &link)
{
  int r = OK;
  lc->acquire(ino);
  r = ec->get(ino, link);  //获得symlink文件中的链接
  lc->release(ino);
  return r;
}

