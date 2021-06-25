Title: OSlabs project 5 - Defragmentation
Author: WeiYang[10142130214]
Date: Dec 23 2016
=================================================================================================

	这次project要求写一个文件系统去碎片化程序.

		文件系统结构如下:
		| boot | superblock | inode1~5 | inode6~10 | inode11~15 | inode16~20 | null | dblock | iblock | null | dblock | ... |
		| boot | superblock |                       inode                    | null |              data region              |
		
	要求实现将数据块去碎片化,空闲块全部合并.
	我的方法如下:
		首先boot原封不动复制到目标文件系统中,
		再复制superblock,并修改free_iblock等值.
		再暂且复制inode到目标文件系统中,留作最后修改.
		接着将数据块去碎片化后重排写入文件系统.
		
		最后进行修改最为复杂的inode:
		
			由于去碎片化后的文件系统结构是这样的:
				| boot | superblock | inode1~5 | inode6~10 | inode11~15 | inode16~20 | null | dblock | dblock | lblock | null | ... |
				| boot | superblock |                       inode                    | null |       data region        | swap region|
			所以数据块排好之后,iblock索引块位置可以算出来,只要从数据块最后一块之后的空闲块开始依次排放即可.
			
			设置两个全局变量:data_block_id,iblock_id,分别表示当前待索引的数据块编号和当前待分配的索引块编号
			具体分为以下4种情况:
				1.直接索引
					对dblocks数组依次赋值()即可.
				2.1级索引
					对于iblocks数组依次分配一个索引块,对于每个索引块,可以存放512 / 4 = 128个索引,依次分配数据块号就行.
				3.2级索引
					给i2block分配一个2级索引块,可以存放512 / 4 = 128个1级索引.对于每个1级索引,递归调用1级索引分配方法即可.
				4.3级索引
					给i3block分配一个3级索引块,可以存放512 / 4 = 128个2级索引.对于每个2级索引,递归调用2级索引分配方法即可.
		
		最后再计算出需要补全的空闲块数量,整个程序就完成了.
		
	经过测试,作业要求给的文件系统一共有20个inode,其中17个inode存放着文件.
