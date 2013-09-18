/*
 * block.c
 *
 *  Created on: 2013-9-12
 *      Author: yangkun
 */
#include "block.h"
static int hd_fd;
static long long hd_blocks;
static char year_data[(sizeof(struct year_block) + BLOCKSIZE -1)/BLOCKSIZE*BLOCKSIZE];
static char day_data[(sizeof(struct day_block) + BLOCKSIZE -1)/BLOCKSIZE*BLOCKSIZE];
/**********************************************************
 * 初始化硬盘读写功能
 **********************************************************/
int dh_init()
{
#ifdef TEST_RAM
	hd_fd = open("/mnt/ramfs/a.img",O_RDWR|O_SYNC);
	if(hd_fd<0)
	{
		perror("open");
		return HD_ERR_FD;
	}

#endif
	return 0;
}
/***********************************************************
 * 写硬盘
 *
 *bufsize 缓冲区的总大小
 *buf_size缓冲区中有效数据的大小
 ***********************************************************/
int hd_write(char *buf, int bufsize, int buf_size, long long seek)
{
	//每块512字节，计算有多少块。
	int blocks;
#ifdef TEST_RAM
	if(hd_fd<0){
		return HD_ERR_FD;
	}
	if(lseek(hd_fd, seek*BLOCKSIZE, SEEK_SET)<0){
		return HD_ERR_SEEK;
	}
#else
	//真实硬盘代码

#endif
	//计算数据占有的512字节的块数
	blocks = ( buf_size + BLOCKSIZE -1)/BLOCKSIZE;
	//判断写缓冲区是不是够大
	if(blocks*BLOCKSIZE>bufsize)
		return HD_ERR_OVER;
#ifdef TEST_RAM
	//开始写
	if(write(hd_fd, buf, blocks*BLOCKSIZE) < 0)
		return HD_ERR_WRITE;
#else
	//真实硬盘代码

#endif
	return 0;
}
/***********************************************************
 * 读硬盘
 *bufsize 缓冲区的总大小
 *block_num要读多少块
 *seek从哪一块开始
 ***********************************************************/
int hd_read(char *buf, int bufsize, int block_num, long long seek)
{
	//每块512字节，计算有多少块。
	int blocks;
#ifdef TEST_RAM
	if(hd_fd<0){
		return HD_ERR_FD;
	}
	if(lseek(hd_fd,  seek*BLOCKSIZE, SEEK_SET)<0){
		return HD_ERR_SEEK;
	}
#else
	//真实硬盘代码

#endif
	//计算数据占有的512字节的块数
	blocks = ( block_num + BLOCKSIZE -1)/BLOCKSIZE;
	//判断读缓冲区是不是够大
	if(blocks*BLOCKSIZE>bufsize)
		return HD_ERR_OVER;
#ifdef TEST_RAM
	//开始读
	if(read(hd_fd, buf, blocks*BLOCKSIZE) < 0)
		return HD_ERR_WRITE;
#else
	//真实硬盘代码

#endif
	return 0;
}
/***********************************************************
 * 获取硬盘大小
 *参数：blocks硬盘的块数
 * 返回0成功
 ***********************************************************/
int hd_getsize(long long *blocks)
{
#ifdef TEST_RAM
	int begin,end;
	if( (begin=lseek(hd_fd, 0, SEEK_SET)) < 0 )
		return HD_ERR_SEEK;
	if( (end=  lseek(hd_fd, 0, SEEK_END)) < 0 )
		return HD_ERR_SEEK;
	if( (end-begin)<0 )
		return HD_ERR_SEEK;
	else
	{
		hd_blocks = (end-begin)/512;
		*blocks = hd_blocks;
	}
#else
	//真实硬盘代码

#endif
	return 0;
}



/******************************************************************************
 * 以下代码是针对块写入的：年块，天块，秒块，不同的处理
 *
 ******************************************************************************/
/***********************************************************
 * 对比数据的前8个字节，判断数据块类型
 ***********************************************************/
enum block_type block_check(char *buf)
{
	//判断头类型
	if(memcmp(buf, framehead , sizeof(framehead)) == 0)//秒块
	{
		return sec_block;
	}
	else if(memcmp(buf, day_head , sizeof(day_head)) == 0)//天块
	{
		return day_block;
	}
	else if(memcmp(buf, year_head , sizeof(year_head)) == 0)//年块
	{
		return 	year_block;
	}
	else
		return no_block;
}
/***********************************************************
 * 初始化块
 ***********************************************************/
int block_init()
{


	return 0;
}

/***********************************************************
 * 年块，天块，秒块的写入
 * buf:要写入块的内容
 * bufsize:缓冲区的大小
 * buf_size:要写入内容的大小
 ***********************************************************/
int block_write(char *buf, int bufsize, int buf_size)
{
#ifdef TEST_RAM



#else
	//真实硬盘代码
#endif
	return 0;
}
/***********************************************************
 * 年块，天块，秒块的读取
 * buf:要写入块的内容
 * bufsize:缓冲区的大小
 * seek 读取块的位置
 ***********************************************************/
int block_read(char *buf, int bufsize ,long long seek)
{
	int err;
	static char  block_head[512];
#ifdef TEST_RAM
	//先读一块
	//printf("size of year_block:%d\n",(sizeof(struct year_block) + BLOCKSIZE -1)/BLOCKSIZE);
	//struct year_block *year_block_data=(struct year_block*)year_data;
	//先读第一块
	memset((void *)block_head, 0, sizeof(block_head));
	if( (err=hd_read(block_head, sizeof(block_head),1, first_block)) < 0 )
	{
	 printf("hd_read error\n");
	}
	switch ( block_check(block_head) )
	{
	case sec_block:
		memcpy(year_data, block_head, sizeof(block_head));
		//从年块开始的第二块开始读 （年块大小-1）块数据
		if( (err=hd_read(year_data+BLOCKSIZE ,sizeof(year_data)- BLOCKSIZE, \
				(sizeof(struct year_block) + BLOCKSIZE -1)/BLOCKSIZE - 1, first_block+1)) < 0 )
		{
		 printf("hd_read error\n");
		}
		//年块读到了
		//......
		break;
	case day_block:
		memcpy(day_data, block_head, sizeof(block_head));
		//从年块开始的第二块开始读 （年块大小-1）块数据
		if( (err=hd_read(day_data+BLOCKSIZE ,sizeof(day_data)- BLOCKSIZE, \
				(sizeof(struct day_block) + BLOCKSIZE -1)/BLOCKSIZE - 1, first_block+1)) < 0 )
		{
		 printf("hd_read error\n");
		}
		//年块读到了
		//......
		break;
	case year_block:
		break;
	default:
		return -1;
		break;
	}


#else
#endif//TEST_RAM
	return 0;
}
