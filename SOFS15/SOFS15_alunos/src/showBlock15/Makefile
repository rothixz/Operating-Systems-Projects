CC = gcc
CFLAGS = -Wall -I "../debugging" -I "../rawIO15" -I "../sofs15"
LFLAGS = -L "../../lib"

all32:			showblock_sofs15_32

showblock_sofs15_32:	showblock_sofs15.o
			$(CC) $(LFLAGS) -o showblock_sofs15 $^ -lsofs15bin_32 -lsofs15 -lrawIO15bin_32 -lrawIO15 -ldebugging
			cp showblock_sofs15 ../../run
			rm -f $^ showblock_sofs15

all64:			showblock_sofs15_64

showblock_sofs15_64:	showblock_sofs15.o
			$(CC) $(LFLAGS) -o showblock_sofs15 $^ -lsofs15bin_64 -lsofs15 -lrawIO15bin_64 -lrawIO15 -ldebugging
			cp showblock_sofs15 ../../run
			rm -f $^ showblock_sofs15

clean:
			rm -f ../../run/showblock_sofs15 ../../run/showblock_sofs15_32 ../../run/showblock_sofs15_64
