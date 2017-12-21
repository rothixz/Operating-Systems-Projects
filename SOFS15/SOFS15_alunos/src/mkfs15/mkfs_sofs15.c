/**
 *  \file mkfs_sofs15.c (implementation file)
 *
 *  \brief The SOFS15 formatting tool.
 *
 *  It stores in predefined blocks of the storage device the file system metadata. With it, the storage device may be
 *  envisaged operationally as an implementation of SOFS15.
 *
 *  The following data structures are created and initialized:
 *     \li the superblock
 *     \li the table of inodes
 *     \li the data zone
 *     \li the contents of the root directory seen as empty.
 *
 *  SINOPSIS:
 *  <P><PRE>                mkfs_sofs15 [OPTIONS] supp-file
 *
 *                OPTIONS:
 *                 -n name --- set volume name (default: "SOFS15")
 *                 -i num  --- set number of inodes (default: N/8, where N = number of blocks)
 *                 -z      --- set zero mode (default: not zero)
 *                 -q      --- set quiet mode (default: not quiet)
 *                 -h      --- print this help.</PRE>
 *
 *  \author Artur Carneiro Pereira - September 2008
 *  \author Miguel Oliveira e Silva - September 2009
 *  \author António Rui Borges - September 2010 - August 2015
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "sofs_const.h"
#include "sofs_buffercache.h"
#include "sofs_superblock.h"
#include "sofs_inode.h"
#include "sofs_direntry.h"
#include "sofs_basicoper.h"
#include "sofs_basicconsist.h"

/* Allusion to internal functions */

static int fillInSuperBlock (SOSuperBlock *p_sb, uint32_t ntotal, uint32_t itotal, uint32_t fcblktotal,
		                     uint32_t nclusttotal, unsigned char *name);
static int fillInINT (SOSuperBlock *p_sb);
static int fillInRootDir (SOSuperBlock *p_sb);
static int fillInTRefFDC (SOSuperBlock *p_sb, int zero);
static int checkFSConsist (void);
static void printUsage (char *cmd_name);
static void printError (int errcode, char *cmd_name);

/* The main function */

int main (int argc, char *argv[])
{
  char *name = "SOFS15";                         /* volume name */
  uint32_t itotal = 0;                           /* total number of inodes, if kept, set value automatically */
  int quiet = 0;                                 /* quiet mode, if kept, set not quiet mode */
  int zero = 0;                                  /* zero mode, if kept, set not zero mode */

  /* process command line options */

  int opt;                                       /* selected option */

  do
  { switch ((opt = getopt (argc, argv, "n:i:qzh")))
    { case 'n': /* volume name */
                name = optarg;
                break;
      case 'i': /* total number of inodes */
                if (atoi (optarg) < 0)
                   { fprintf (stderr, "%s: Negative inodes number.\n", basename (argv[0]));
                     printUsage (basename (argv[0]));
                     return EXIT_FAILURE;
                   }
                itotal = (uint32_t) atoi (optarg);
                break;
      case 'q': /* quiet mode */
                quiet = 1;                       /* set quiet mode for processing: no messages are issued */
                break;
      case 'z': /* zero mode */
                zero = 1;                        /* set zero mode for processing: the information content of all free
                                                    data clusters are set to zero */
                break;
      case 'h': /* help mode */
                printUsage (basename (argv[0]));
                return EXIT_SUCCESS;
      case -1:  break;
      default:  fprintf (stderr, "%s: Wrong option.\n", basename (argv[0]));
                printUsage (basename (argv[0]));
                return EXIT_FAILURE;
    }
  } while (opt != -1);
  if ((argc - optind) != 1)                      /* check existence of mandatory argument: storage device name */
     { fprintf (stderr, "%s: Wrong number of mandatory arguments.\n", basename (argv[0]));
       printUsage (basename (argv[0]));
       return EXIT_FAILURE;
     }

  /* check for storage device conformity */

  char *devname;                                 /* path to the storage device in the Linux file system */
  struct stat st;                                /* file attributes */

  devname = argv[optind];
  if (stat (devname, &st) == -1)                 /* get file attributes */
     { printError (-errno, basename (argv[0]));
       return EXIT_FAILURE;
     }
  if (st.st_size % BLOCK_SIZE != 0)              /* check file size: the storage device must have a size in bytes
                                                    multiple of block size */
     { fprintf (stderr, "%s: Bad size of support file.\n", basename (argv[0]));
       return EXIT_FAILURE;
     }


  /* evaluating the file system architecture parameters
   * full occupation of the storage device when seen as an array of blocks supposes that the equation bellow
   *
   *    NTBlk = 1 + sige(NTClt/RPB) + NBlkTIN + NTClt*BLOCKS_PER_CLUSTER
   *
   *    where NTBlk means total number of blocks
   *          NTClt means total number of clusters of the data zone
   *          RPB means total number of references to clusters which can be stored in a block
   *          NBlkTIN means total number of blocks required to store the inode table
   *          BLOCKS_PER_CLUSTER means number of blocks which fit in a cluster
   *          sige (.) means the smallest integer greater or equal to the argument
   *
   * has integer solutions
   * this is not always true, so a final adjustment may be made to the parameter NBlkTIN to warrant this
   * furthermore, since the equation is non-linear, the procedure to solve it requires several steps
   * (in this case three)
   */

  uint32_t ntotal;                               /* total number of blocks */
  uint32_t iblktotal;                            /* number of blocks of the inode table */
  uint32_t nclusttotal;                          /* total number of clusters */
  uint32_t fcblktotal;                           /* number of blocks of the table of references to data clusters */
  uint32_t tmp;                                  /* temporary variable */

  ntotal = st.st_size / BLOCK_SIZE;
  if (itotal == 0) itotal = ntotal >> 3;         /* use the default value */
  if ((itotal % IPB) == 0)
     iblktotal = itotal / IPB;
     else iblktotal = itotal / IPB + 1;
                                                 /* step number 1 */
  tmp = (ntotal - 1 - iblktotal) / BLOCKS_PER_CLUSTER;
  if ((tmp % RPB) == 0)
	 fcblktotal = tmp / RPB;
     else fcblktotal = tmp / RPB + 1;
                                                 /* step number 2 */
  nclusttotal = (ntotal - 1 - iblktotal - fcblktotal) / BLOCKS_PER_CLUSTER;
  if ((nclusttotal % RPB) == 0)
	 fcblktotal = nclusttotal / RPB;
     else fcblktotal = nclusttotal / RPB + 1;
                                                 /* step number 3 */
  if ((nclusttotal % RPB) != 0)
     { if ((ntotal - 1 - iblktotal - fcblktotal - nclusttotal * BLOCKS_PER_CLUSTER) >= BLOCKS_PER_CLUSTER)
          nclusttotal += 1;
     }
                                                 /* final adjustment */
  iblktotal = ntotal - 1 - fcblktotal - nclusttotal * BLOCKS_PER_CLUSTER;
  itotal = iblktotal * IPB;

  /* formatting of the storage device is going to start */

  SOSuperBlock *p_sb;                            /* pointer to the superblock */
  int status;                                    /* status of operation */

  if (!quiet)
     printf("\e[34mInstalling a %"PRIu32"-inodes SOFS15 file system in %s.\e[0m\n", itotal, argv[optind]);

  /* open a buffered communication channel with the storage device */

  if ((status = soOpenBufferCache (argv[optind], BUF)) != 0)
     { printError (status, basename (argv[0]));
       return EXIT_FAILURE;
     }

  /* read the contents of the superblock to the internal storage area
   * this operation only serves at present time to get a pointer to the superblock storage area in main memory
   */

  if ((status = soLoadSuperBlock ()) != 0)
     return status;
  p_sb = soGetSuperBlock ();

  /* filling in the superblock fields:
   *   magic number should be set presently to 0xFFFF, this enables that if something goes wrong during formating, the
   *   device can never be mounted later on
   */

  if (!quiet)
     { printf ("Filling in the superblock fields ... ");
       fflush (stdout);                          /* make sure the message is printed now */
     }

  if ((status = fillInSuperBlock (p_sb, ntotal, itotal, fcblktotal, nclusttotal, (unsigned char *) name)) != 0)
     { printError (status, basename (argv[0]));
       soCloseBufferCache ();
       return EXIT_FAILURE;
     }

  if (!quiet) printf ("done.\n");

  /* filling in the inode table:
   *   only inode 0 is in use (it describes the root directory)
   */

  if (!quiet)
     { printf ("Filling in the table of inodes ... ");
       fflush (stdout);                          /* make sure the message is printed now */
     }

  if ((status = fillInINT (p_sb)) != 0)
     { printError (status, basename (argv[0]));
       soCloseBufferCache ();
       return EXIT_FAILURE;
     }

  if (!quiet) printf ("done.\n");

  /* filling in the contents of the root directory:
   *   the first 2 entries are filled in with "." and ".." references
   *   the other entries are kept empty
   */

  if (!quiet)
     { printf ("Filling in the contents of the root directory ... ");
       fflush (stdout);                          /* make sure the message is printed now */
     }

  if ((status = fillInRootDir (p_sb)) != 0)
     { printError (status, basename (argv[0]));
       soCloseBufferCache ();
       return EXIT_FAILURE;
     }

  if (!quiet) printf ("done.\n");

  /*
   * create the table of references to free data clusters as a static linear FIFO
   * zero fill the remaining data clusters if full formating was required:
   *   zero mode was selected
   */

  if (!quiet)
     { printf ("Filling in the contents of the table of references to free data clusters ... ");
       fflush (stdout);                          /* make sure the message is printed now */
     }

  if ((status = fillInTRefFDC (p_sb, zero)) != 0)
     { printError (status, basename (argv[0]));
       soCloseBufferCache ();
       return EXIT_FAILURE;
     }

  if (!quiet) printf ("done.\n");

  /* magic number should now be set to the right value before writing the contents of the superblock to the storage
     device */

  p_sb->magic = MAGIC_NUMBER;
  if ((status = soStoreSuperBlock ()) != 0)
     return status;

  /* check the consistency of the file system metadata */

  if (!quiet)
     { printf ("Checking file system metadata... ");
       fflush (stdout);                          /* make sure the message is printed now */
     }

  if ((status = checkFSConsist ()) != 0)
     { fprintf(stderr, "error # %d - %s\n", -status, soGetErrorMessage (-status));
       soCloseBufferCache ();
       return EXIT_FAILURE;
     }

  if (!quiet) printf ("done.\n");

  /* close the unbuffered communication channel with the storage device */

  if ((status = soCloseBufferCache ()) != 0)
     { printError (status, basename (argv[0]));
       return EXIT_FAILURE;
     }

  /* that's all */

  if (!quiet) printf ("Formating concluded.\n");

  return EXIT_SUCCESS;

} /* end of main */

/*
 * print help message
 */

static void printUsage (char *cmd_name)
{
  printf ("Sinopsis: %s [OPTIONS] supp-file\n"
          "  OPTIONS:\n"
          "  -n name --- set volume name (default: \"SOFS15\")\n"
          "  -i num  --- set number of inodes (default: N/8, where N = number of blocks)\n"
          "  -z      --- set zero mode (default: not zero)\n"
          "  -q      --- set quiet mode (default: not quiet)\n"
          "  -h      --- print this help\n", cmd_name);
}

/*
 * print error message
 */

static void printError (int errcode, char *cmd_name)
{
  fprintf(stderr, "%s: error #%d - %s\n", cmd_name, -errcode,
          soGetErrorMessage (-errcode));
}

  /* filling in the superblock fields:
   *   magic number should be set presently to 0xFFFF, this enables that if something goes wrong during formating, the
   *   device can never be mounted later on
   *  Author: Carla Gonçalves
   */

static int fillInSuperBlock (SOSuperBlock *p_sb, uint32_t ntotal, uint32_t itotal, uint32_t fcblktotal,
		                     uint32_t nclusttotal, unsigned char *name)
{
  unsigned int i;

  /*header*/

  p_sb->magic = 0xFFFF;     /* numero de serie do volume */

  p_sb->version = VERSION_NUMBER; /* versao do sofs12 */

  strncpy((char*) p_sb->name,(const char*) name,PARTITION_NAME_SIZE); /* preenchimento do campo name do superbloco */
  p_sb->name[PARTITION_NAME_SIZE] = '\0';

  p_sb->ntotal = ntotal;  /* numero total de blocos do disco */

  p_sb->mstat = PRU;      /* flag que sinaliza se o disco foi ou nao removido adequadamente do sistema */

  /*inode table*/

  p_sb->itable_start = 1; /* numero fisico do bloco onde a tabela de inodes comeca */

  p_sb->itable_size = itotal/IPB; /* de blocos que a tabela de inodes contem */

  p_sb->itotal = itotal; /* numero total de inodes */

  p_sb->ifree = itotal - 1; /* numero de inodes deixados livres */

  p_sb->ihdtl = itotal - p_sb->ifree; /* indice do elemento do array que forma a cabeca/cauda da lista bi-ligada de inodes livres */

  /*data zone*/

  p_sb->tbfreeclust_start = p_sb->itable_start + p_sb->itable_size; /* numero fisico do bloco onde se inicia a tabela de referencias */

  p_sb->dzone_total = nclusttotal; /* numero total de dataclusters */

  p_sb->dzone_free = nclusttotal - 1; /* numero de dataclusters livres */

  p_sb->dzone_retriev.cache_idx = DZONE_CACHE_SIZE;

  for (i = 0; i < DZONE_CACHE_SIZE; i++)
    p_sb->dzone_retriev.cache[i] = NULL_BLOCK;

  p_sb->dzone_insert.cache_idx = 0;

  for (i = 0; i < DZONE_CACHE_SIZE; i++)
    p_sb->dzone_insert.cache[i] = NULL_BLOCK; /*CHECK*/

  p_sb->tbfreeclust_size = fcblktotal; /* numero total de blocos da tabela de referencias */

  p_sb->tbfreeclust_head = 1;

  p_sb->tbfreeclust_tail = 0;// p_sb->dzone_total - 1;

  p_sb->dzone_start = p_sb->tbfreeclust_start + p_sb->tbfreeclust_size; /* numero fisico onde se inicia a zona de dados */
  
  for(i=0; i < (BLOCK_SIZE - PARTITION_NAME_SIZE - 1 - 16 * sizeof(uint32_t) - 2 * sizeof(struct fCNode));i++){
    p_sb->reserved[i] = 0xEE;
  }

  return 0;
}

/*
 * filling in the inode table:
 *   only inode 0 is in use (it describes the root directory)
 *  Author: Hugo Fragata
 */

static int fillInINT (SOSuperBlock *p_sb)
{
  SOInode *iNode;       /* Pointer to contents of each inode        */
  uint32_t block, inode, i;   /* variables to use in cycles. block for each block, inode for 
               each inode and i is used in the array of direct references in inode table */
  int status;       /* variable to return the errors if they happen   */
  
  /* Validate Arguments */
  if(p_sb == NULL)
    return -EINVAL;

  /*  Load the contents of all blocks of the table of inodes from devide storage  */
  /*      into internal storage and fill them as free inodes    */

  for( block = p_sb->itable_start; block < (p_sb->itable_size + p_sb->itable_start); block++ ) {
    if((status = soLoadBlockInT(block - p_sb->itable_start)) != 0)  /* Load block's content to internal storage */
      return status;
    
    iNode = soGetBlockInT();      /* Get a pointer to the content */
    
    for( inode = 0; inode < IPB; inode++ ) {  /* Put all inode of the block in free state */
      iNode[inode].mode = INODE_FREE;   /* flag signaling inode is free */
      iNode[inode].refcount = 0;
      iNode[inode].owner = 0;
      iNode[inode].group = 0;
      iNode[inode].size = 0;
      iNode[inode].clucount = 0;
      iNode[inode].vD1.prev = ((block-1)*IPB + inode - 1);      /* Reference to the prev and    */
      iNode[inode].vD2.next = ((block-1)*IPB + inode + 1);      /* next inode in the double-    */
                        /* linked list of free inodes */
      for (i = 0; i < N_DIRECT; i++)    
        iNode[inode].d[i] = NULL_CLUSTER;     /* Put all direct and indirect references to  */
                      /* data clusters pointing to null   */
      iNode[inode].i1 = iNode[inode].i2 = NULL_CLUSTER;       
    }                 

    if((status = soStoreBlockInT()) != 0 )          /* Store new values to storage device*/
      return status;
  }

  /* Fill in the inode 0 (root directory) */

  if((status = soLoadBlockInT(0)) != 0)     /* Load block's content to internal storage */
    return status;

  iNode = soGetBlockInT();        /* Get a pointer to the content */

  iNode[0].mode= INODE_RD_USR | INODE_WR_USR | INODE_EX_USR | INODE_RD_GRP | INODE_WR_GRP |
                 INODE_EX_GRP | INODE_RD_OTH | INODE_WR_OTH |INODE_EX_OTH | INODE_DIR;        /* flag signaling inode describes a directory and its permissions*/
  iNode[0].refcount=2;                /* references to previous and own directory*/
  iNode[0].owner=getuid();            /* get user and group id*/
  iNode[0].group=getgid();
  iNode[0].size = CLUSTER_SIZE;         /* size of the first i-node is one cluster's size */
  iNode[0].clucount=1;
  iNode[0].vD1.atime=time(NULL);        /* time.h  */
  iNode[0].vD2.mtime=time(NULL); 
  iNode[0].d[0] = 0;;             /* first direct reference points to cluster 0*/

  iNode[1].vD1.prev = (((block-1)*IPB)-1);    /* The prev of first inode is the last inode of double-linked list  */

  if((status = soStoreBlockInT()) != 0 )      /* Store new values to storage device*/
    return status;

  if((status = soLoadBlockInT(p_sb->itable_size -1)) != 0)  /* Load last block's content to internal storage */
    return status;

  iNode = soGetBlockInT();          /* Get a pointer to the content */
  iNode[IPB-1].vD2.next = 1;          /* The next of last inode, is first inode */

  if((status = soStoreBlockInT()) != 0)       /* Store new values to storage device*/
    return status;
    
  if((status = soStoreSuperBlock()) != 0)
    return status;

  return 0;
}

/*
	filling in the contents of the root directory:
	the first 2 entries are filled in with "." and ".." references
	the other entries are empty
  Author: Miguel Ferreira()
*/

static int fillInRootDir (SOSuperBlock *p_sb)
{
	unsigned int index;
	union soDataClust cluster;

	/* Clear the data segment to ensure its empty */
	memset(&(cluster.data), 0, BSLPC);

	/* Fill the first two entrys as "/." and "/.." references and Inode numbers to 0 (root directory)*/
	strcpy((char*) cluster.de[0].name, (const char*)".");
	cluster.de[0].nInode = 0;

	strcpy((char*) cluster.de[1].name, (const char*)"..");
	cluster.de[1].nInode = 0;

	/* Inicialize all other directory entrys */
	for (index = 2; index < DPC; index++)
	{
		/* Empty directory name */
		memset(&(cluster.de[index].name), 0, MAX_NAME + 1);
		/* Null inode number */
		cluster.de[index].nInode = NULL_INODE;
	}

	return soWriteCacheCluster(p_sb->dzone_start, &(cluster));;
}

  /*
   * create the table of references to free data clusters as a static circular FIFO
   * zero fill the remaining data clusters if full formating was required:
   *   zero mode was selected
   *  Author: Gonçalo Grilo
   */

static int fillInTRefFDC (SOSuperBlock *p_sb, int zero)
{
  int i, j, k, error;
  uint32_t *ref;
  SODataClust c; //cluster

  if(zero){ //escrever 0 em tudo

    for (i = 0;i < BSLPC; i++) //percorrer o array data[]
      c.data[i] = 0;

    for(i = p_sb->dzone_start + 4; i <  (p_sb->dzone_start + p_sb->dzone_total * 4); i += 4){  //percorrer todos os clusters e guardar
      if ((error = soWriteCacheCluster(i, &c)) != 0)
        return error;
    }
  }

  for(i = 0; i < p_sb->tbfreeclust_size; i++){ //passar por todos os blocos

    if ((error = soLoadBlockFCT(i)) != 0)
      return error;

    ref = soGetBlockFCT(); //get ref[RPB]

    for(j = 0; j < RPB; j++){
      if(i == 0 && j == 0)
        ref[j] = NULL_CLUSTER; //cluster 0
      else if((k = i * RPB + j) <= p_sb->dzone_free) //cluster livre
        ref[j] = k;
      else
        ref[j] = 0xFFFFFFFE; //cluster nao livre
    }

    if ((error = soStoreBlockFCT ()) != 0) //guardar bloco
      return error;
  }
  return 0;
}

/*
 * check the consistency of the file system metadata
 */

static int checkFSConsist (void)
{
  SOSuperBlock *p_sb;                            /* pointer to the superblock */
  SOInode *inode;                                /* pointer to the contents of a block of the inode table */
  int stat;                                      /* status of operation */

  /* read the contents of the superblock to the internal storage area and get a pointer to it */

  if ((stat = soLoadSuperBlock ()) != 0) return stat;
  p_sb = soGetSuperBlock ();

  /* check superblock and related structures */

if ((stat = soQCheckSuperBlock (p_sb)) != 0) return stat;

  /* read the contents of the first block of the inode table to the internal storage area and get a pointer to it */

if ((stat = soLoadBlockInT (0)) != 0) return stat;
inode = soGetBlockInT ();

  /* check inode associated with root directory (inode 0) and the contents of the root directory */

if ((stat = soQCheckInodeIU (p_sb, &inode[0])) != 0) return stat;
if ((stat = soQCheckDirCont (p_sb, &inode[0])) != 0) return stat;

  /* everything is consistent */

  return 0;
}
