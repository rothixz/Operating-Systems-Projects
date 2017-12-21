/**
 *  \file soAddAttDirEntry.c (implementation file)
 *
 *  \author ---
 */

#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <errno.h>
#include <libgen.h>
#include <string.h>

#include "sofs_probe.h"
#include "sofs_buffercache.h"
#include "sofs_superblock.h"
#include "sofs_inode.h"
#include "sofs_direntry.h"
#include "sofs_basicoper.h"
#include "sofs_basicconsist.h"
#include "sofs_ifuncs_1.h"
#include "sofs_ifuncs_2.h"
#include "sofs_ifuncs_3.h"

/* Allusion to external functions */

int soGetDirEntryByName (uint32_t nInodeDir, const char *eName, uint32_t *p_nInodeEnt, uint32_t *p_idx);
int soCheckDirectoryEmptiness (uint32_t nInodeDir);

/** \brief operation remove a generic entry from a directory */
#define REM         0
/** \brief operation detach a generic entry from a directory */
#define DETACH      1

/**
 *  \brief Remove / detach a generic entry from a directory.
 *
 *  The entry whose name is <tt>eName</tt> is removed / detached from the directory associated with the inode whose
 *  number is <tt>nInodeDir</tt>. Thus, the inode must be in use and belong to the directory type.
 *
 *  Removal of a directory entry means exchanging the first and the last characters of the field <em>name</em>.
 *  Detachment of a directory entry means filling all the characters of the field <em>name</em with the \c NULL
 *  character.
 *
 *  The <tt>eName</tt> must be a <em>base name</em> and not a <em>path</em>, that is, it can not contain the
 *  character '/'. Besides there should exist an entry in the directory whose <em>name</em> field is <tt>eName</tt>.
 *
 *  Whenever the operation is removal and the type of the inode associated to the entry to be removed is of directory
 *  type, the operation can only be carried out if the directory is empty.
 *
 *  The <em>refcount</em> field of the inode associated to the entry to be removed / detached and, when required, of
 *  the inode associated to the directory are updated.
 *
 *  The file described by the inode associated to the entry to be removed / detached is only deleted from the file
 *  system if the <em>refcount</em> field becomes zero (there are no more hard links associated to it). In this case,
 *  the data clusters that store the file contents and the inode itself must be freed.
 *
 *  The process that calls the operation must have write (w) and execution (x) permissions on the directory.
 *
 *  \param nInodeDir number of the inode associated to the directory
 *  \param eName pointer to the string holding the name of the directory entry to be removed / detached
 *  \param op type of operation (REM / DETACH)
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the <em>inode number</em> is out of range or the pointer to the string is \c NULL or the
 *                      name string does not describe a file name or no operation of the defined class is described
 *  \return -\c ENAMETOOLONG, if the name string exceeds the maximum allowed length
 *  \return -\c ENOTDIR, if the inode type whose number is <tt>nInodeDir</tt> is not a directory
 *  \return -\c ENOENT,  if no entry with <tt>eName</tt> is found
 *  \return -\c EACCES, if the process that calls the operation has not execution permission on the directory
 *  \return -\c EPERM, if the process that calls the operation has not write permission on the directory
 *  \return -\c ENOTEMPTY, if the entry with <tt>eName</tt> describes a non-empty directory
 *  \return -\c EDIRINVAL, if the directory is inconsistent
 *  \return -\c EDEINVAL, if the directory entry is inconsistent
 *  \return -\c EIUININVAL, if the inode in use is inconsistent
 *  \return -\c ELDCININVAL, if the list of data cluster references belonging to an inode is inconsistent
 *  \return -\c EDCINVAL, if the data cluster header is inconsistent
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

int soRemDetachDirEntry (uint32_t nInodeDir, const char *eName, uint32_t op)
{
	soColorProbe (314, "07;31", "soRemDetachDirEntry (%"PRIu32", \"%s\", %"PRIu32")\n", nInodeDir, eName, op);

	/*---------------------	Variables	---------------------*/

	uint32_t nInodeEntry;
	uint32_t offset;
	uint32_t index;
	int j;
	SOInode inodeDir, inodeEntry;
	SODirEntry entrys[DPC];
	SOSuperBlock *p_sb;
	int error_status;

	/*---------------------	Validation	---------------------*/

	if ( op != 0 && op != 1 ) { return -EINVAL; }

	if ( eName == NULL ) { return -EINVAL; }
	if ( strlen(eName) > MAX_NAME ) { return -ENAMETOOLONG; }
	if ( strchr(eName, '/') != NULL ) { return -EINVAL; }

	if ( (error_status = soReadInode(&inodeDir, nInodeDir)) != 0 ) { return error_status; }
	if ( (inodeDir.mode & INODE_DIR) != INODE_DIR ) { return -ENOTDIR; }

	if ( (error_status = soLoadSuperBlock()) != 0 ) { return error_status; }
	p_sb = soGetSuperBlock();
	if ( (error_status = soQCheckDirCont( p_sb, &inodeDir)) != 0 ) { return error_status; }

	if ( (error_status = soAccessGranted(nInodeDir, X)) != 0 ) { return -EACCES; }
	if ( (error_status = soAccessGranted(nInodeDir, W)) != 0 ) { return -EPERM; }

	if ( (error_status = soGetDirEntryByName(nInodeDir, eName, &nInodeEntry, &index)) != 0 ) { return error_status; }

	/*---------------------	Code		---------------------*/

	if ( (error_status = soReadInode(&inodeEntry, nInodeEntry)) != 0 ) { return error_status; }
	if ( (error_status = soReadFileCluster(nInodeDir, index/DPC, &entrys)) != 0 ) { return error_status; }

	offset = index % DPC;
	switch (op)
	{
		case REM:
			if ( (inodeEntry.mode & INODE_DIR) == INODE_DIR )
				if ( (error_status = soCheckDirectoryEmptiness(nInodeEntry)) != 0 ) { return error_status; }
			entrys[offset].name[MAX_NAME] = entrys[offset].name[0];
			entrys[offset].name[0] = '\0';
			break;
		case DETACH:
			for (j=0;j<MAX_NAME+1;j++){
				entrys[offset].name[j] = '\0';
			}
			entrys[offset].nInode = NULL_INODE;
			break;
	}
	if ( (error_status = soWriteFileCluster(nInodeDir, index/DPC, &entrys)) != 0 ) { return error_status; }

	/* INode count actualisation, if refcount == 0 then clears the cluster */

	if ( (inodeEntry.mode & INODE_DIR) == INODE_DIR ) {
		inodeEntry.refcount -= 2;
	 	inodeDir.refcount--;
	 	if ( (error_status = soWriteInode(&inodeDir, nInodeDir)) != 0 ) { return error_status; }
	}
	else {
		inodeEntry.refcount--;

	}
	if ( (error_status = soWriteInode(&inodeEntry, nInodeEntry)) != 0 ) { return error_status; }

	if ( (inodeEntry.refcount == 0 && op != DETACH) )
	{
		if ( (error_status = soHandleFileClusters(nInodeEntry, 0)) != 0 ) { return error_status; }
		if ( (error_status = soFreeInode(nInodeEntry)) != 0 ) { return error_status; }
	}

	return 0;
}
