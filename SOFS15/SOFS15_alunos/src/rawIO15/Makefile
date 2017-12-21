CC = gcc
CFLAGS = -Wall -I "../debugging"

all:			librawIO15

librawIO15:		sofs_rawdisk.o
			ar -r librawIO15.a $^
			cp librawIO15.a ../../lib
			rm -f $^ librawIO15.a

clean:
			rm -f ../../lib/librawIO15.a
