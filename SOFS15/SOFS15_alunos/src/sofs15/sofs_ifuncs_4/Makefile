CC = gcc
CFLAGS = -Wall -I "../../debugging" -I "../../rawIO15" -I "../../sofs15"
IFUNCS4 = soGetDirEntryByPath.o soGetDirEntryByName.o \
	  soAddAttDirEntry.o soRemDetachDirEntry.o \
	  soRenameDirEntry.o

all:			ifuncs4

ifuncs4:		$(IFUNCS4)

clean:
			rm -f $(IFUNCS4)
