/*
 * receive request from fuse and call methods of yfs_client
 *
 * started life as low-level example in the fuse distribution.  we
 * have to use low-level interface in order to get i-numbers.  the
 * high-level interface only gives us complete paths.
 */

#include <fuse_lowlevel.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fstream>
#include "lang/verify.h"
#include "yfs_client.h"

int myid;
yfs_client *yfs;

static int last_version = 0;
static int now_version = 0;

typedef struct log_ {
   int version;
   enum {t_write,t_create,t_unlink} kind;
   yfs_client::inum ino;
   yfs_client::inum parent;
   std::string name;
   int old_size;
   int new_size;
   std::string oldbuf;
   std::string newbuf;
   int type;  
}log;


std::vector<log> parselog()
{
   std::fstream fs;
   fs.open("file.log");
   std::vector<log> logs;
   while(!fs.eof())
   {
      log vlog;
      vlog.name = "";
      vlog.oldbuf="";
      vlog.newbuf="";
      char v = fs.get();
      if(v != 'V')
         break;
      fs >> vlog.version;
      fs.get();
      v = fs.get();
      switch(v){
        case 'c':
        {
          vlog.kind = log::t_create;
          fs.get();
          fs>>vlog.type;
          fs.get();
          fs>>vlog.parent;
          fs.get();
          char ch;
          while((ch = fs.get()) != '/')
              vlog.name += ch;
          fs.get();
          break;
        }
        case 'w':{
          vlog.kind = log::t_write;
          fs.get();
          fs >> vlog.ino;
          fs.get();
          fs >> vlog.old_size;
          fs >> vlog.new_size;
          fs.get();
          for(int i = 0;i<vlog.old_size;i++)
          {
             char ch;
             ch = fs.get();
             vlog.oldbuf+=ch;
          }
          fs.get();
          for(int i = 0;i<vlog.new_size;i++)
          {
             char ch;
             ch = fs.get();
             vlog.newbuf += ch;
          }
          fs.get();
          break;
        }
        case 'u':{
          vlog.kind = log::t_unlink;
          fs.get();
          fs >>vlog.parent;
          fs.get();
          char ch;
          while((ch = fs.get()) != '/')
              vlog.name += ch;
          fs.get();
          fs >> vlog.old_size;
          fs.get();
          for(int i = 0;i<vlog.old_size;i++)
          {
             char ch;
             ch = fs.get();
             vlog.oldbuf += ch;
          }
          fs.get();
          break;
        }
      }
      logs.push_back(vlog);
   }   
   fs.close();
   return logs;
}

std::vector<log> getlog(std::vector<log> &logs,int version)
{
   std::vector<log> vlogs;
   for(int i = 0;i<logs.size();i++)
   {
      if(logs[i].version == version)
         vlogs.push_back(logs[i]);
   }
   return vlogs;
}

void printlog(std::vector<log> & logs)
{
   std::fstream fs;
   fs.open("check.log",std::ios::out);
   printf("12345\n");
   for(unsigned int i = 0;i<logs.size();i++){
     log vlog = logs[i];
     printf("123456\n");
     switch(vlog.kind){
       case log::t_write:{
          fs <<'V'<<vlog.version<<" "<<"w "<<vlog.ino<<" "<<vlog.old_size<<" "<<vlog.new_size<<" "<<vlog.oldbuf<<" "<<vlog.newbuf<<"\n";
          break;
       }
       case log::t_create:{
          fs <<'V'<<vlog.version<<" "<<"c "<<vlog.type<<" "<<vlog.parent<<" "<<vlog.name<<"/\n";
          break;
       }
       case log::t_unlink:{
          fs<<'V'<<vlog.version<<" "<<"u "<<vlog.parent<<" "<<vlog.name<<"/ "<<vlog.old_size<<" "<<vlog.oldbuf<<"\n";
          break;
       }
    }
   }
   fs.close();
}

void undo(int version)
{
   std::vector<log> logs = parselog();
   std::vector<log> vlogs = getlog(logs,version+1);
   for(int i = vlogs.size()-1;i >=0;i--)
   {
      log vlog = vlogs[i];
      switch(vlog.kind){
        case log::t_create:{
          yfs->unlink(vlog.parent,vlog.name.c_str());
          break;
        }
        case log::t_write:{
          size_t byte_written;
          yfs->write(vlog.ino,vlog.old_size,0,vlog.oldbuf.c_str(),byte_written);
          break;
        }
        case log::t_unlink:{
          yfs_client::inum ino;
          mode_t mode = 0;
          yfs->create(vlog.parent,vlog.name.c_str(),mode,ino);      
          size_t byte_written;
          yfs->write(ino,vlog.old_size,0,vlog.oldbuf.c_str(),byte_written);
          break;
        }
     }
   }
   return;
}

void redo(int version)
{
   std::vector<log> logs = parselog();
   std::vector<log> vlogs = getlog(logs,version);
   for(int i = 0;i<vlogs.size();i++)
   {
      log vlog = vlogs[i];
      switch(vlog.kind){
         case log::t_create:{
           yfs_client::inum ino;
           mode_t mode = 0;
           yfs->create(vlog.parent,vlog.name.c_str(),mode,ino);
           break;
         }
         case log::t_write:{
            size_t byte_written;
            yfs->write(vlog.ino,vlog.new_size,0,vlog.newbuf.c_str(),byte_written);
            break;
         }
         case log::t_unlink:{
            yfs->unlink(vlog.parent,vlog.name.c_str());
            break;
         }
      }
   }
   return;
}
      
        



int id() { 
    return myid;
}

 void sig_handler(int no) {
      switch (no) {
      case SIGINT:{
          printf("commit a new version\n");
          if(now_version != last_version)
          {
              printf("cannot commit when you are in an old version\n");
              return;
          }
          last_version++;
          now_version++;
          break;
      }
      case SIGUSR1:{
          printf("to previous version\n");
          if(now_version-1 < 0)
          {
            printf("no previous version\n");
            return;
          }
          now_version--;
          undo(now_version);              
          break;
      }
      case SIGUSR2:
          printf("to next version\n");
          if(now_version >= last_version)
          {
              printf("no next version\n");
              return;
          }
          now_version++;
          redo(now_version);
          break;
      }
}

//
// A file/directory's attributes are a set of information
// including owner, permissions, size, &c. The information is
// much the same as that returned by the stat() system call.
// The kernel needs attributes in many situations, and some
// fuse functions (such as lookup) need to return attributes
// as well as other information, so getattr() gets called a lot.
//
// YFS fakes most of the attributes. It does provide more or
// less correct values for the access/modify/change times
// (atime, mtime, and ctime), and correct values for file sizes.
//
yfs_client::status
getattr(yfs_client::inum inum, struct stat &st)
{
    yfs_client::status ret;

    bzero(&st, sizeof(st));

    st.st_ino = inum;
    if(yfs->isfile(inum)){
        yfs_client::fileinfo info;
        ret = yfs->getfile(inum, info);
        if(ret != yfs_client::OK)
            return ret;
        st.st_mode = S_IFREG | (info.mode & 0777);
        st.st_nlink = 1;
        st.st_atime = info.atime;
        st.st_mtime = info.mtime;
        st.st_ctime = info.ctime;
        st.st_size = info.size;
        st.st_uid = info.uid;
        st.st_gid = info.gid;
    } else if(yfs->isdir(inum)){
        yfs_client::dirinfo info;
        ret = yfs->getdir(inum, info);
        if(ret != yfs_client::OK)
            return ret;
        st.st_mode = S_IFDIR | (info.mode & 0777);
        st.st_nlink = 2;
        st.st_atime = info.atime;
        st.st_mtime = info.mtime;
        st.st_ctime = info.ctime;
	st.st_uid = info.uid;
	st.st_gid = info.gid;
    }
    else if(yfs->issymlink(inum)){    //新增判断issymlink
       yfs_client::fileinfo info;
       ret = yfs->getfile(inum, info);
       if(ret != yfs_client::OK)
            return ret;
        st.st_mode = S_IFLNK | 0755;   //此处存疑，按照提供的网址采用|0755
        st.st_nlink = 2;
        st.st_atime = info.atime;
        st.st_mtime = info.mtime;
        st.st_ctime = info.ctime;
        st.st_size = info.size;
        printf("   getattr -> %lu %lu %lu\n", info.atime, info.mtime, info.ctime);
    }
    return yfs_client::OK;
}

//
// This is a typical fuse operation handler; you'll be writing
// a bunch of handlers like it.
//
// A handler takes some arguments
// and supplies either a success or failure response. It provides
// an error response by calling either fuse_reply_err(req, errno), and
// a normal response by calling ruse_reply_xxx(req, ...). The req
// argument serves to link up this response with the original
// request; just pass the same @req that was passed into the handler.
// 
// The @ino argument indicates the file or directory FUSE wants
// you to operate on. It's a 32-bit FUSE identifier; just assign
// it to a yfs_client::inum to get a 64-bit YFS inum.
//
void
fuseserver_getattr(fuse_req_t req, fuse_ino_t ino,
        struct fuse_file_info *fi)
{
    struct stat st;
    yfs_client::inum inum = ino; // req->in.h.nodeid;
    yfs_client::status ret;

    ret = getattr(inum, st);
    if(ret != yfs_client::OK){
		if (ret == yfs_client::NOPEM) {
			fuse_reply_err(req, EACCES);
		} else {
        	fuse_reply_err(req, ENOENT);
		}
		return;
    }
    fuse_reply_attr(req, &st, 0);
}

//
// Set the attributes of a file. Often used as part of overwriting
// a file, to set the file length to zero.
//
// to_set is a bitmap indicating which attributes to set. You only
// have to implement the FUSE_SET_ATTR_SIZE bit, which indicates
// that the size of the file should be changed. The new size is
// in attr->st_size. If the new size is bigger than the current
// file size, fill the new bytes with '\0'.
//
// On success, call fuse_reply_attr, passing the file's new
// attributes (from a call to getattr()).
//

#define LAB6_ATTR_MASK  FUSE_SET_ATTR_SIZE|FUSE_SET_ATTR_GID|FUSE_SET_ATTR_UID|FUSE_SET_ATTR_MODE

void
fuseserver_setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr,
        int to_set, struct fuse_file_info *fi)
{
    printf("fuseserver_setattr 0x%x\n", to_set);
	printf("attr->mode %o\n", attr->st_mode);
	printf("attr->u_id %d\n", attr->st_uid);
	printf("attr->g_id %d\n", attr->st_gid);

	yfs_client::status ret;

	if (LAB6_ATTR_MASK & to_set) {
        struct stat st;
		yfs_client::filestat fst;
		fst.mode = attr->st_mode;
		fst.uid = attr->st_uid;
		fst.gid = attr->st_gid;
		fst.size = attr->st_size;

#if 1
        // Change the above line to "#if 1", and your code goes here
        // Note: fill st using getattr before fuse_reply_attr
        int flag = 0;
        if(to_set & FUSE_SET_ATTR_MODE)
           flag = flag | 0x1;
        if(to_set & FUSE_SET_ATTR_UID)
           flag = flag | 0x10;
        if(to_set & FUSE_SET_ATTR_GID)
           flag = flag | 0x100;
        if(to_set & FUSE_SET_ATTR_SIZE)
           flag = flag | 0x1000;
        ret = yfs->setattr(ino,fst,flag);
		if (ret != yfs_client::OK) {
			if (ret == yfs_client::NOPEM) {
				fuse_reply_err(req, EACCES);
			} else {
        		fuse_reply_err(req, ENOENT);
			}
			return ;
		}
        getattr(ino, st);
        fuse_reply_attr(req, &st, 0);
#else
    fuse_reply_err(req, ENOSYS);
#endif

    } else {
        //fuse_reply_err(req, ENOSYS);
        struct stat st; 
        getattr(ino, st);
        fuse_reply_attr(req, &st, 0);
    }
}

//
// Read up to @size bytes starting at byte offset @off in file @ino.
//
// Pass the number of bytes actually read to fuse_reply_buf.
// If there are fewer than @size bytes to read between @off and the
// end of the file, read just that many bytes. If @off is greater
// than or equal to the size of the file, read zero bytes.
//
// Ignore @fi. 
// @req identifies this request, and is used only to send a 
// response back to fuse with fuse_reply_buf or fuse_reply_err.
//
void
fuseserver_read(fuse_req_t req, fuse_ino_t ino, size_t size,
        off_t off, struct fuse_file_info *fi)
{
#if 1
    std::string buf;
    // Change the above "#if 0" to "#if 1", and your code goes here
    int r;
    if ((r = yfs->read(ino, size, off, buf)) == yfs_client::OK) {
        fuse_reply_buf(req, buf.data(), buf.size());    
    } else {
		if (r == yfs_client::NOPEM) {
			fuse_reply_err(req, EACCES);
		} else {
        	fuse_reply_err(req, ENOENT);
		}
    }
#else
    fuse_reply_err(req, ENOSYS);
#endif


}



//Symbolic link
void
fuseserver_symlink(fuse_req_t req, const char *link, fuse_ino_t parent,     //创建接口，来调用yfs->symlink
			 const char *name)
{
    yfs_client::inum ino;
    struct fuse_entry_param e;
    // In yfs, timeouts are always set to 0.0, and generations are always set to 0
    e.attr_timeout = 0.0;
    e.entry_timeout = 0.0;
    e.generation = 0;
 #if 1
    // Change the above line to "#if 1", and your code goes here
    yfs_client::status ret;
    if ((ret = yfs->symlink(parent,name,link,ino)) == yfs_client::OK) {
        e.ino = ino;
        ret = getattr(ino,e.attr);
        fuse_reply_entry(req, &e);    //reply_entry   
    } else {
        if (ret == yfs_client::EXIST) {
            fuse_reply_err(req, EEXIST);
        } else {
            fuse_reply_err(req, ENOENT);
        }
    }
#else
    fuse_reply_err(req, ENOSYS);
#endif
}

void fuseserver_readlink (fuse_req_t req, fuse_ino_t ino)   //调用yfs->readlink
{
    #if 1
    // Change the above line to "#if 1", and your code goes here
    yfs_client::status ret;
    std::string link;
    if ( (ret = yfs->readlink(ino,link)) == yfs_client::OK) {
        fuse_reply_readlink(req, link.c_str());      //reply_readlink
    } else {
        fuse_reply_err(req, ENOENT);
    }
#else
    fuse_reply_err(req, ENOSYS);
#endif

}
//
// Write @size bytes from @buf to file @ino, starting
// at byte offset @off in the file.
//
// If @off + @size is greater than the current size of the
// file, the write should cause the file to grow. If @off is
// beyond the end of the file, fill the gap with null bytes.
//
// Set the file's mtime to the current time.
//
// Ignore @fi.
//
// @req identifies this request, and is used only to send a 
// response back to fuse with fuse_reply_buf or fuse_reply_err.
//
void
fuseserver_write(fuse_req_t req, fuse_ino_t ino,
        const char *buf, size_t size, off_t off,
        struct fuse_file_info *fi)
{
#if 1
    // Change the above line to "#if 1", and your code goes here
    int r;
    std::string oldbuf;
    std::string newbuf;
    std::fstream fs;
    yfs_client::fileinfo fin; 
    yfs_client::fileinfo newfin;
    yfs->getfile(ino,fin);
    yfs->read(ino,fin.size,0,oldbuf);
    if ((r = yfs->write(ino, size, off, buf, size)) == yfs_client::OK) {
        fuse_reply_write(req, size);
        fs.open("file.log",std::ios::app|std::ios::out);
        yfs->getfile(ino,newfin);
        yfs->read(ino,newfin.size,0,newbuf);
        fs <<"V"<<last_version<<" "<<"w "<<ino<<" "<<fin.size<<" "<<newfin.size<< " "<<oldbuf<<" "<<newbuf<<"\n";
        fs.close();        
    } else {
		if (r == yfs_client::NOPEM) {
			fuse_reply_err(req, EACCES);
		} else {
        	fuse_reply_err(req, ENOENT);
		}
    }
#else
    fuse_reply_err(req, ENOSYS);
#endif
}

//
// Create file @name in directory @parent. 
//
// - @mode specifies the create mode of the file. Ignore it - you do not
//   have to implement file mode.
// - If a file named @name already exists in @parent, return EXIST.
// - Pick an ino (with type of yfs_client::inum) for file @name. 
//   Make sure ino indicates a file, not a directory!
// - Create an empty extent for ino.
// - Add a <name, ino> entry into @parent.
// - Change the parent's mtime and ctime to the current time/date
//   (this may fall naturally out of your extent server code).
// - On success, store the inum of newly created file into @e->ino, 
//   and the new file's attribute into @e->attr. Get the file's
//   attributes with getattr().
//
// @return yfs_client::OK on success, and EXIST if @name already exists.
//
yfs_client::status
fuseserver_createhelper(fuse_ino_t parent, const char *name,
        mode_t mode, struct fuse_entry_param *e, int type)
{
    int ret;
    // In yfs, timeouts are always set to 0.0, and generations are always set to 0
    e->attr_timeout = 0.0;
    e->entry_timeout = 0.0;
    e->generation = 0;

    yfs_client::inum inum;
    if ( type == extent_protocol::T_FILE)
		ret = yfs->create(parent, name, mode, inum);
	else 
		ret = yfs->mkdir(parent,name,mode,inum);
    if (ret != yfs_client::OK)
        return ret;
    std::fstream fs;
    fs.open("file.log",std::ios::app|std::ios::out);
    fs<<"V"<<last_version<<" "<<"c "<<type<< " "<<parent<<" "<<std::string(name)<<"/\n";
    fs.close();
    e->ino = inum;
    ret = getattr(inum, e->attr);
    return ret;
}

void
fuseserver_create(fuse_req_t req, fuse_ino_t parent, const char *name,
        mode_t mode, struct fuse_file_info *fi)
{
    struct fuse_entry_param e;
    yfs_client::status ret;
    if( (ret = fuseserver_createhelper( parent, name, mode, &e, extent_protocol::T_FILE)) == yfs_client::OK ) {
        fuse_reply_create(req, &e, fi);
        printf("OK: create returns.\n");
    } else {
        if (ret == yfs_client::EXIST) {
            fuse_reply_err(req, EEXIST);
        } else if (ret == yfs_client::NOPEM) {
			fuse_reply_err(req, EACCES);
		}else{
            fuse_reply_err(req, ENOENT);
        }
    }
}

void fuseserver_mknod( fuse_req_t req, fuse_ino_t parent, 
        const char *name, mode_t mode, dev_t rdev ) {
    struct fuse_entry_param e;
    yfs_client::status ret;
    if( (ret = fuseserver_createhelper( parent, name, mode, &e, extent_protocol::T_FILE)) == yfs_client::OK ) {
        fuse_reply_entry(req, &e);
    } else {
        if (ret == yfs_client::EXIST) {
            fuse_reply_err(req, EEXIST);
        }else if (ret == yfs_client::NOPEM) {
			fuse_reply_err(req, EACCES);
		}else{
            fuse_reply_err(req, ENOENT);
        }
    }
}

//
// Look up file or directory @name in the directory @parent. If @name is
// found, set e.attr (using getattr) and e.ino to the attribute and inum of
// the file.
//
void
fuseserver_lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    struct fuse_entry_param e;
    // In yfs, timeouts are always set to 0.0, and generations are always set to 0
    e.attr_timeout = 0.0;
    e.entry_timeout = 0.0;
    e.generation = 0;
    bool found = false;
	yfs_client::status ret;

     yfs_client::inum ino;
     ret = yfs->lookup(parent, name, found, ino);

	if (ret == yfs_client::NOPEM){
		fuse_reply_err(req, EACCES);
		return;
	}
    if (found) {
        e.ino = ino;
        getattr(ino, e.attr);
        fuse_reply_entry(req, &e);
    } else {
        	fuse_reply_err(req, ENOENT);
    }

}


struct dirbuf {
    char *p;
    size_t size;
};

void dirbuf_add(struct dirbuf *b, const char *name, fuse_ino_t ino)
{
    struct stat stbuf;
    size_t oldsize = b->size;
    b->size += fuse_dirent_size(strlen(name));
    b->p = (char *) realloc(b->p, b->size);
    memset(&stbuf, 0, sizeof(stbuf));
    stbuf.st_ino = ino;
    fuse_add_dirent(b->p + oldsize, name, &stbuf, b->size);
}

#define min(x, y) ((x) < (y) ? (x) : (y))

int reply_buf_limited(fuse_req_t req, const char *buf, size_t bufsize,
        off_t off, size_t maxsize)
{
    if ((size_t)off < bufsize)
        return fuse_reply_buf(req, buf + off, min(bufsize - off, maxsize));
    else
        return fuse_reply_buf(req, NULL, 0);
}

//
// Retrieve all the file names / i-numbers pairs
// in directory @ino. Send the reply using reply_buf_limited.
//
// You can ignore @size and @off (except that you must pass
// them to reply_buf_limited).
//
// Call dirbuf_add(&b, name, inum) for each entry in the directory.
//
void
fuseserver_readdir(fuse_req_t req, fuse_ino_t ino, size_t size,
        off_t off, struct fuse_file_info *fi)
{
    yfs_client::inum inum = ino; // req->in.h.nodeid;
    struct dirbuf b;

    printf("fuseserver_readdir\n");

    if(!yfs->isdir(inum)){
        fuse_reply_err(req, ENOTDIR);
        return;
    }

    memset(&b, 0, sizeof(b));

    std::list<yfs_client::dirent> entries;
    yfs->readdir(inum, entries);
    for (std::list<yfs_client::dirent>::iterator it = entries.begin(); it != entries.end(); ++it) {
        dirbuf_add(&b, it->name.c_str(), (fuse_ino_t) it->inum);
    }

    reply_buf_limited(req, b.p, b.size, off, size);
    free(b.p);
}


void
fuseserver_open(fuse_req_t req, fuse_ino_t ino,
        struct fuse_file_info *fi)
{
    fuse_reply_open(req, fi);
}

//
// Create a new directory with name @name in parent directory @parent.
// Leave new directory's inum in e.ino and attributes in e.attr.
//
// The new directory should be empty (no . or ..).
// 
// If a file/directory named @name already exists, indicate error EEXIST.
//
// Ignore mode.
//
void
fuseserver_mkdir(fuse_req_t req, fuse_ino_t parent, const char *name,
        mode_t mode)
{
    struct fuse_entry_param e;
    // In yfs, timeouts are always set to 0.0, and generations are always set to 0
    e.attr_timeout = 0.0;
    e.entry_timeout = 0.0;
    e.generation = 0;
    // Suppress compiler warning of unused e.
    (void) e;

#if 1
    // Change the above line to "#if 1", and your code goes here
    yfs_client::status ret;
    if (( ret = fuseserver_createhelper( parent, name, mode, &e, extent_protocol::T_DIR)) == yfs_client::OK) {
        fuse_reply_entry(req, &e);    
    } else {
        if (ret == yfs_client::EXIST) {
            fuse_reply_err(req, EEXIST);
        } else if (ret == yfs_client::NOPEM) {
			fuse_reply_err(req, EACCES);
		}else {
            fuse_reply_err(req, ENOENT);
        }
    }
#else
    fuse_reply_err(req, ENOSYS);
#endif

}

//
// Remove the file named @name from directory @parent.
// Free the file's extent.
// If the file doesn't exist, indicate error ENOENT.
//
// Do *not* allow unlinking of a directory.
//
void
fuseserver_unlink(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    int r;
    std::string oldbuf;
    yfs_client::fileinfo fin; 
    bool found;
    yfs_client::inum ino;
    yfs->lookup(parent,name,found,ino);
    yfs->getfile(ino,fin);
    yfs->read(ino,fin.size,0,oldbuf);
    if ((r = yfs->unlink(parent, name)) == yfs_client::OK) {
        std::fstream fs;
        fs.open("file.log",std::ios::app|std::ios::out);
        fs<<"V"<<last_version<<" "<<"u "<<parent<<" "<<std::string(name)<<"/ "<<fin.size<<" "<<oldbuf<<"\n";
        fs.close();
        fuse_reply_err(req, 0);
    } else {
        if (r == yfs_client::NOENT) {
            fuse_reply_err(req, ENOENT);
        } else if (r == yfs_client::NOPEM) {
			fuse_reply_err(req, EACCES);
		}else{
            fuse_reply_err(req, ENOTEMPTY);
        }
    }
}

void
fuseserver_statfs(fuse_req_t req)
{
    struct statvfs buf;

    printf("statfs\n");

    memset(&buf, 0, sizeof(buf));

    buf.f_namemax = 255;
    buf.f_bsize = 512;

    fuse_reply_statfs(req, &buf);
}

struct fuse_lowlevel_ops fuseserver_oper;

int
main(int argc, char *argv[])
{
    char *mountpoint = 0;
    int err = -1;
    int fd;
    
    signal(SIGINT,sig_handler);
    signal(SIGUSR1,sig_handler);
    signal(SIGUSR2,sig_handler);

    FILE* filelog = NULL;
    filelog = fopen("file.log","w+");
    fclose(filelog);

    setvbuf(stdout, NULL, _IONBF, 0);


#if 1
    if(argc != 5){
        fprintf(stderr, "Usage: yfs_client <mountpoint> <port-extent-server> <port-lock-server> <user-cerficiate-file>\n");
        exit(1);
    }
#else
    if(argc != 2){
        fprintf(stderr, "Usage: yfs_client <mountpoint>\n");
        exit(1);
    }
#endif
    mountpoint = argv[1];

    srandom(getpid());

    myid = random();

    yfs = new yfs_client(argv[2], argv[3], argv[4]);
    // yfs = new yfs_client();


    fuseserver_oper.getattr    = fuseserver_getattr;
    fuseserver_oper.statfs     = fuseserver_statfs;
    fuseserver_oper.readdir    = fuseserver_readdir;
    fuseserver_oper.lookup     = fuseserver_lookup;
    fuseserver_oper.create     = fuseserver_create;
    fuseserver_oper.mknod      = fuseserver_mknod;
    fuseserver_oper.open       = fuseserver_open;
    fuseserver_oper.read       = fuseserver_read;
    fuseserver_oper.write      = fuseserver_write;
    fuseserver_oper.setattr    = fuseserver_setattr;

    fuseserver_oper.unlink     = fuseserver_unlink;

    fuseserver_oper.mkdir      = fuseserver_mkdir;
    /** Your code here for Lab.
     * you may want to add
     * routines here to implement symbolic link,
     * rmdir, etc.
     * */

    fuseserver_oper.symlink = fuseserver_symlink; //添加记录
    fuseserver_oper.readlink = fuseserver_readlink;

    const char *fuse_argv[20];
    int fuse_argc = 0;
    fuse_argv[fuse_argc++] = argv[0];
#ifdef __APPLE__
    fuse_argv[fuse_argc++] = "-o";
    fuse_argv[fuse_argc++] = "nolocalcaches"; // no dir entry caching
    fuse_argv[fuse_argc++] = "-o";
    fuse_argv[fuse_argc++] = "daemon_timeout=86400";
#endif

    // everyone can play, why not?
    //fuse_argv[fuse_argc++] = "-o";
    //fuse_argv[fuse_argc++] = "allow_other";

    fuse_argv[fuse_argc++] = mountpoint;
    fuse_argv[fuse_argc++] = "-d";

    fuse_args args = FUSE_ARGS_INIT( fuse_argc, (char **) fuse_argv );
    int foreground;
    int res = fuse_parse_cmdline( &args, &mountpoint, 0 /*multithreaded*/, 
            &foreground );
    if( res == -1 ) {
        fprintf(stderr, "fuse_parse_cmdline failed\n");
        return 0;
    }

    args.allocated = 0;

    fd = fuse_mount(mountpoint, &args);
    if(fd == -1){
        fprintf(stderr, "fuse_mount failed\n");
        exit(1);
    }

    struct fuse_session *se;

    se = fuse_lowlevel_new(&args, &fuseserver_oper, sizeof(fuseserver_oper),
            NULL);
    if(se == 0){
        fprintf(stderr, "fuse_lowlevel_new failed\n");
        exit(1);
    }

    struct fuse_chan *ch = fuse_kern_chan_new(fd);
    if (ch == NULL) {
        fprintf(stderr, "fuse_kern_chan_new failed\n");
        exit(1);
    }

    fuse_session_add_chan(se, ch);
    // err = fuse_session_loop_mt(se);   // FK: wheelfs does this; why?
    err = fuse_session_loop(se);

    fuse_session_destroy(se);
    close(fd);
    fuse_unmount(mountpoint);

    return err ? 1 : 0;
}
