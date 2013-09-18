/*
 * block.c
 *
 *  Created on: 2013-9-12
 *      Author: yangkun
 */
#include "block.h"
enum get_type
{
	get_start = 1,
	get_rear   = 2
};
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
 * 根据传入 参数 返回队列中最后的时间，或最前的时间
 ***********************************************************/
int get_seek(struct  seek_block *block, int num,enum get_type type )
{
	int mark_begin = 0;
	int tmp = 0;
	if(type == get_start)
	{
		while(num>=0)
		{
			num--;
			if(block->time > tmp)
				tmp = block->time;
		}
		return tmp;
	}else if(type == get_rear)
	{
		while(num>=0)
		{
			num--;
			if(!mark_begin)
			{
				if(block->time > 0)
				{
					mark_begin =1;
					tmp = block->time;
				}
			}else
			{
				if(block->time > 0 && tmp < block->time)
					tmp = block->time;
			}
		}
		return tmp;
	}
	return -1;
}
/***********************************************************
 * 初始化块
 ***********************************************************/
int block_init()
{
	//year_data
	enum block_type this_block_type;
	if( block_read(year_data, sizeof(struct year_data) ,1,&this_block_type ) < 0)
	{
		printf("block_read\n");
		return -1;
	}
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
 * 自动识别年块，天块，秒块并读取
 * buf:要写入块的内容
 * bufsize:缓冲区的大小
 * seek 读取块的位置
 ***********************************************************/
int block_read(char *buf, int bufsize ,long long seek, enum block_type *this_block_type)
{
	int err;
	struct hd_frame *sec_block_head;
#ifdef TEST_RAM
	//先读第一块
	if( (err=hd_read(buf, bufsize,1, seek)) < 0 )
	{
	 printf("hd_read error\n");
	}
	*this_block_type = block_check(buf);
	switch ( *this_block_type )
	{
	case sec_block:
		sec_block_head = (struct hd_frame *)buf;
		if( (err=hd_read(buf+BLOCKSIZE ,bufsize- BLOCKSIZE, \
				(sec_block_head->size + sizeof(struct hd_frame) + BLOCKSIZE -1)/BLOCKSIZE - 1, seek+1)) < 0 )
		{
		 printf("hd_read error\n");
		}
		//秒块读到了
		//......
		break;
	case day_block:
		//从天块开始的第二块开始读 （天块大小-1）块数据
		if( (err=hd_read(buf+BLOCKSIZE ,bufsize- BLOCKSIZE, \
				(sizeof(struct day_block) + BLOCKSIZE -1)/BLOCKSIZE - 1, seek+1)) < 0 )
		{
		 printf("hd_read error\n");
		}
		//天块读到了
		//......
		break;
	case year_block:
		//从年块开始的第二块开始读 （年块大小-1）块数据
		if( (err=hd_read(buf+BLOCKSIZE ,bufsize- BLOCKSIZE, \
				(sizeof(struct year_block) + BLOCKSIZE -1)/BLOCKSIZE - 1, seek+1)) < 0 )
		{
		 printf("hd_read error\n");
		}
		//年块读到了
		//......
		break;
	default:
		return -1;
		break;
	}


#else
#endif//TEST_RAM
	return 0;
}
