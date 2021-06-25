#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defrag.h"

int num_of_datablocks = 0;	//数据块个数
int file_system_bytes = 0;	//文件系统字节数
int size_of_block = 0;	//块大小
int data_block_id = 0;	//数据块编号
int iblock_id = 0;	//索引块编号
char s_filename[30], d_filename[30];	//源文件名,目标文件名

//读一级索引块
void read_iblocks(RB *rb, FILE *fin, int iblock) {
    fseek(fin, 0, SEEK_CUR);
    int c_offset = ftell(fin);
    fseek(fin, (iblock + 6) * size_of_block, SEEK_SET);

    for (int i = 0; i < (size_of_block / 4); ++i) {
    	//一个索引四个字节
        fread(&rb[num_of_datablocks].block_number, 4, 1, fin);
        if (rb[num_of_datablocks].block_number > (file_system_bytes / 512) || rb[num_of_datablocks].block_number <= 0) {
            break;
        }
        rb[num_of_datablocks].seq_number = num_of_datablocks;
        num_of_datablocks++;
    }
    
    fseek(fin, c_offset, SEEK_SET);
}

//读二级索引块
void read_i2blocks(RB *rb, IN *inode, FILE *fin) {
    int iblocks[size_of_block / 4];
    fseek(fin, 0, SEEK_CUR);
    int c_offset = ftell(fin);
    fseek(fin, (inode -> i2block + 6) * size_of_block, SEEK_SET);
    for (int i = 0; i < size_of_block / 4; ++i) {
        fread(&iblocks[i], 4, 1, fin);
        if (iblocks[i] > (file_system_bytes / 512) || iblocks[i] <= 0) {
            break;
        }
        read_iblocks(rb, fin, iblocks[i]);
    }
    fseek(fin, c_offset, SEEK_SET);
}

//读三级索引块
void read_i3blocks(RB *rb, IN *inode, FILE *fin) {
    int iblocks[size_of_block / 4];
    fseek(fin, 0, SEEK_CUR);
    int c_offset = ftell(fin);
    fseek(fin, (inode -> i3block + 6) * size_of_block, SEEK_SET);
    for (int i = 0; i < size_of_block / 4; ++i) {
        fread(&iblocks[i], 4, 1, fin);
        if (iblocks[i] > (file_system_bytes / 512) || iblocks[i] <= 0) {
            break;
        }
        read_i2blocks(rb, inode, fin);
    }
    fseek(fin, c_offset, SEEK_SET);
}

//读取文件块并缓存
void read_content(WB *wb, RB *rb, SB *superblock, IN *inode , FILE *fin) {
    fseek(fin, 0, SEEK_CUR);
    int c_offset = ftell(fin);
    fseek(fin, (rb[0].block_number + 6) * size_of_block, SEEK_SET);
    fread(wb[0].buffer, size_of_block, 1, fin);
    for (int i = 1; i < num_of_datablocks; ++i) {
        fseek(fin, (rb[i].block_number - rb[i - 1].block_number - 1) * size_of_block, SEEK_CUR);
        fread(wb[i].buffer, size_of_block, 1, fin);
    }
    fseek(fin, c_offset, SEEK_SET);
}

int cmp1(const void *a, const void *b) {
    return ((RB*)a)->block_number - ((RB*)b)->block_number;
}

int cmp2(const void *a, const void *b) {
    return ((WB*)a)->seq_number - ((WB*)b)->seq_number;
}

//将数据块全部读出并排序输出
void read_and_write_file(SB *super_block, IN *inode, FILE *fin) {
    int blocks_of_file = (inode->size - 1) / size_of_block + 1;
    RB rb[blocks_of_file];
    WB wb[blocks_of_file];
    
    //直接索引
    for (int i = 0; i < N_DBLOCKS; ++i) {
        if (inode->dblocks[i] == 0) break;
        rb[num_of_datablocks].block_number = inode->dblocks[i];
        rb[num_of_datablocks].seq_number = num_of_datablocks;
        num_of_datablocks++;
    }
    
    //一级索引
    if (blocks_of_file > N_DBLOCKS) {
        int num_of_iblocks = (blocks_of_file - 1) / (size_of_block / 4) + 1;
        for (int j = 0; j < num_of_iblocks; ++j) {
            if (j >= N_IBLOCKS) break;  
            read_iblocks(rb, fin, inode->iblocks[j]);
        }
    }
    
    //二级索引
    if (blocks_of_file > N_DBLOCKS + N_IBLOCKS * (size_of_block / 4)) {
        read_i2blocks(rb, inode, fin);
    }
    
    //三级索引
    if (blocks_of_file > N_DBLOCKS + N_IBLOCKS * (size_of_block / 4) + (size_of_block / 4) * (size_of_block / 4)) {
        read_i3blocks(rb, inode, fin);
    }
    
    //按数据块在文件系统中物理位置排序,方便读出
    qsort(rb, blocks_of_file, sizeof(rb[0]), cmp1);
    read_content(wb, rb, super_block, inode , fin);
    
    for (int i = 0; i < num_of_datablocks; ++i) {
        wb[i].seq_number = rb[i].seq_number;
    }
    
    //按数据块的逻辑顺序排序输出
    qsort(wb, blocks_of_file, sizeof(wb[0]), cmp2);
    FILE * fout = fopen(d_filename, "a+");
    fseek(fout, 0, SEEK_END);
    for (int i = 0; i < num_of_datablocks; ++i) {
        fwrite(wb[i].buffer, size_of_block, 1, fout);
    }
    fclose(fout);
}

//增加一级索引的索引块
void add_iblocks(int *iblock_id, int *block_cnt, FILE *fout) {
    fseek(fout, 0, SEEK_CUR);
    int c_offset = ftell(fout);
    fseek(fout, ((*iblock_id) + 6) * size_of_block, SEEK_SET);
    for(int i = 0; i < (size_of_block / 4); ++i) {
    	//如果索引结束,填充0
        int id;
        if ((*block_cnt) <= 0) id = 0;
        else {
            id = data_block_id++;
            (*block_cnt)--;
        }
        fwrite(&id, 4, 1, fout);
    }
    fseek(fout, c_offset, SEEK_SET);
    (*iblock_id)++;
}

//增加二级索引的索引块
void add_i2blocks(int *iblock_id, int *block_cnt, FILE *fout) {
    fseek(fout, 0, SEEK_CUR);
    int c_offset = ftell(fout);
    fseek(fout, ((*iblock_id) + 6) * size_of_block, SEEK_SET);
    iblock_id++;
    for(int i = 0; i < (size_of_block / 4); ++i) {
        //如果索引结束,填充0
        int id;
        if ((*block_cnt) <= 0) id = 0;
        else {
            id = *iblock_id;
            add_iblocks(iblock_id, block_cnt, fout);
        }
        fwrite(&id, 4, 1, fout);
    }
    fseek(fout, c_offset, SEEK_SET);
}

//增加三级索引的索引块
void add_i3blocks(int *iblock_id, int *block_cnt, FILE *fout) {
    fseek(fout, 0, SEEK_CUR);
    int c_offset = ftell(fout);
    fseek(fout, ((*iblock_id) + 6) * size_of_block, SEEK_SET);
    iblock_id++;
    for(int i = 0; i < (size_of_block / 4); ++i) {
        //如果索引结束,填充0
        int id;
        if ((*block_cnt) <= 0) id = 0;
        else {
            id = *iblock_id;
            add_i2blocks(iblock_id, block_cnt, fout);
        }
        fwrite(&id, 4, 1, fout);
    }
    fseek(fout, c_offset, SEEK_SET);
}

void modify_inodes(IN *inode, FILE *fout) {
	//文件数据块数目
    int block_cnt = (inode->size - 1) / 512 + 1;

    //直接索引
    for (int i = 0; i < N_DBLOCKS; ++i) {
    	if (inode->dblocks[i] == 0) break;
    	block_cnt--;
        inode->dblocks[i] = data_block_id++;
    }
    
    //一级索引
    if (block_cnt > 0) {
        int num_of_iblocks = (block_cnt - 1) / (size_of_block / 4) + 1;
        for (int i = 0; i < num_of_iblocks; ++i) {
            if (i >= N_IBLOCKS) break;
            inode->iblocks[i] = iblock_id;
            add_iblocks(&iblock_id, &block_cnt, fout);
        }
    }
    
    //二级索引
    if (block_cnt > 0) {
        inode->i2block = iblock_id;
        add_i2blocks(&iblock_id, &block_cnt, fout);
    }

    //三级索引
    if (block_cnt > 0) {
        inode->i3block = iblock_id;
        add_i3blocks(&iblock_id, &block_cnt, fout);
    }
}

int main(int argc, char *argv[]) {
	//文件系统名称转化
    strcpy(s_filename, argv[1]);
    strcpy(d_filename, argv[1]);
    strcpy(d_filename + strlen(argv[1]), "-defrag");

	//打开源文件系统
    FILE *fin = fopen(s_filename, "r");
    fseek(fin, 0, SEEK_END);
    file_system_bytes = ftell(fin);
    fseek(fin, 0, SEEK_SET);

	//新建去碎片化后的目标文件系统
    FILE *fout = fopen(d_filename, "w");
    fclose(fout);
    fout = fopen(d_filename, "r+");

	//读取boot和superblock,并写入目标文件系统
    SB *super_block = (SB *)malloc(512);
    fread(super_block, 512, 1, fin);
    fwrite(super_block, 512, 1, fout);
    fread(super_block, 512, 1, fin);
    fwrite(super_block, 512, 1, fout);
    size_of_block = super_block->size;
    IN *inode;
    inode = (IN *)malloc(sizeof(IN));

    int total_file_blocks = 0;

	//将20个inode写入目标文件系统
    for (int i = 0; i < 20; ++i) {
        fread(inode, sizeof(IN), 1, fin);
        total_file_blocks += (inode->size - 1) / 512 + 1;
        fwrite(inode, sizeof(IN), 1, fout);
    }
    
    //4块填充完inode后还剩512 * 4 - 100 * 20 = 48字节,并空出一块空闲块
    fwrite(inode, 48, 1, fout);
    fwrite(super_block, 512, 1, fout);
    fclose(fout);
    fseek(fin, 1024, SEEK_SET);
    
	//处理每一个inode,读取文件,并依次写入目标文件系统
    for (int i = 0; i < 20; ++i) {
        num_of_datablocks = 0;
        fread(inode, sizeof(IN), 1, fin);
        if (inode->nlink == 0) {
            continue;
        }
        
        read_and_write_file(super_block, inode, fin);
        
    }
    fclose(fin);

	//修改inode
    fout = fopen(d_filename, "r+");
    fseek(fout, 1024, SEEK_SET);
    data_block_id = 1;
    iblock_id = total_file_blocks + 1;
    
    for (int i = 0; i < 20; ++i) {
        int c_offset = (int)ftell(fout);
        fread(inode, sizeof(IN), 1, fout);
        if (inode->nlink == 0) {
            continue;
        }
        
        modify_inodes(inode, fout);
        
        //将修改后的inode写回目标文件系统
    	fseek(fout, c_offset, SEEK_SET);
    	fwrite(inode, sizeof(IN), 1, fout);
    }
    
    //填充空闲块
    fseek(fout, (iblock_id + 6) * size_of_block, SEEK_SET);
    int free_blocks = (file_system_bytes / 512) - iblock_id - 6;
    for (int i = 0; i < free_blocks; ++i) {
        fwrite(super_block, 512, 1, fout);
    }
    
    //修改superblock
    fseek(fout, 512, SEEK_SET);
    fread(super_block, 512, 1, fout);
    super_block->free_iblock = iblock_id;
    fseek(fout, 512, SEEK_SET);
    fwrite(super_block, 512, 1, fout);
    
    fclose(fout);
    free(super_block);
    free(inode);
    return 0;
}
