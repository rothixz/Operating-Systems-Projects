#!/bin/bash

# This test vector deals mainly with operation attach a directory, detach a direntry and add a direntry.
# It defines a storage device with 100 blocks and formats it with 48 inodes.
# It starts by allocating ten inodes, associated to regular files, directories and symbolic links, and
# organize them in a hierarchical faction. Then it proceeds by moving one directory in the hierarchical
# tree.
# The showblock_sofs15 application should be used in the end to check metadata.

./createEmptyFile myDisk 100
./mkfs_sofs15 -n SOFS15 -i 48 -z myDisk
./testifuncs15 -b -l 300,700 -L testVector14.rst myDisk <testVector14.cmd
