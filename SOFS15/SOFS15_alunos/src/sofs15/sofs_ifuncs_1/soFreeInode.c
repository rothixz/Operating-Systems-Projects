/**
 *  \file soFreeInode.c (implementation file)
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
 *  \brief Free the referenced inode.
 *
 *  The inode must be in use, belong to one of the legal file types and have no directory entries associated with it
 *  (refcount = 0). The inode is marked free and inserted in the list of free inodes.
 *
 *  Notice that the inode 0, supposed to belong to the file system root directory, can not be freed.
 *
 *  The only affected fields are:
 *     \li the free flag of mode field, which is set
 *     \li the <em>time of last file modification</em> and <em>time of last file access</em> fields, which change their
 *         meaning: they are replaced by the <em>prev</em> and <em>next</em> pointers in the double-linked list of free
 *         inodes.
 * *
 *  \param nInode number of the inode to be freed
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the <em>inode number</em> is out of range
 *  \return -\c EIUININVAL, if the inode in use is inconsistent
 *  \return -\c ELDCININVAL, if the list of data cluster references belonging to an inode is inconsistent
 *  \return -\c ESBTINPINVAL, if the table of inodes metadata in the superblock is inconsistent
 *  \return -\c ETINDLLINVAL, if the double-linked list of free inodes is inconsistent
 *  \return -\c EFININVAL, if a free inode is inconsistent
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 *  author : Nuno Silva 72708
 */

int soFreeInode (uint32_t nInode)
{
	soColorProbe (612, "07;31", "soFreeInode (%"PRIu32")\n", nInode);

	SOSuperBlock* p_sb;

	int err;
    
    if((err=soLoadSuperBlock())!=0){ return err; }
    
    if((p_sb=soGetSuperBlock())==NULL){ return -EIO; }
    
    if((nInode <1) || (nInode >= p_sb->itotal)){ return -EINVAL; }
    
    uint32_t nBlk;         // local onde está o numero logico do bloco que contem o nó-I 
    uint32_t offset;       // local onde está a posição do nó-I dentro do bloco
    
    SOInode* p_Inode;         // ponteiro para o bloco de nós-I
    
    if((err=soConvertRefInT(nInode, &nBlk, &offset)) != 0 ){ return err; } // obter a posição do no-i a libertar
    
    if((err=soLoadBlockInT(nBlk)) != 0 ){ return err; }   // carregar em memória o bloco que contem o no-i a libertar
    
    if((p_Inode = soGetBlockInT()) == NULL ){ return -EIO; }

    if(p_Inode[offset].refcount !=0){ return -EINVAL; } // caso o refcount seja diferente de 0
   
    if(p_Inode[offset].mode==INODE_FREE){ return -EIUININVAL; }  // caso esteja livre
    
    if((err=soQCheckInodeIU(p_sb, &p_Inode[offset])) != 0){ return err; }

    p_Inode[offset].mode = p_Inode[offset].mode | INODE_FREE;
    if((err=soStoreBlockInT()) != 0){ return err; }

	if(nInode==0){ return -EINVAL; }

	if (p_sb->ihdtl == NULL_INODE)
	{ /* a lista está vazia */
		if((err=soConvertRefInT(nInode, &nBlk, &offset)) != 0 ){ return err; } 
   	    if((err=soLoadBlockInT(nBlk)) != 0 ){ return err; } 
        if((p_Inode = soGetBlockInT()) == NULL ){ return -EIO; }
		p_Inode[offset].vD1.prev = p_Inode[offset].vD2.next = nInode; 
		p_sb->ihdtl = nInode;
		if((err = soStoreBlockInT()) != 0){ return err; }
	}

	else { 

		if((err=soConvertRefInT(p_sb->ihdtl, &nBlk, &offset)) != 0 ){ return err; } 
   	    if((err=soLoadBlockInT(nBlk)) != 0 ){ return err; } 
        if((p_Inode = soGetBlockInT()) == NULL ){ return -EIO; }

		/* a lista tem um elemento */
		if (p_Inode[offset].vD1.prev == p_sb->ihdtl){ 

			p_Inode[offset].vD1.prev = p_Inode[offset].vD2.next = nInode; //p_Inode[ihdtl] ja esta carregado
			if((err = soStoreBlockInT()) != 0){ return err; }

			if((err=soConvertRefInT(nInode, &nBlk, &offset)) != 0 ){ return err; }  //p_Inode[nInode]
   	        if((err=soLoadBlockInT(nBlk)) != 0 ){ return err; } 
            if((p_Inode = soGetBlockInT()) == NULL ){ return -EIO; }
			p_Inode[offset].vD1.prev = p_Inode[offset].vD2.next = p_sb->ihdtl;
			if((err = soStoreBlockInT()) != 0){ return err; }
		}

		/* a lista tem dois ou mais elementos */
		else { 
            //p_inode[ihdtl] ja esta carregado

			uint32_t prev;
			prev = p_Inode[offset].vD1.prev;
			p_Inode[offset].vD1.prev = nInode;
			if((err=soStoreBlockInT()) != 0){ return err; }   // guardar a informação do ihdtl

            // carregar o ultimo elemento da lista 
            if((err=soConvertRefInT(prev, &nBlk, &offset)) != 0 ){ return err; } // obter a posição do ultimo elemento da lista
            if((err=soLoadBlockInT(nBlk)) != 0 ){ return err; }   // carregar em memória o bloco que contem o ultimo elemento da lista
            if((p_Inode = soGetBlockInT()) == NULL ){ return -EIO; }

			p_Inode[offset].vD2.next= nInode;
			if((err=soStoreBlockInT()) != 0){ return err; }   // guardar a informação do ultimo elemento da lista
            // carregar o no-i a libertar

            if((err=soConvertRefInT(nInode, &nBlk, &offset)) != 0 ){ return err; } // obter a posição do no-i a libertar
            if((err=soLoadBlockInT(nBlk)) != 0 ){ return err; }   // carregar em memória o bloco que contem o no-i a libertar    
            if((p_Inode = soGetBlockInT()) == NULL ){ return -EIO; }

			p_Inode[offset].vD1.prev = prev;
			p_Inode[offset].vD2.next = p_sb->ihdtl;

			if((err=soStoreBlockInT()) != 0){ return err; }   // guardar a informação do no-i a libertar
            
		}
	}
	p_sb->ifree += 1;

    if((err=soStoreSuperBlock()) != 0){ return err; }


	return 0;
}
