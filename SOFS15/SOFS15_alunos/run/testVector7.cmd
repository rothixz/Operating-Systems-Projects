1 #alloc inode
2
6 #write inode
1 666
5 #read inode
1
10 #handle file cluster alloc    (it is going to fail - inode is free)
10 0 1
10 #handle file cluster alloc    (it is going to fail - inode number out of range)
100 0 1
10 #handle file cluster ---      (it is going to fail - illegal operation)
1 0 3
10 #handle file cluster alloc
1 0 1
10 #handle file cluster alloc
1 6 1
10 #handle file cluster alloc    (it is going to fail - a data cluster was already allocated at that index)
1 6 1
10 #handle file cluster alloc
1 100 1
10 #handle file cluster alloc
1 102 1
10 #handle file cluster alloc
1 110 1
10 #handle file cluster alloc    (it is going to fail - a data cluster was already allocated at that index)
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
10 #handle file cluster alloc    (it is going to fail - a data cluster was already allocated at that index)
1 4000 1
10 #handle file cluster alloc
1 5000 1
10 #handle file cluster alloc
1 5001 1
10 #handle file cluster alloc
1 5002 1
5 #read inode
1
10 #handle file cluster get
1 0 0
10 #handle file cluster get
1 1 0
10 #handle file cluster get
1 10 0
10 #handle file cluster get
1 110 0
10 #handle file cluster get
1 5000 0
10 #handle file cluster get
1 10000 0
10 #handle file cluster free
1 5002 2
10 #handle file cluster free
1 5001 2
10 #handle file cluster free    (it is going to fail - there was not a data cluster allocated at that index)
1 5010 2
10 #handle file cluster free
1 5000 2
10 #handle file cluster free
1 4000 2
10 #handle file cluster free
1 3000 2
10 #handle file cluster free
1 2000 2
10 #handle file cluster free
1 1001 2
10 #handle file cluster free
1 1000 2
10 #handle file cluster free
1 110 2
10 #handle file cluster free
1 102 2
10 #handle file cluster free    (it is going to fail - there was not a data cluster allocated at that index)
1 200 2
10 #handle file cluster free
1 100 2
10 #handle file cluster free
1 6 2
10 #handle file cluster free    (it is going to fail - there was not a data cluster allocated at that index)
1 6 2
10 #handle file cluster free
1 0 2
5 #read inode
1
2 #free inode
1
10 #handle file cluster get    (it is going to fail - inode is free)
1 0 0
10 #handle file cluster alloc    (it is going to fail - inode is free)
1 0 1
10 #handle file cluster free    (it is going to fail - inode is free)
1 0 2
0
