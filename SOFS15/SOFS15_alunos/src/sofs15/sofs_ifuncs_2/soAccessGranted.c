/**
 *  \file soAccessGranted.c (implementation file)
 *
 *  \author Nuno Silva [72708]
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
#include "sofs_basicoper.h"
#include "sofs_basicconsist.h"

/* Allusion to internal functions */

int soReadInode (SOInode *p_inode, uint32_t nInode);

/** \brief performing a read operation */
#define R  0x0004
/** \brief performing a write operation */
#define W  0x0002
/** \brief performing an execute operation */
#define X  0x0001

/**
 *  \brief Check the inode access rights against a given operation.
 *
 *  The inode must to be in use and belong to one of the legal file types.
 *  It checks if the inode mask permissions allow a given operation to be performed.
 *
 *  When the calling process is <em>root</em>, access to reading and/or writing is always allowed and access to
 *  execution is allowed provided that either <em>user</em>, <em>group</em> or <em>other</em> have got execution
 *  permission.
 *
 *  \param nInode number of the inode
 *  \param opRequested operation to be performed:
 *                    a bitwise combination of R, W, and X
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the <em>inode number</em> is out of range or no operation of the defined class is described
 *  \return -\c EACCES, if the operation is denied
 *  \return -\c EIUININVAL, if the inode in use is inconsistent
 *  \return -\c ELDCININVAL, if the list of data cluster references belonging to an inode is inconsistent
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

int soAccessGranted (uint32_t nInode, uint32_t opRequested)
{
  soColorProbe (513, "07;31", "soAccessGranted (%"PRIu32", %"PRIu32")\n", nInode, opRequested);

  SOSuperBlock* p_sb;	// Ponteiro para superblock
  SOInode inode;		// Inode
  int status;			// variável para retorno de erros

  if(((opRequested|(R|W|X))!= (R|W|X)) || (opRequested==0)) //verificar se operaçao é válida
		return -EINVAL;

	if((status=soLoadSuperBlock())!=0) return status;
  if((p_sb=soGetSuperBlock())== NULL) return -EIO;  
  if(nInode > p_sb->itotal-1) return -EINVAL; // checking if inode number is out of range

  if((status = soReadInode(&inode, nInode)) != 0) return status; // leitura do inode

  // caso usrid = root //

  if((getuid() == 0) && (getgid() == 0))
	{ 
		if((opRequested == R) || opRequested == W ) /* R e W sempre permitidos */
			return 0;  
			
		else {	// X se em own, group ou other houver um x
			if((inode.mode & ( X << 6 | X << 3 | X ) ) == ( X << 6 | X << 3 | X ))
				return 0;
				
			else return -EACCES;			
		}

	}

	// caso usrid != root

  if(inode.owner == getuid())
	{
		/*  usrid = own -> R, W ou X são permitidos se em own houver, respectivamente, um r, um w ou um x */
		if((inode.mode & opRequested << 6) != opRequested << 6)
			return -EACCES;
	}
	else
	{/* gid = group -> R, W ou X são permitidos se em group houver, respectivamente, um r, um w ou um x */
		if(inode.group == getgid())
		{
			if((inode.mode & ( opRequested << 3) ) != ( opRequested << 3 ) )
				return -EACCES; 
		}
		else 
		{/* usrid != own && gid != group -> R, W ou X são permitidos se em other houver, respectivamente, um r, um w ou um x */
			if((inode.mode & ( opRequested ) ) != ( opRequested ) )
				return -EACCES;
		}
	}

	return 0;
}
