/**
 *  \file soMkdir.c (implementation file)
 *
 *  \author ---
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <time.h>
#include <utime.h>
#include <libgen.h>
#include <string.h>

#include "sofs_probe.h"
#include "sofs_const.h"
#include "sofs_rawdisk.h"
#include "sofs_buffercache.h"
#include "sofs_superblock.h"
#include "sofs_inode.h"
#include "sofs_direntry.h"
#include "sofs_datacluster.h"
#include "sofs_basicoper.h"
#include "sofs_basicconsist.h"
#include "sofs_ifuncs_1.h"
#include "sofs_ifuncs_2.h"
#include "sofs_ifuncs_3.h"
#include "sofs_ifuncs_4.h"

/**
 *  \brief Create a directory.
 *
 *  It tries to emulate <em>mkdir</em> system call.
 *
 *  \param ePath path to the file
 *  \param mode type and permissions to be set:
 *                    a bitwise combination of S_ISVTX, S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH,
 *                    S_IWOTH, S_IXOTH
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the pointer to the string is \c NULL or or the path string is a \c NULL string or the path does
 *                      not describe an absolute path or no mode of the defined class is described
 *  \return -\c ENAMETOOLONG, if the path name or any of its components exceed the maximum allowed length
 *  \return -\c ENOTDIR, if any of the components of <tt>ePath</tt> is not a directory
 *  \return -\c ELOOP, if the path resolves to more than one symbolic link
 *  \return -\c ENOENT, if no entry with a name equal to any of the components of <tt>ePath</tt>, but the last one, is
 *                      found
 *  \return -\c EEXIST, if a file described by <tt>ePath</tt> already exists or the last component is a symbolic link
 *  \return -\c EACCES, if the process that calls the operation has not execution permission on any of the components
 *                      of <tt>ePath</tt>, but the last one
 *  \return -\c EPERM, if the process that calls the operation has not write permission on the directory that will hold
 *                     <tt>ePath</tt>
 *  \return -\c ENOSPC, if there are no free data clusters
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails on writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

int soMkdir (const char *ePath, mode_t mode)
{
 	soColorProbe (232, "07;31", "soMkdir (\"%s\", %u)\n", ePath, mode);

 	uint32_t InodeDir, Inode;			//numero do no-i do dir e da entry
	int stat;					//stat
	char path[MAX_PATH + 1];			//directory path
	char name[MAX_PATH + 1];			//name
	SOInode inode;					//stored inode
	
	if ( ePath == NULL )				//ePath tem de existir
		return -EINVAL;				//e ser maior q 0
							//e ser caminho absoluto

	if ( strlen(ePath) == 0 )						
		return -EINVAL;
	
	if ( strlen(ePath) > MAX_PATH ) 					
		return -ENAMETOOLONG;
	
	if ( ePath[0] != '/' )							
		return -EINVAL;
	
	if ( (mode & 0x01FF) == 0 )			//tem de haver mode			
		return -EINVAL;

	strcpy(path, ePath);							
	strcpy(name, ePath);							
	strcpy(name,basename(name));						
	strcpy(path,dirname(path));						

	
	if ((stat = soGetDirEntryByPath (path, NULL, &InodeDir)) != 0)		//apanhar o valor do no i pai
		return stat;

	if ((stat = soReadInode(&inode,InodeDir)) != 0)
		return stat;
	
	if ((inode.mode & INODE_TYPE_MASK) != INODE_DIR)			//tem de ser directorio
		return -ENOTDIR;


	if ((stat = soAccessGranted(InodeDir, W)) != 0)				//tem q se ter permissao para
		return -EPERM;							//escrever e executar

	if ((stat = soAccessGranted(InodeDir, X)) != 0)				
		return -EACCES;


	
	stat = soGetDirEntryByName (InodeDir, name, NULL, NULL);

	if ( stat == 0 )							//verifica se tal dir ja existe
		return -EEXIST;

	if ( stat != -ENOENT )
		return stat;


	//aloca no i
	if ((stat = soAllocInode(INODE_DIR, &Inode)) != 0)
		return stat;
	

	
	if ((stat = soReadInode(&inode,Inode)) != 0)
		return stat;
	
	//permissoes
	inode.mode |= mode;
	inode.owner = getuid();
	inode.group = getgid();

	if ((stat = soWriteInode(&inode,Inode)) != 0)
		return stat;


	//adicionar a dir entry
	if ((stat = soAddAttDirEntry(InodeDir, name, Inode, ADD)) != 0)
		return stat;

	return 0;
}
