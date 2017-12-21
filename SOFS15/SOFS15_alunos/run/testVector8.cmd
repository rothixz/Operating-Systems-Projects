1 #alloc inode
2
6 #write inode
1 666
5 #read inode
1
10 #handle file cluster alloc
1 0 1
10 #handle file cluster alloc
1 6 1
10 #handle file cluster alloc
1 100 1
10 #handle file cluster alloc
1 102 1
10 #handle file cluster alloc
1 110 1
10 #handle file cluster alloc
1 1000 1
10 #handle file cluster alloc
1 1001 1
10 #handle file cluster alloc
1 2000 1
10 #handle file cluster alloc
1 3000 1
10 #handle file cluster alloc
1 4000 1
10 #handle file cluster alloc
1 5000 1
10 #handle file cluster alloc
1 5001 1
10 #handle file cluster alloc
1 5002 1
10 #handle file cluster alloc
1 5010 1
5 #read inode
1
10 #handle file cluster alloc   (it is going to fail - there are not enough data clusters available)
1 10000 1
5 #read inode
1
0
