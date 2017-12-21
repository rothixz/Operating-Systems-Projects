CC = gcc
CFLAGS = -Wall -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 -I "../debugging" -I "../rawIO15" -I "../sofs15" -I "../syscalls15"
LFLAGS = -L "../../lib" -L/lib

all32:			mount_sofs15_32

mount_sofs15_32:	mount_sofs15.o
			$(CC) $(LFLAGS) -o mount_sofs15 $^ -lsyscalls15 -lsyscalls15bin_32 -lsofs15 -lsofs15bin_32 -lrawIO15bin_32 \
			-lrawIO15 -ldebugging -lpthread -lfuse
			cp mount_sofs15 ../../run
			rm -f $^ mount_sofs15

all64:			mount_sofs15_64

mount_sofs15_64:	mount_sofs15.o
			$(CC) $(LFLAGS) -o mount_sofs15 $^ -lsyscalls15 -lsyscalls15bin_64 -lsofs15 -lsofs15bin_64 -lrawIO15bin_64 \
			-lrawIO15 -ldebugging -lpthread -lfuse
			cp mount_sofs15 ../../run
			rm -f $^ mount_sofs15

clean:
			rm -f ../../run/mount_sofs15 ../../run/mount_sofs15_32 ../../run/mount_sofs15_64
