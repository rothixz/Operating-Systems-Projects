/**
 *  \file soReadInode.c (implementation file)
 *
 *  \author Hugo Fragata de Oliveira Torres 73875
 */

#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#include "sofs_probe.h"
#include "sofs_buffercache.h"
#include "sofs_superblock.h"
#include "sofs_inode.h"
#include "sofs_basicoper.h"
#include "sofs_basicconsist.h"

/**
 *  \brief Read specific inode data from the table of inodes.
 *
 *  The inode must be in use and belong to one of the legal file types.
 *  Upon reading, the <em>time of last file access</em> field is set to current time.
 *
 *  \param p_inode pointer to the buffer where inode data must be read into
 *  \param nInode number of the inode to be read from
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the <em>buffer pointer</em> is \c NULL or the <em>inode number</em> is out of range
 *  \return -\c EIUININVAL, if the inode in use is inconsistent
 *  \return -\c ELDCININVAL, if the list of data cluster references belonging to an inode is inconsistent
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

int soReadInode (SOInode *p_inode, uint32_t nInode)
{
  soColorProbe (511, "07;31", "soReadInode (%p, %"PRIu32")\n", p_inode, nInode);


  int error;
  SOSuperBlock* p_sb;

  //validacao do superbloco e ter ponteiro para tal
  if((error=soLoadSuperBlock())!=0) return error;
  p_sb = soGetSuperBlock();

  //validacao dos parametros de entrada
  if(p_inode == NULL) return -EINVAL;
  if(nInode>=p_sb->itotal) return -EINVAL;

  uint32_t NBLK, offset;
  SOInode *p_read;

  //carregar para p_read
  if( (error = soConvertRefInT(nInode, &NBLK, &offset)) != 0 ) return error;
  if( (error = soLoadBlockInT(NBLK)) != 0 ) return error;
  if( (p_read = soGetBlockInT()) == NULL ) return -EINVAL;
  p_read += offset;

  if( (error=soQCheckInodeIU(p_sb, p_read)) != 0) return error; //verificar q o no-i esta a ser usado

  //atualizar campo atime e copiar
  p_read->vD1.atime = time(NULL);
  memcpy(p_inode, p_read, sizeof(SOInode));

  //guardar
  if( (error = soStoreBlockInT()) != 0) return error;

  return 0;
}
