CC = gcc
CFLAGS = -Wall -I "../../debugging" -I "../../rawIO15" -I "../../sofs15"
IFUNCS3 = soReadFileCluster.o soWriteFileCluster.o \
	  soHandleFileCluster.o soHandleFileClusters.o

all:			ifuncs3

ifuncs3:		$(IFUNCS3)

clean:
			rm -f $(IFUNCS3)
