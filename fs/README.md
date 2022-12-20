# File system

## Temp

> 因时间所限，没来得及在ddl前写完完整版报告，报告缺失部分可从 https://github.com/efJerryYang/xv6-labs-2021 仓库对应目录下查看

文件系统实验主要要完成的是两个部分，一个是将原来的其中一个直接索引改为添加的一个 doubly-indirect 的索引节点，以增大单个文件的最大容量，如下图所示，之后按照这个逻辑写代码即可：

```txt
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
```

第二个部分是添加一个符号链接，主要的整体逻辑很自然，主要遇到的问题是没有理解清楚题干给的 O_NOFOLLOW 的意思，导致代码完成之后评测始终通过不了，之后注意到问题在于没有理解 O_NOFOLLOW 是用于强调访问时不访问到最下层指向的 inode 而是直接返回当前符号链接的 inode。这个点弄清楚了之后就能跑过测试了

![](./resources/fs-make-grade.png)

## Large files (moderate)

To handle large files in an operating system with an ext2-like filesystem, it is necessary to analyze the code example provided for handling inode layout, which stores pointers to the data blocks.
<!-- To handle the large files in an operating system with ext2-like filesystem, it is straightforward to start analyzing the code example given for handling inode layout, where store the pointers to the datablocks. -->

The given filesystem inode in this case should have the following layout: (the layout was generated using `copilot`, so there is no script available for creating it)
<!-- In this case the given filesystem inode should look like the following: (the layout is generated with the help of copilot, so there is no script for creating it) -->

```txt
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
```

To implement the doubly-indirect block, we can follow the structure of the singly-indirect block and modify the `bmap` function as follows:

1. First, check if the given block number (`bn`) is within the range of the direct blocks (`0` to `NDIRECT-1`). If it is, return the block number as is.
2. If the block number is within the range of the singly-indirect blocks (`NDIRECT` to `NDIRECT + NINDIRECT - 1`), compute the group index and in-group index as described above. Then, use these indices to access the corresponding block in the singly-indirect block.
3. If the block number is within the range of the doubly-indirect blocks (`NDIRECT + NINDIRECT` to `NDIRECT + NINDIRECT + NINDIRECT^2 - 1`), repeat the process used in step 2, but this time accessing the block in the doubly-indirect block.

For example, to locate the datablock with number 511 in the doubly-indirect block, we would first compute the group index as `511 / NINDIRECT = 1` and the in-group index as `511 % NINDIRECT = 255`. This would tell us that we need to access the 255th block in the second group of the doubly-indirect block.

So we can have the following code:
<!-- So we can just follow the structure to manage our filesystem. According to the instructions given, we should modify the `bmap` function to enable the use of a doubly-indirect block, and we shall start from here.

Additionally, we wrote a script to enumerate the acceptable choices for these 3 types of index (direct, singly-indirect and doubly-indirect), although it was not necessary here, the instructions have already explicitly specified the task we should accomplish.

According to the layout we have above, we can see that the index blocks should contain the same number of data block entries. For the first level of index blocks (index block 0 and index block 1 above), they should contain 256 entries each, and for the second level of index blocks should also contain the same number of entries.

So, for the singly-indirect block with index 11, it should contain 256 datablocks; for the doubly-indirect block with index 12, it should contain 256 * 256 = 65536 datablocks in total. But for convenience, we can compute the index by group, each group of the datablocks in doubly-indirect block will be identified by the index block entry number, so when we need to locate specific datablocks in the doubly-indirect block, it is easy to compute the group index and the in-group index with `bn / NINDIRECT` and `bn % NINDIRECT`, respectively. Here, `bn` represents the datablock number, `NINDIRECT` represents the number of first level indirect blocks.

Therefore, we can follow the implementation for singly-indirect blocks to accomplish our doubly-indirect blocks like this: -->
```c
// in file fs.c, function bmap
...
  // the part handling the direct and singly-indirect blocks
  ...

  bn -= NINDIRECT;
  if (bn < NDINDIRECT) {
    // Load doubly-indirect block, allocating if necessary.
    if ((addr = ip->addrs[NDIRECT + 1]) == 0)
      ip->addrs[NDIRECT + 1] = addr = balloc(ip->dev);
    bp = bread(ip->dev, addr);
    a = (uint*)bp->data;
    if ((addr = a[bn / NINDIRECT]) == 0) {
      a[bn / NINDIRECT] = addr = balloc(ip->dev);
      log_write(bp);
    }
    brelse(bp);
    bp = bread(ip->dev, addr);
    a = (uint*)bp->data;
    if ((addr = a[bn % NINDIRECT]) == 0) {
      a[bn % NINDIRECT] = addr = balloc(ip->dev);
      log_write(bp);
    }
    brelse(bp);
    return addr;
  }
```

To modify the code that handles discarding the inode in the `itrunc` function, we can follow a similar structure as the original code handling the singly-indirect blocks. Here is the code that should be added:

1. First, we need to handle the doubly-indirect block by looping through the first level index entries. We can do this by adding a `for` loop that iterates from the base index of the `addr` array (which is the index of the doubly-indirect block, DNIRECT + 1) to the end of the array.
2. Inside the loop, we need to check if the current index entry is non-zero. If it is, we should free the block pointed to by the entry and set the entry to zero.
3. After the loop, we can proceed with the rest of the `itrunc` function as normal, handling the singly-indirect and direct blocks in the same way as before.
<!-- Additionally, we need to modify code that discarding the indoe in function `itrunc`. We can insert our code handling the doubly-indirect blocks in a similar with as the original code handling the singly-indirect blocks. The only differences here is the base index for `addr` array should be the index of the doubly-indirect index `DNIRECT + 1`, and there should be a loop to iterate through the first level index entries.

Here is the code we should add: -->
```c 
// in file fs.c, function itrunc
  ...
  // trucate inode, handle the case for direct and singly-indirect index blocks
  ...
  // handle the case for doubly-indirect blocks
  if(ip->addrs[NDIRECT + 1]) {
    bp = bread(ip->dev, ip->addrs[NDIRECT + 1]);
    a = (uint*)bp->data;
    for (i = 0; i < NINDIRECT; i++) {
      if (a[i]) {
        struct buf *bp2 = bread(ip->dev, a[i]);
        uint *a2 = (uint*)bp2->data;
        for (j = 0; j < NINDIRECT; j++) {
          if (a2[j])
            bfree(ip->dev, a2[j]);
        }
        brelse(bp2);
        bfree(ip->dev, a[i]);
      }
    }
    brelse(bp);
  }
  ...
```

After making these modifications, the `bigfile` test should pass. It is expected that running this test will take some time, as it involves creating a large file that utilizes the doubly-indirect block. On our machine, the default time limit for running the test is not enough, so we update the `Makefile` by setting the `timeout` in `test_bigfile` from `180` to `300`.
<!-- We should pass the `bigfile` test now, notice that it really takes time to run the test, and the time limit for our machine is not enough, so when run `make grade` for this lab, we have to update the time limit in the `Makefile`. -->

## Symbolic links (moderate)
