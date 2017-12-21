#!/bin/bash

# This test vector checks if FUSE can mount the storage device in an empty file system scenario.
# It only checks if the formatting tool is operational.
# Basic system calls involved: readdir.

echo -e '\n**** Creating the storage device.****\n'
./createEmptyFile myDisk 100
echo -e '\n**** Converting the storage device into a SOFS15 file system.****\n'
./mkfs_sofs15 -i 56 -z myDisk
echo -e '\n**** Mounting the storage device as a SOFS15 file system.****\n'
./mount_sofs15 myDisk mnt
echo -e '\n**** Listing the root directory.****\n'
ls -la mnt
echo -e '\n**** Getting the file system attributes.****\n'
stat -f mnt/.
echo -e '\n**** Getting the root directory attributes.****\n'
stat mnt/.
echo -e '\n**** Unmounting the storage device.****\n'
sleep 1
fusermount -u mnt
