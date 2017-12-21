CC = gcc
CFLAGS = -Wall -I "../debugging" -I "../rawIO15"
IFUNCS1 = sofs_ifuncs_1/soAllocInode.o sofs_ifuncs_1/soFreeInode.o sofs_ifuncs_1/soAllocDataCluster.o \
	  sofs_ifuncs_1/soFreeDataCluster.o
IFUNCS2 = sofs_ifuncs_2/soReadInode.o sofs_ifuncs_2/soWriteInode.o sofs_ifuncs_2/soAccessGranted.o
IFUNCS3 = sofs_ifuncs_3/soReadFileCluster.o sofs_ifuncs_3/soWriteFileCluster.o \
	  sofs_ifuncs_3/soHandleFileCluster.o sofs_ifuncs_3/soHandleFileClusters.o
IFUNCS4 = sofs_ifuncs_4/soGetDirEntryByPath.o sofs_ifuncs_4/soGetDirEntryByName.o \
	  sofs_ifuncs_4/soAddAttDirEntry.o sofs_ifuncs_4/soRemDetachDirEntry.o \
	  sofs_ifuncs_4/soRenameDirEntry.o
#IFUNCS4 = sofs_ifuncs_4/soGetDirEntryByPath.o sofs_ifuncs_4/soGetDirEntryByName.o \
#	  sofs_ifuncs_4/soAddAttDirEntry.o \
#	  sofs_ifuncs_4/soRenameDirEntry.o

all:			ifuncs1 ifuncs2 ifuncs3 ifuncs4 libsofs15

ifuncs1:
			make -C sofs_ifuncs_1 all

ifuncs2:
			make -C sofs_ifuncs_2 all

ifuncs3:
			make -C sofs_ifuncs_3 all

ifuncs4:
			make -C sofs_ifuncs_4 all

libsofs15:		sofs_blockviews.o sofs_basicoper.o $(IFUNCS1) $(IFUNCS2) $(IFUNCS3) $(IFUNCS4)
			ar -r libsofs15.a $^
			cp libsofs15.a ../../lib
			rm -f $^ libsofs15.a

clean:
			rm -f ../../lib/libsofs15.a
			rm -f *.o */*.o libsofs15.a
