CC = gcc
CFLAGS = -Wall -D_FILE_OFFSET_BITS=64 -I "../debugging" -I "../rawIO15" -I "../sofs15"
#IFUNCS = soLink.o soUnlink.o soMknod.o soRead.o soWrite.o soTruncate.o soMkdir.o soRmdir.o soReaddir.o \
#	 soRename.o soSymlink.o soReadlink.o
IFUNCS = soReaddir.o soRename.o soTruncate.o soWrite.o soMkdir.o

all:			libsyscalls15

libsyscalls15:		$(IFUNCS)
			ar -r libsyscalls15.a $^
			cp libsyscalls15.a ../../lib
			rm -f $^ libsyscalls15.a

clean:
			rm -f ../../lib/libsyscalls15.a
			rm -f *.o libsyscalls15.a
