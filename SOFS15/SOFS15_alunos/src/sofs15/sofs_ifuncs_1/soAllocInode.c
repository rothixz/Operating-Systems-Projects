/**
 *  \file soAllocInode.c (implementation file)
 *
 *  \author António Rui Borges - September 2012
 */

#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#include "sofs_probe.h"
#include "sofs_buffercache.h"
#include "sofs_superblock.h"
#include "sofs_inode.h"
#include "sofs_datacluster.h"
#include "sofs_basicoper.h"
#include "sofs_basicconsist.h"

/**
 *  \brief Allocate a free inode.
 *
 *  The inode is retrieved from the list of free inodes, marked in use, associated to the legal file type passed as
 *  a parameter and generally initialized. It must be free.
 *
 *  Upon initialization, the new inode has:
 *     \li the field mode set to the given type, while the free flag and the permissions are reset
 *     \li the owner and group fields set to current userid and groupid
 *     \li the <em>prev</em> and <em>next</em> fields, pointers in the double-linked list of free inodes, change their
 *         meaning: they are replaced by the <em>time of last file modification</em> and <em>time of last file
 *         access</em> which are set to current time
 *     \li the reference fields set to NULL_CLUSTER
 *     \li all other fields reset.

 *  \param type the inode type (it must represent either a file, or a directory, or a symbolic link)
 *  \param p_nInode pointer to the location where the number of the just allocated inode is to be stored
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the <em>type</em> is illegal or the <em>pointer to inode number</em> is \c NULL
 *  \return -\c ENOSPC, if the list of free inodes is empty
 *  \return -\c ESBTINPINVAL, if the table of inodes metadata in the superblock is inconsistent
 *  \return -\c ETINDLLINVAL, if the double-linked list of free inodes is inconsistent
 *  \return -\c EFININVAL, if a free inode is inconsistent
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

int soAllocInode (uint32_t type, uint32_t* p_nInode)
{
   soColorProbe (611, "07;31", "soAllocInode (%"PRIu32", %p)\n", type, p_nInode);

   if(p_nInode == NULL)                  //inode number is null
   	return -EINVAL;

   if((type != INODE_DIR) && (type != INODE_FILE) && (type != INODE_SYMLINK))     //nao e um tipo valido  
   	return -EINVAL;

   int error;
   SOSuperBlock *p_sb;                   //load do super_block        
   
   if ((error = soLoadSuperBlock ()) != 0)
    return error;
   p_sb = soGetSuperBlock ();

   if((error=soQCheckSuperBlock(p_sb)) != 0)
      return error;

   if((error=soQCheckInT(p_sb)) != 0)
      return error;
   
   if(p_sb->ifree == 0)                  //nao ha inodes livres
   	return -ENOSPC;

   uint32_t p_nBlk;                     //load da head/tail da lista de inodes, agora com 50% menos warnings
   uint32_t p_offset;
   *p_nInode = p_sb->ihdtl; //vai ser guardado na antiga head
   SOInode *p_inode;
   if((error = soConvertRefInT(p_sb->ihdtl, &p_nBlk, &p_offset)) != 0) 
    return error;
   if((error = soLoadBlockInT(p_nBlk)) != 0)
   	return error;
   p_inode = soGetBlockInT();
   
   uint32_t next, prev;

   if(p_inode[p_offset].vD1.prev == p_inode[p_offset].vD2.next){  //1 ou 2 elementos a
   	if(p_inode[p_offset].vD1.prev == p_sb->ihdtl){ //so tem 1 elemento
   		p_sb->ihdtl = NULL_INODE;
      }
   	else{                                          //2 elementos
   		next = p_inode[p_offset].vD2.next;

         if((error = soConvertRefInT(next, &p_nBlk, &p_offset)) != 0 ) //o que faltava
            return error;
         if((error = soLoadBlockInT(p_nBlk)) != 0 )
            return error;
         p_inode = soGetBlockInT();

   		p_inode[p_offset].vD1.prev = p_inode[p_offset].vD2.next = next;

         if((error = soStoreBlockInT()) != 0) //store
            return error;
   		p_sb->ihdtl = next;
   	}
   }
   else{//a lista tem mais de 2 elementos
   	prev = p_inode[p_offset].vD1.prev;
   	next = p_inode[p_offset].vD2.next;

      if((error = soConvertRefInT(next, &p_nBlk, &p_offset)) != 0 )
         return error;
      if((error=soLoadBlockInT(p_nBlk)) != 0 )
         return error;
      p_inode = soGetBlockInT();
   	p_inode[p_offset].vD1.prev = prev;
      if((error = soStoreBlockInT()) != 0) //store next
         return error;

      if((error = soConvertRefInT(prev, &p_nBlk, &p_offset)) != 0 ) 
         return error;
      if((error = soLoadBlockInT(p_nBlk)) != 0 )
         return error;
      p_inode = soGetBlockInT();
      p_inode[p_offset].vD2.next = next;
      if((error = soStoreBlockInT()) != 0)  //store prev
         return error;

   	p_sb->ihdtl = next;
   }
   p_sb->ifree--;

   if((error = soConvertRefInT(*p_nInode, &p_nBlk, &p_offset)) != 0 ) //inode que queremos
      return error;
   if((error=soLoadBlockInT(p_nBlk)) != 0 )
      return error;
   p_inode = soGetBlockInT();
   //init iNode
   p_inode[p_offset].mode &= 0x0000; //primeiros 3 bits a 1
   p_inode[p_offset].mode |= type;
   p_inode[p_offset].refcount = 0;
   p_inode[p_offset].owner = getuid();
   p_inode[p_offset].group = getgid();
   p_inode[p_offset].size = 0;
   p_inode[p_offset].clucount = 0;
   p_inode[p_offset].vD1.atime = p_inode[p_offset].vD2.mtime = time(NULL); //modif e last access time sao o mesmo
   p_inode[p_offset].i1 = NULL_CLUSTER;
   p_inode[p_offset].i2 = NULL_CLUSTER;

   int i;
   for(i = 0; i < N_DIRECT; i++)
      p_inode[p_offset].d[i] = NULL_CLUSTER;

   //guardar todas as alteraçoes

   if((error = soStoreBlockInT()) != 0)
      return error;

   if((error = soStoreSuperBlock()) != 0)
      return error;
   return 0;
}
