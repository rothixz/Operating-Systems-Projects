#!/bin/bash

# This test vector deals mainly with operation get a directory entry by path.
# It defines a storage device with 100 blocks and formats it with 48 inodes.
# It starts by allocating seven inodes, associated to regular files and directories, and organize them
# in a hierarchical faction. Then it proceeds by defining some symbolic links and trying to find
# different directory entries through the use of different paths containing symbolic links.
# The showblock_sofs15 application should be used in the end to check metadata.

./createEmptyFile myDisk 100
./mkfs_sofs15 -n SOFS15 -i 48 -z myDisk
./testifuncs15 -b -l 300,700 -L testVector13.rst myDisk <testVector13.cmd
