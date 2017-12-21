/**
 *  \file soAddAttDirEntry.c (implementation file)
 *
 *  \author Goncalo Grilo
 */

#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <errno.h>
#include <libgen.h>
#include <string.h>
#include <stdlib.h>


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

/* Allusion to external function */

int soGetDirEntryByName (uint32_t nInodeDir, const char *eName, uint32_t *p_nInodeEnt, uint32_t *p_idx);

/** \brief operation add a generic entry to a directory */
#define ADD         0
/** \brief operation attach an entry to a directory to a directory */
#define ATTACH      1

/**
 *  \brief Add a generic entry / attach an entry to a directory to a directory.
 *
 *  In the first case, a generic entry whose name is <tt>eName</tt> and whose inode number is <tt>nInodeEnt</tt> is added
 *  to the directory associated with the inode whose number is <tt>nInodeDir</tt>. Thus, both inodes must be in use and
 *  belong to a legal type, the former, and to the directory type, the latter.
 *
 *  Whenever the type of the inode associated to the entry to be added is of directory type, the directory is initialized
 *  by setting its contents to represent an empty directory.
 *
 *  In the second case, an entry to a directory whose name is <tt>eName</tt> and whose inode number is <tt>nInodeEnt</tt>
 *  is attached to the directory, the so called <em>base directory</em>, associated to the inode whose number is
 *  <tt>nInodeDir</tt>. The entry to be attached is supposed to represent itself a fully organized directory, the so
 *  called <em>subsidiary directory</em>. Thus, both inodes must be in use and belong to the directory type.
 *
 *  The <tt>eName</tt> must be a <em>base name</em> and not a <em>path</em>, that is, it can not contain the
 *  character '/'. Besides there should not already be any entry in the directory whose <em>name</em> field is
 *  <tt>eName</tt>.
 *
 *  The <em>refcount</em> field of the inode associated to the entry to be added / updated and, when required, of the
 *  inode associated to the directory are updated. This may also happen to the <em>size</em> field of either or both
 *  inodes.
 *
 *  The process that calls the operation must have write (w) and execution (x) permissions on the directory.
 *
 *  \param nInodeDir number of the inode associated to the directory
 *  \param eName pointer to the string holding the name of the entry to be added / attached
 *  \param nInodeEnt number of the inode associated to the entry to be added / attached
 *  \param op type of operation (ADD / ATTACH)
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if any of the <em>inode numbers</em> are out of range or the pointer to the string is \c NULL
 *                      or the name string does not describe a file name or no operation of the defined class is described
 *  \return -\c ENAMETOOLONG, if the name string exceeds the maximum allowed length
 *  \return -\c ENOTDIR, if the inode type whose number is <tt>nInodeDir</tt> (ADD), or both the inode types (ATTACH),
 *                       are not directories
 *  \return -\c EEXIST, if an entry with the <tt>eName</tt> already exists
 *  \return -\c EACCES, if the process that calls the operation has not execution permission on the directory where the
 *                      entry is to be added / attached
 *  \return -\c EPERM, if the process that calls the operation has not write permission on the directory where the entry
 *                     is to be added / attached
 *  \return -\c EMLINK, if the maximum number of hardlinks in either one of inodes has already been attained
 *  \return -\c EFBIG, if the directory where the entry is to be added / attached, has already grown to its maximum size
 *  \return -\c ENOSPC, if there are no free data clusters
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

int soAddAttDirEntry (uint32_t nInodeDir, const char *eName, uint32_t nInodeEnt, uint32_t op)
{
  soColorProbe (313, "07;31", "soAddAttDirEntry (%"PRIu32", \"%s\", %"PRIu32", %"PRIu32")\n", nInodeDir,
                eName, nInodeEnt, op);

  int error, i;
  uint32_t index; //entrada livre
  SODirEntry tmp[DPC];
  SOInode *p_dir; //para onde nInodeDir vai ser lido
  SOInode *p_ent; //para onde nInodeEnt vai ser lido

  if((error = soAccessGranted(nInodeDir, X)) != 0)  //permicoes de escrita e exe
  	return error; 
  if((error = soAccessGranted(nInodeDir, W)) != 0)
  	return error;

  p_dir = (SOInode*) malloc(sizeof(SOInode));
  if(p_dir == NULL)
    return -ELIBBAD;
  p_ent = (SOInode*) malloc(sizeof(SOInode));
  if(p_ent == NULL) 
  	return -ELIBBAD;

  if((error = soReadInode(p_dir, nInodeDir)) !=0)   //esta em uso e e um tipo legal
  	return error;
  if((error = soReadInode(p_ent, nInodeEnt)) != 0)  //esta em uso e e um tipo legal
  	return error;

  if((p_dir->mode & INODE_DIR) == 0) //nInodeDir nao e Dir
  	return -ENOTDIR;
  if(p_dir->size >= MAX_FILE_SIZE)  //tamanho maximo
  	return -EFBIG;
  if(p_dir->refcount >= 0xFFFF || p_ent->refcount >= 0xFFFF) //nº max de hardlinks atingido
  	return -EMLINK;

  if(eName==NULL)
  	return -EINVAL;
  if(strlen(eName) > MAX_NAME)
  	return ENAMETOOLONG;
  error = soGetDirEntryByName(nInodeDir, eName, NULL, &index);  //index vai ser entrada na tabela livre
  uint32_t clustInd = index/DPC; //index do cluster onde esta a entrada da tabela livre
  if(error == 0)  //nao da erro, ou seja existe
  	return -EEXIST;
  if(error != -ENOENT)  //ENOENT entry com este nome nao existe, mas da outro erro
  	return error;

  
  switch (op){
  	case ADD:
  		if(p_ent->mode & INODE_DIR){  //fazer add de uma Dir
  			SODirEntry vazia[DPC];
  			vazia[0].name[0] = '.'; //autoref
        for(i=1; i < MAX_NAME+1; i++){
          vazia[0].name[i] = '\0'; //fim da str
        }
  			vazia[0].nInode = nInodeEnt;

  			vazia[1].name[0] = '.'; //dir "pai"
  			vazia[1].name[1] = '.';
  			for(i=2; i < MAX_NAME+1; i++){
          vazia[1].name[i] = '\0'; //fim da str
        }
  			vazia[1].nInode = nInodeDir;
  			
        int j;
  			for(i = 2; i < DPC; i++){ //o resto das entradas a \0
  				for (j = 0; j < MAX_NAME+1; j++)
  					vazia[i].name[j] = '\0';
  				vazia[i].nInode = NULL_INODE;
  			}
  			p_dir->refcount++;    //ent tem uma ref para esta
  			p_ent->refcount += 2; //. e ref no dir
  			p_ent->size = CLUSTER_SIZE;

  			if((error = soWriteInode(p_ent,nInodeEnt)) != 0) //escrever tudo
  				return error;
        	if((error = soWriteInode(p_dir,nInodeDir)) != 0)
        		return error;
        	if((error = soWriteFileCluster(nInodeEnt,0,&vazia)) != 0)
        		return error;
  		}
  		else{  //ent nao e uma DIR
  			p_ent->refcount++; //vai ter uma ref no dir
  			if((error = soWriteInode(p_ent,nInodeEnt)) != 0)
  				return error;
  		}
  		if((error = soReadFileCluster(nInodeDir,clustInd,&tmp)) !=0 )
  			return error;

  		tmp[index].nInode = nInodeEnt;
  		memcpy(&(tmp[index].name), eName, strlen(eName));
  		for(i=strlen(eName); i < MAX_NAME+1; i++){
          tmp[index].name[i] = '\0'; //fim da str
        }
      //tmp[index].name[strlen(eName)] = '\0';
  		
  		if((error = soWriteFileCluster(nInodeDir,clustInd,&tmp)) != 0)
  			return error;
  		break;


  	case ATTACH:
  		if((p_ent->mode & INODE_DIR) == 0) //caso ent nao seja dir
  			return -ENOTDIR;
  		if((error = soReadFileCluster(nInodeDir,clustInd,&tmp)) != 0)
  			return error;

  		tmp[index].nInode = nInodeEnt;
  		memcpy(&(tmp[index].name), eName,strlen(eName));
  		tmp[index].name[strlen(eName)] = '\0';
      for(i=strlen(eName); i < MAX_NAME+1; i++){
          tmp[index].name[i] = '\0'; //fim da str
        }
  		p_ent->refcount++; //dir vai ter uma ref para este
  		if((error = soWriteFileCluster(nInodeDir,clustInd,&tmp)) != 0)
  			return error;

  		//ent
  		if((error = soGetDirEntryByName(nInodeEnt, "..", NULL, &index)) != 0) //tem de ser alterado para nInodeDir
  			return error;
  		clustInd = index/DPC;
    	if((error = soReadFileCluster(nInodeEnt,clustInd,&tmp)) != 0)
    		return error;
    	tmp[index].nInode = nInodeDir;
    	p_dir->refcount++; //ent tem a ref .. para dir

    	if((error = soWriteInode(p_dir,nInodeDir)) != 0)
    		return error;
    	if((error = soWriteInode(p_ent,nInodeEnt)) != 0)
    		return error;
    	if((error = soWriteFileCluster(nInodeEnt,clustInd,&tmp)) != 0)
    		return error;
  		break;


  	default:
  		return -EINVAL;
  }
  return 0;
}
