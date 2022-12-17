#!/usr/bin/env python3

def largest_file(direct, singly_indirect, doubly_indirect):
    """Return the largest file size that can be stored in a inode."""
    return direct * 1 + singly_indirect * 256 + doubly_indirect * 256**2

param_list = []
for i in range(0, 14):
    for j in range(0, 14-i):
        for k in range(0, 14-i-j):
            maxsize = largest_file(i, j, k)
            if maxsize >= 65803:
                param_list.append((i, j, k, maxsize))
                # print(i, j, k, maxsize, 65803)

param_list.sort(key=lambda x: x[3], reverse=False)
for i in param_list:
    print(i)