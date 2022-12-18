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

# Structure of the inode
"""
0-10: direct
11: singly indirect
12: doubly indirect

| index | addrs[i] |    singly indirect      |            doubly indirect              |
|-------|----------|-------------------------|-----------------------------------------|
|   0   |        --|--> [ data block  0 ]
|   1   |        --|--> [ data block  1 ]
|   2   |        --|--> [ data block  2 ]
|   3   |        --|--> [ data block  3 ]
|  ...  |        --|--> [ data block ...]
|   9   |        --|--> [ data block  9 ]
|  10   |        --|--> [ data block 10 ]
|  11   |        --|-->   index block 0
                        [   entry  0    ] --> [data block  11 ]
                        [   entry  1    ] --> [data block  12 ]
                        [   entry  2    ] --> [data block  13 ]
                        [   entry ...   ] --> [data block ... ]
                        [   entry 255   ] --> [data block 266 ]
|  12   |        --|-->   index block 1
                        [ entry  0   ] -->   index block 2
                                            [   entry  0    ] --> [ data block  267  ]
                                            [   entry  1    ] --> [ data block  268  ]
                                            [   entry  2    ] --> [ data block  269  ]
                                            [   entry ...   ] --> [ data block  ...  ]
                                            [   entry 255   ] --> [ data block  522  ]
                        [ entry  1   ] -->   index block 3
                                            [   entry  0    ] --> [ data block  523  ]
                                            [   entry  1    ] --> [ data block  524  ]
                                            [   entry  2    ] --> [ data block  525  ]
                                            [   entry ...   ] --> [ data block  ...  ]
                                            [   entry 255   ] --> [ data block  776  ]
                        [ entry ...  ] -->   index block ...
                                            [   entry  0    ] --> [ data block  ...  ]
                                            [   entry  1    ] --> [ data block  ...  ]
                                            [   entry  2    ] --> [ data block  ...  ]
                                            [   entry ...   ] --> [ data block  ...  ]
                                            [   entry 255   ] --> [ data block  ...  ]
                        [ entry 255  ] -->   index block 255
                                            [   entry  0    ] --> [ data block 65547 ]
                                            [   entry  1    ] --> [ data block 65548 ]
                                            [   entry  2    ] --> [ data block 65549 ]
                                            [   entry ...   ] --> [ data block  ...  ]
                                            [   entry 255   ] --> [ data block 65802 ]
"""