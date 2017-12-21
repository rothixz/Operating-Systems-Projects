#!/bin/bash

# This test vector checks the capability of renaming / moving files in the file system.
# Basic system calls involved: readdir, mknode, read, write, mkdir, link, rmdir, unlink and rename.

echo -e '\n**** Creating the storage device.****\n'
./createEmptyFile myDisk 200
echo -e '\n**** Converting the storage device into a SOFS15 file system.****\n'
./mkfs_sofs15 -i 56 -z myDisk
echo -e '\n**** Mounting the storage device as a SOFS15 file system.****\n'
./mount_sofs15 myDisk mnt
echo -e '\n**** Creating the directory hierarchy.****\n'
mkdir mnt/ex
mkdir mnt/testVec
mkdir mnt/new
mkdir mnt/new/newAgain
cp ex*.sh mnt/ex
cp testVector*.cmd mnt/testVec
ln mnt/ex/ex10.sh mnt/new/newAgain/sameAsEx10.sh
sleep 1
echo -e '\n**** Listing all the directories.****\n'
ls -la mnt
echo -e '\n********\n'
ls -la mnt/ex
echo -e '\n********\n'
ls -la mnt/testVec
echo -e '\n********\n'
ls -la mnt/new
echo -e '\n********\n'
ls -la mnt/new/newAgain
echo -e '\n**** Renaming the name of a file and a directory.****\n'
echo -e '\n**** ex/ex10.sh --> ex/newNameForEx10.sh ****\n'
echo -e '\n**** new --> newNameForNew ****\n'
mv mnt/ex/ex10.sh mnt/ex/newNameForEx10.sh
mv mnt/new mnt/newNameForNew
echo -e '\n**** Listing some of the directories.****\n'
ls -la mnt
echo -e '\n********\n'
ls -la mnt/ex
echo -e '\n**** Moving a file to another directory.****\n'
echo -e '\n**** ex/ex1.sh --> newNameForNew/newPlaceForEx1.sh ****\n'
echo -e '\n**** ex/ex2.sh --> testVec/testVector2.cm ****\n'
mv mnt/ex/ex1.sh mnt/newNameForNew/newPlaceForEx1.sh
mv mnt/ex/ex2.sh mnt/testVec/testVector2.cmd
echo -e '\n**** Listing some of the directories.****\n'
ls -la mnt/newNameForNew
echo -e '\n********\n'
ls -la mnt/testVec
echo -e '\n********\n'
ls -la mnt/ex
echo -e '\n**** Moving a directory.****\n'
echo -e '\n**** newNameForNew/newAgain --> ex ****\n'
mv mnt/newNameForNew/newAgain mnt/ex
echo -e '\n**** Listing some of the directories.****\n'
ls -la mnt/ex
echo -e '\n********\n'
ls -la mnt/ex/newAgain
echo -e '\n**** Unmounting the storage device.****\n'
sleep 1
fusermount -u mnt
