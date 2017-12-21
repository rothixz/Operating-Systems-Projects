/**
 *  \file soGetDirEntryByPath.c (implementation file)
 *
 *  \author Nuno Silva [72708]
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

/* Allusion to external function */

int soGetDirEntryByName (uint32_t nInodeDir, const char *eName, uint32_t *p_nInodeEnt, uint32_t *p_idx);

/* Allusion to internal function */

static int soTraversePath (const char *ePath, uint32_t *p_nInodeDir, uint32_t *p_nInodeEnt);

/** \brief Number of symbolic links in the path */

static uint32_t nSymLinks = 0;

/** \brief Old directory inode number */

static uint32_t oldNInodeDir = 0;

/**
 *  \brief Get an entry by path.
 *
 *  The directory hierarchy of the file system is traversed to find an entry whose name is the rightmost component of
 *  <tt>ePath</tt>. The path is supposed to be absolute and each component of <tt>ePath</tt>, with the exception of the
 *  rightmost one, should be a directory name or symbolic link name to a path.
 *
 *  The process that calls the operation must have execution (x) permission on all the components of the path with
 *  exception of the rightmost one.
 *
 *  \param ePath pointer to the string holding the name of the path
 *  \param p_nInodeDir pointer to the location where the number of the inode associated to the directory that holds the
 *                     entry is to be stored
 *                     (nothing is stored if \c NULL)
 *  \param p_nInodeEnt pointer to the location where the number of the inode associated to the entry is to be stored
 *                     (nothing is stored if \c NULL)
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the pointer to the string is \c NULL
 *  \return -\c ENAMETOOLONG, if the path or any of the path components exceed the maximum allowed length
 *  \return -\c ERELPATH, if the path is relative and it is not a symbolic link
 *  \return -\c ENOTDIR, if any of the components of <tt>ePath</tt>, but the last one, is not a directory
 *  \return -\c ELOOP, if the path resolves to more than one symbolic link
 *  \return -\c ENOENT,  if no entry with a name equal to any of the components of <tt>ePath</tt> is found
 *  \return -\c EACCES, if the process that calls the operation has not execution permission on any of the components
 *                      of <tt>ePath</tt>, but the last one
 *  \return -\c EDIRINVAL, if the directory is inconsistent
 *  \return -\c EDEINVAL, if the directory entry is inconsistent
 *  \return -\c EIUININVAL, if the inode in use is inconsistent
 *  \return -\c ELDCININVAL, if the list of data cluster references belonging to an inode is inconsistent
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

int soGetDirEntryByPath (const char *ePath, uint32_t *p_nInodeDir, uint32_t *p_nInodeEnt)
{
  soColorProbe (311, "07;31", "soGetDirEntryByPath (\"%s\", %p, %p)\n", ePath, p_nInodeDir, p_nInodeDir);

  int status;  //variável para retornar código de erro
  uint32_t nInodeDir;   //numero do i-node do diretório pai
  uint32_t nInodeEnt;	//numero do i-node da entrada de diretorio


  //validação dos argumentos
  if (ePath == NULL) return -EINVAL;					//verificar se ePath é NULL
  if (strlen(ePath) == 0) return -EINVAL;				//verificar se a string tem tamanho 0
  if (strlen(ePath) > MAX_PATH) return -ENAMETOOLONG;	//verificar se a string nao excede o MAX_PATH de comprimento
  if (ePath[0] != '/') return -ERELPATH;				//verificar se é um path absoluto
  
  //inicio
  if ((status = soTraversePath(ePath,&nInodeDir, &nInodeEnt)) != 0) return status;

  if ( p_nInodeDir != NULL)  *p_nInodeDir = nInodeDir;	//numero do inode para o diretorio pai
  if ( p_nInodeEnt != NULL)  *p_nInodeEnt = nInodeEnt;	//numero para o inode da entrada

  return 0;
}

/**
 *  \brief Traverse the path.
 *
 *  \param ePath pointer to the string holding the name of the path
 *  \param p_nInodeDir pointer to the location where the number of the inode associated to the directory that holds the
 *                     entry is to be stored
 *  \param p_nInodeEnt pointer to the location where the number of the inode associated to the entry is to be stored
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c ENAMETOOLONG, if any of the path components exceed the maximum allowed length
 *  \return -\c ERELPATH, if the path is relative and it is not a symbolic link
 *  \return -\c ENOTDIR, if any of the components of <tt>ePath</tt>, but the last one, is not a directory
 *  \return -\c ELOOP, if the path resolves to more than one symbolic link
 *  \return -\c ENOENT,  if no entry with a name equal to any of the components of <tt>ePath</tt> is found
 *  \return -\c EACCES, if the process that calls the operation has not execution permission on any of the components
 *                      of <tt>ePath</tt>, but the last one
 *  \return -\c EDIRINVAL, if the directory is inconsistent
 *  \return -\c EDEINVAL, if the directory entry is inconsistent
 *  \return -\c EIUININVAL, if the inode in use is inconsistent
 *  \return -\c ELDCININVAL, if the list of data cluster references belonging to an inode is inconsistent
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

static int soTraversePath (const char *ePath, uint32_t *p_nInodeDir, uint32_t *p_nInodeEnt)
{

	char path [MAX_PATH + 1];	// array de chars que contem dirname do ePath
	char name [MAX_PATH + 1];	// array de chars que contem basename do ePath
	char data [BSLPC];			// array de chars que contem informacao do atalho
	int status;					// variavel de retorno de erro.
	SOInode inode;				// Inode para verificar se é atalho
	uint32_t nInodeDir;			// numero do inode do diretorio pai
	uint32_t nInodeEnt;			// numero do inode da entrada
	
	// copiar 'ePath' para o 'name' e 'path'
	strcpy( name, ePath );
	strcpy( path, ePath );
	// executa o basename e o dirname no name e no path respectivamente
	strcpy(name, basename(name) );
	strcpy(path, dirname(path) );

	//situacao de paragem: se o root retornar o inode correspondente: inode(0)
	if (strcmp(path,"/") == 0){
		if (strcmp(name,"/") == 0)		// se entrada == '/', mudar para '.'
			strcpy(name,".");		
		
		nInodeDir = 0;	// Inode da root
	
		// chamar funçao para retornar inode da entrada (basename)
		if ((status = soGetDirEntryByName(nInodeDir,name,&nInodeEnt,NULL)) != 0) return status;	
	}
	else{	//caso normal da iteraçao recursiva	
		if ((status = soTraversePath(path,&nInodeDir, &nInodeEnt)) != 0) return status;	
		// the present directory gonna be anterior for the GetByName
		// ex: /a/b --> /a   -> no_dir = 0, no_ent = 1
		//		/a/b -> no_dir = 1, no_ent = 2
		nInodeDir = nInodeEnt;
		// Call function to return no-i of entry (basename)
		// ex: path = "\b" --> *p_nInodeEnt = no-i that represent 'b'
		if ((status = soGetDirEntryByName(nInodeDir,name,&nInodeEnt,NULL)) != 0) return status; 
	}
	
	////////////////ATALHOS//////////////////////
	// verificar se inode contem atalho (symlink)
	if ((status = soReadInode(&inode,nInodeEnt)) != 0) return status;	

	//se não for atalho, terminar.
	if ((inode.mode & INODE_SYMLINK) != (INODE_SYMLINK)){

		if ( p_nInodeDir != NULL)  *p_nInodeDir = nInodeDir;	//numero do inode para o diretorio pai
		if ( p_nInodeEnt != NULL)  *p_nInodeEnt = nInodeEnt;	//numero para o inode da entrada

		return 0;
	}
	else{	// verifica se o atalho encontrado não é o primeiro
		
		if (nSymLinks>=1){
			nSymLinks=0;
			return -ELOOP;
		}

		// ler conteudo do cluster de dados que pertence ao atalho
		if ((status = soReadFileCluster(nInodeEnt, 0, data)) != 0) return status; 
	
		if (data[0] != '/'){		// se não for um path absoluto

			//verfifica se o ultimo caracter do path é '/'. Senão coloca-o.	
			if (path[strlen(path)-1] != '/')	
				strncat(path,"/",MAX_PATH+1);	

			strncat(path,data,MAX_PATH+1);	// juncao do path relativo (atalho) com o path dado (absoluto, diretorio pai)	
		}
		else
			strncpy(path,data,MAX_PATH+1);	// se for path absoluto, o novo path é o nome do atalho
	
		nSymLinks = 1;
		// recursividade com o novo path do atalho
		if ((status = soTraversePath(path, &nInodeDir, &nInodeEnt)) != 0) return status;	
	
		// fim da recursividade, reset do counter de atalhos.
		nSymLinks = 0;
	}

	if ( p_nInodeDir != NULL)  *p_nInodeDir = nInodeDir;	//numero do inode para o diretorio pai
	if ( p_nInodeEnt != NULL)  *p_nInodeEnt = nInodeEnt;	//numero para o inode da entrada

	return 0;
}
