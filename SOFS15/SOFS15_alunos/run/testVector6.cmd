1 #alloc inode
2
1 #alloc inode
2
2 #free inode
1
6 #write inode (it is going to fail - inode 1 is free)
1 555
6 #write inode (it is going to fail - inode 100 is out of range)
100 555
6 #write inode (it is going to fail - inode 7 is free)
7 555
6 #write inode
2 555
5 #read inode (it is going to fail - inode 1 is free)
1
5 #read inode (it is going to fail - inode 100 is out of range)
100
5 #read inode (it is going to fail - inode 7 is free)
7
5 #read inode
2
7 #access granted (it is going to fail - inode 1 is free)
1 1
7 #access granted (it is going to fail - inode 100 is out of range)
100 2
7 #access granted (it is going to fail - inode 7)
7 4
7 #access granted (it is going to fail - illegal mask)
2 0
7 #access granted (it is going to fail - illegal mask)
2 9
7 #access granted (single test)
2 1
7 #access granted (double test)
2 5
7 #access denied (single test)
2 2
7 #access denied (double test)
2 6
7 #access denied (triple test)
2 7
6 #write inode
2 777
5 #read inode
2
7 #access granted (triple test)
2 7
0
