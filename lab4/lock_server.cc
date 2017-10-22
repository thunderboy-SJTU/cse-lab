// the lock server implementation

#include "lock_server.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

lock_server::lock_server():
  nacquire (0)
{
   pthread_mutex_init(&lock, NULL);
}

lock_protocol::status
lock_server::stat(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
  printf("stat request from clt %d\n", clt);
  r = nacquire;
  return ret;
}

lock_protocol::status
lock_server::acquire(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
	// Your lab4 code goes here
  while(true)
  {
     pthread_mutex_lock(&lock);
     if(lockset.find(lid) == lockset.end())
     {
        lockset[lid] = 1;
        nacquire++;
        pthread_mutex_unlock(&lock);
        return ret;
     } 
     else if(lockset[lid] == 0)
     {
        lockset[lid] = 1;
        nacquire++;
        pthread_mutex_unlock(&lock);
        return ret;
     }
     else if(lockset[lid] == 1)
     {
        pthread_mutex_unlock(&lock);
        continue;
     }
  }
  return ret;
}

lock_protocol::status
lock_server::release(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
	// Your lab4 code goes here
  pthread_mutex_lock(&lock);
  if(lockset.find(lid) == lockset.end())
     ret = lock_protocol::NOENT;
  else if(lockset[lid] == 1)
  {
     lockset[lid] = 0 ;
     nacquire--;
  }
  pthread_mutex_unlock(&lock);
  return ret;
}
