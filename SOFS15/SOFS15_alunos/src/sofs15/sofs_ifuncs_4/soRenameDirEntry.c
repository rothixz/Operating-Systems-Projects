/**
 *  \file soRenameDirEntry.c (implementation file)
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

/**
 *  \brief Rename an entry of a directory.
 *
 *  The directory entry whose name is <tt>oldName</tt> has its <em>name</em> field changed to <tt>newName</tt>. Thus,
 *  the inode associated to the directory must be in use and belong to the directory type.
 *
 *  Both the <tt>oldName</tt> and the <tt>newName</tt> must be <em>base names</em> and not <em>paths</em>, that is,
 *  they can not contain the character '/'. Besides an entry whose <em>name</em> field is <tt>oldName</tt> should exist
 *  in the directory and there should not be any entry in the directory whose <em>name</em> field is <tt>newName</tt>.
 *
 *  The process that calls the operation must have write (w) and execution (x) permissions on the directory.
 *
 *  \param nInodeDir number of the inode associated to the directory
 *  \param oldName pointer to the string holding the name of the direntry to be renamed
 *  \param newName pointer to the string holding the new name
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the <em>inode number</em> is out of range or either one of the pointers to the strings are
 *                      \c NULL or the name strings do not describe file names
 *  \return -\c ENAMETOOLONG, if one of the name strings exceeds the maximum allowed length
 *  \return -\c ENOTDIR, if the inode type is not a directory
 *  \return -\c ENOENT,  if no entry with <tt>oldName</tt> is found
 *  \return -\c EEXIST,  if an entry with the <tt>newName</tt> already exists
 *  \return -\c EACCES, if the process that calls the operation has not execution permission on the directory
 *  \return -\c EPERM, if the process that calls the operation has not write permission on the directory
 *  \return -\c EDIRINVAL, if the directory is inconsistent
 *  \return -\c EDEINVAL, if the directory entry is inconsistent
 *  \return -\c EIUININVAL, if the inode in use is inconsistent
 *  \return -\c ELDCININVAL, if the list of data cluster references belonging to an inode is inconsistent
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

int soRenameDirEntry (uint32_t nInodeDir, const char *oldName, const char *newName)
{
	soColorProbe (315, "07;31", "soRenameDirEntry (%"PRIu32", \"%s\", \"%s\")\n", nInodeDir, oldName, newName);

	/*---------------------	Variables	---------------------*/

	SOInode Inode;
	SODirEntry entrys[DPC];
	uint32_t index;
	int error_status;

	/*---------------------	Validation	---------------------*/

	if ( oldName == NULL || newName == NULL ) { return -EINVAL; }
	if ( strlen( oldName ) > MAX_NAME || strlen( newName ) > MAX_NAME ) { return -ENAMETOOLONG; }
	if ( strcmp( oldName, "." ) == 0 || strcmp( oldName, ".." ) == 0 || strcmp( newName, "." ) == 0 || strcmp( newName, ".." ) == 0 ) { return -EINVAL; }
	if ( strchr( oldName, '/' ) != NULL || strchr( newName, '/' ) ) { return -EINVAL; }

	if ( (error_status = soReadInode(&Inode, nInodeDir)) != 0 ) { return error_status; }
	if ( Inode.mode && INODE_DIR == 0 ) { return -ENOTDIR; }
	if ( Inode.clucount == 0 ) { return -ELIBBAD; }
	if ( soAccessGranted( nInodeDir, X) ) { return -EACCES; }
	if ( soAccessGranted( nInodeDir, W) ) { return -EPERM; }

	error_status = soGetDirEntryByName(nInodeDir, oldName, NULL, &index);
	if ( error_status != 0 ) { return error_status; }
	error_status = soGetDirEntryByName(nInodeDir, newName, NULL, NULL);
	if ( error_status == 0 ) { return -EEXIST; }
	if ( error_status != -ENOENT ) { return error_status; }

	/*---------------------	Code		---------------------*/

	if ( (error_status = soReadFileCluster(nInodeDir, (index/DPC), entrys)) != 0 ) { return error_status; }

	strcpy( (char *) entrys[index].name, newName );
	memset( &entrys[index].name[strlen(newName)], '\0', MAX_NAME - strlen(newName) - 1 );

	if ( (error_status = soWriteFileCluster(nInodeDir, (index/DPC), entrys)) != 0) { return error_status; }
	return 0;
}
