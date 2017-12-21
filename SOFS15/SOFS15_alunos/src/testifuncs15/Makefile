CC = gcc
CFLAGS = -Wall -I "../debugging" -I "../rawIO15" -I "../sofs15"
LFLAGS = -L "../../lib"

all32:			testifuncs15_32

testifuncs15_32:	testifuncs15.o
			$(CC) $(LFLAGS) -o testifuncs15 $^ -lsofs15 -lsofs15bin_32 -lrawIO15bin_32 -lrawIO15 -ldebugging
			cp testifuncs15 ../../run
			rm -f $^ testifuncs15

all64:			testifuncs15_64

testifuncs15_64:	testifuncs15.o
			$(CC) $(LFLAGS) -o testifuncs15 $^ -lsofs15 -lsofs15bin_64 -lrawIO15bin_64 -lrawIO15 -ldebugging
			cp testifuncs15 ../../run
			rm -f $^ testifuncs15

clean:
			rm -f ../../run/testifuncs15
