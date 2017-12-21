#!/bin/bash

# This test vector deals mainly with operations read / write data clusters and handle file clusters.
# It defines a storage device with 1000 blocks and formats it with 246 data clusters.
# It starts by allocating an inode, then it proceeds by writing in 60 data clusters in all the reference
# areas (direct, single indirect and double indirect). This means that in fact 70 data clusters are
# allocated. Then two data clusters are read (one previously allocated and one not yet allocated), all
# the data clusters are freed and the inode is freed.
# The showblock_sofs15 application should be used in the end to check metadata.

./createEmptyFile myDisk 1000
./mkfs_sofs15 -n SOFS15 -i 100 -z myDisk
./testifuncs15 -b -l 400,700 -L testVector9.rst myDisk <testVector9.cmd
