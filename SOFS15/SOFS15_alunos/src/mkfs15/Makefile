CC = gcc
CFLAGS = -Wall -I "../debugging" -I "../rawIO15" -I "../sofs15"
LFLAGS = -L "../../lib"

all32:			mkfs_sofs15_32

mkfs_sofs15_32:		mkfs_sofs15.o
			$(CC) $(LFLAGS) -o mkfs_sofs15 $^ -lsofs15bin_32 -lsofs15 -lrawIO15bin_32 -lrawIO15 -ldebugging
			cp mkfs_sofs15 ../../run
			rm -f $^ mkfs_sofs15

all64:			mkfs_sofs15_64

mkfs_sofs15_64:		mkfs_sofs15.o
			$(CC) $(LFLAGS) -o mkfs_sofs15 $^ -lsofs15bin_64 -lsofs15 -lrawIO15bin_64 -lrawIO15 -ldebugging
			cp mkfs_sofs15 ../../run
			rm -f $^ mkfs_sofs15

clean:
			rm -f ../../run/mkfs_sofs15 ../../run/mkfs_sofs15_32 ../../run/mkfs_sofs15_64
