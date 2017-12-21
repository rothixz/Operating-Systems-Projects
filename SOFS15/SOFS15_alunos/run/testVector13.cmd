1 #alloc inode (directory)
1
6 #write inode
1 777
5 #read inode
1
1 #alloc inode (regular file)
2
6 #write inode
2 777
5 #read inode
2
1 #alloc inode (directory)
1
6 #write inode
3 777
5 #read inode
3
1 #alloc inode (directory)
1
6 #write inode
4 777
5 #read inode
4
1 #alloc inode (regular file)
2
6 #write inode
5 777
5 #read inode
5
1 #alloc inode (directory)
1
6 #write inode
6 777
5 #read inode
6
1 #alloc inode (directory)
1
6 #write inode
7 777
5 #read inode
7
14 # add dir entry
0 1 dira1
0
14 # add dir entry
0 2 filea2
0
14 # add dir entry
0 3 dira3
0
14 # add dir entry
1 4 dirb1
0
14 # add dir entry
1 5 fileb2
0
14 # add dir entry
3 6 dirb3
0
14 # add dir entry
3 5 fileb4
0
14 # add dir entry
4 7 dirc1
0
5 #read inode
0
5 #read inode
1
5 #read inode
2
5 #read inode
3
5 #read inode
4
5 #read inode
5
5 #read inode
6
5 #read inode
7
1 #alloc inode (symbolic link)
3
6 #write inode
8 777
18 #init symbolic link
8 ../../../dira3
14 # add dir entry
7 8 sl1
0
5 #read inode
8
1 #alloc inode (symbolic link)
3
6 #write inode
9 777
18 #init symbolic link
9 dira3/fileb4
14 # add dir entry
0 9 sl2
0
5 #read inode
9
1 #alloc inode (symbolic link)
3
6 #write inode
10 777
18 #init symbolic link
10 /filea2
14 # add dir entry
3 10 sl3
0
5 #read inode
10
12 #get dir entry by path
/sl2
12 #get dir entry by path
/dira1/dirb1/dirc1/sl1
12 #get dir entry by path
/dira1/dirb1/dirc1/sl1/fileb4
12 #get dir entry by path
/dira3/sl3
0
