#!/bin/bash

# This test vector deals with the operations alloc / free data clusters.
# It defines a storage device with 19 blocks and formats it with an inode table of 8 inodes.
# It starts by allocating all the data clusters and testing the error condition. Then, it frees
# all the allocated data clusters in the reverse order of allocation while still testing different
# error conditions.
# The showblock_sofs15 application should be used in the end to check metadata.

./createEmptyFile myDisk 19
./mkfs_sofs15 -n SOFS15 -i 8 -z myDisk
./testifuncs15 -b -l 600,700 -L testVector3.rst myDisk <testVector3.cmd
