#!/bin/bash

# This test vector deals with operations alloc / free inodes, read / write inodes and access granted.
# It defines a storage device with 100 blocks and formats it with an inode table of 8 inodes.
# It starts by allocating two inodes, freeing one of them and setting their permissions. Several
# parameter combinations are tested to check error conditions. Then, it tests different combinations
# of requested access operations.
# The showblock_sofs15 application should be used in the end to check metadata.

./createEmptyFile myDisk 100
./mkfs_sofs15 -n SOFS15 -i 8 -z myDisk
./testifuncs15 -b -l 500,700 -L testVector6.rst myDisk <testVector6.cmd
