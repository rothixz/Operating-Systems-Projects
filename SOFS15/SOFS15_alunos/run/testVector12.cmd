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
12 #get dir entry by path
/dira1/dirb1/dirc1
12 #get dir entry by path
/
12 #get dir entry by path
/filea2
12 #get dir entry by path  (it is going to fail - relative path)
./filea2
12 #get dir entry by path  (it is going to fail - relative path)
filea2
12 #get dir entry by path
/dira1/dirb1/dirc1/../../../dira3/fileb4
12 #get dir entry by path
/dira3/./.././dira3/dirb3
12 #get dir entry by path  (it is going to fail - fa2 is not a directory)
/dira3/./.././filea2/dirb3
6 #write inode
3 666
5 #read inode
3
12 #get dir entry by path  (it is going to fail - da3 has no execution permission)
/dira3/dirb3
0
