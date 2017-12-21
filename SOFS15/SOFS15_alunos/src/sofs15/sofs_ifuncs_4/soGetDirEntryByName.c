/**
 *  \file soGetDirEntryByName.c (implementation file)
 *
 *  \author ---
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

/**
 *  \brief Get an entry by name.
 *
 *  The directory contents, seen as an array of directory entries, is parsed to find an entry whose name is
 *  <tt>eName</tt>. Thus, the inode associated to the directory must be in use and belong to the directory type.
 *
 *  The <tt>eName</tt> must also be a <em>base name</em> and not a <em>path</em>, that is, it can not contain the
 *  character '/'.
 *
 *  The process that calls the operation must have execution (x) permission on the directory.
 *
 *  \param nInodeDir number of the inode associated to the directory
 *  \param eName pointer to the string holding the name of the directory entry to be located
 *  \param p_nInodeEnt pointer to the location where the number of the inode associated to the directory entry whose
 *                     name is passed, is to be stored
 *                     (nothing is stored if \c NULL)
 *  \param p_idx pointer to the location where the index to the directory entry whose name is passed, or the index of
 *               the first entry that is free, is to be stored
 *               (nothing is stored if \c NULL)
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the <em>inode number</em> is out of range or the pointer to the string is \c NULL or the
 *                      name string does not describe a file name
 *  \return -\c ENAMETOOLONG, if the name string exceeds the maximum allowed length
 *  \return -\c ENOTDIR, if the inode type is not a directory
 *  \return -\c ENOENT,  if no entry with <tt>name</tt> is found
 *  \return -\c EACCES, if the process that calls the operation has not execution permission on the directory
 *  \return -\c EDIRINVAL, if the directory is inconsistent
 *  \return -\c EDEINVAL, if the directory entry is inconsistent
 *  \return -\c EIUININVAL, if the inode in use is inconsistent
 *  \return -\c ELDCININVAL, if the list of data cluster references belonging to an inode is inconsistent
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

static int valid_name(const char *name);

int soGetDirEntryByName (uint32_t nInodeDir, const char *eName, uint32_t *p_nInodeEnt, uint32_t *p_idx)
{
  soColorProbe (312, "07;31", "soGetDirEntryByName (%"PRIu32", \"%s\", %p, %p)\n",
                nInodeDir, eName, p_nInodeEnt, p_idx);

  int error;                    // variavel de erro
  SOSuperBlock *p_sb;           // ponteiro para o superbloco  
  SOInode *p_Inode;             // ponteiro para o no-i
  SODataClust *p_dir;           // ponteiro para o cluster de dados      
  uint32_t tdir_index;          // indice da tabela de entrada de directorios da primeira entrada livre
  int tdir;                     // indice da tabela de entradas de directorios
  int flag = 0;                 // flag da primeira posição livre
  int trd=0;                    // idx da tabela de referencias directas
  int count=0;                  // contador de directorios percorridos
  int size = CLUSTER_SIZE/DPC;  // tamanho de uma entrada de directorio em bytes
  
  if((error=soLoadSuperBlock())!=0) return error;	// load sb
  if((p_sb=soGetSuperBlock())==NULL) return -ELIBBAD;	// ponteiro para o sb
       
  // validacao do no-i, ponteiro para o nome do directorio e validacao
  if((nInodeDir<0) || (nInodeDir>=p_sb->itotal) || (eName==NULL)) return -EINVAL; 
    
  // verifica se o nome do directorio nao excede o tamanho maximo
  if((sizeof(eName)/sizeof(char))>(MAX_NAME+1)) return -ENAMETOOLONG;
  // reservar memoria para salvaguardar o Inode que vai ser utilizado
  if((p_Inode = (SOInode*) malloc(sizeof(SOInode)))==NULL) return -ELIBBAD;
  // reservar memoria para salvaguardar o cluster de dados
  if((p_dir = (SODataClust*) malloc(sizeof(SODataClust)))==NULL) return -ELIBBAD;
  
  // guardar o Inode em memoria previamente alocada   
  if((error=soReadInode(p_Inode, nInodeDir)) != 0) return error; 
  if((error=soAccessGranted(nInodeDir, X)) != 0) return error; // verificar permissao de execucao
    
  if((p_Inode->mode & INODE_DIR)!=INODE_DIR) return -ENOTDIR;    // verificar se o no-i está associado a um directorio
 // if((error=soQCheckDirCont(p_sb, p_Inode))!=0) return error;    // verifica a consistencia do directorio

  while(count<=p_Inode->size){ 
    // guardar o cluster em memoria previamente alocada
    if((error=soReadFileCluster(nInodeDir, trd, p_dir)) != 0) return error;

    for(tdir=0; tdir<DPC; tdir++){                 
      if(p_dir->de[tdir].name[0]=='\0' && flag==0){
        tdir_index = (DPC*trd)+tdir;
        flag = 1;
      }
      else if(strcmp((const char*)p_dir->de[tdir].name,eName)==0){
        if(p_nInodeEnt!=NULL) *p_nInodeEnt = p_dir->de[tdir].nInode;
        if(p_idx!=NULL) *p_idx = (DPC*trd)+tdir;
        
        return 0;
      }
      count+=size;
    }

    trd++;
  }
  
  // se cluster cheio, a primeira entrada livre é a primeira posição do proximo
  if(flag==0) tdir_index = trd*DPC; 
  if(p_idx!=NULL) *p_idx = tdir_index;
  
  return -ENOENT;
}