#!/bin/bash

# This test vector deals mainly with operations get a directory entry by name and add / remove directory
# entries.
# It defines a storage device with 100 blocks and formats it with 48 inodes.
# It starts by allocating thirty two inodes, associated to regular files and directories, and organize
# them as entries in the root directory. It proceeds by removing all of them. Finally, it allocates two
# additional inodes, associated to regular files, and organize them also as entries of the root
# directory.
# The major aim is to check the growth of the root directory and the use of freed directory entries.
# The showblock_sofs15 application should be used in the end to check metadata.

./createEmptyFile myDisk 100
./mkfs_sofs15 -n SOFS15 -i 48 -z myDisk
./testifuncs15 -b -l 300,700 -L testVector11.rst myDisk <testVector11.cmd
