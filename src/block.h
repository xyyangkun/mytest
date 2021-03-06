/*
 * block.h
 *
 *  Created on: 2013-9-12
 *      Author: yangkun
 */

#ifndef BLOCK_H_
#define BLOCK_H_
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#ifndef TEST_RAM
#include <syslog.h>
#include "gtlog.h"
#endif
//在内存中建立文件测试
//#define TEST_RAM

//#define DEBUG_LOG
/*硬盘默认块大小512字节  这个值可以说是不能改的*/
#define BLOCKSIZE 512
#define BUFSIZE  400*1024
/*本程序最多能管理800天*/
#define MAXDAY 800
/*年循环队列中头的偏移*/
#define YEAR_OFFSET 8
/*天循环队列中头的偏移*/
#define DATE_OFFSET 8
#ifdef DEBUG_LOG
	/*如果测试程序逻辑，可以让天变短点，就一天只有60秒*/
	#define SECOFDAY (1*60)
#else
	#define SECOFDAY (24*3600)
#endif   /*DEBUG_LOG*/

typedef enum bool{
	true  = 1,
	false = 0
}bool;
enum block_type{
	year_block = 1,
	day_block  = 2,
	sec_block  =3,
	no_block =4
};

#define first_block 1
struct  seek_block{ int time; long long seek;}__attribute__ ((packed));
//年块
static const unsigned char year_head[8]={0x59,0x45,0x41,0x52,0xa5,0xa5,0xa5,0xa5};
struct year_queue{ unsigned int queue_size ,queue_head,queue_tail;};
struct year_block
{
	unsigned char year_head[8];
	struct year_queue year_queue_data;
	struct seek_block sekk_block_data[MAXDAY];
}__attribute__ ((packed));
//天块
static const unsigned char day_head[8]={0x4d,0x4f,0x4e,0x54,0x5a,0xa5,0xa5,0x5a};
struct day_queue { int index, size;};
struct day_block
{
	unsigned char day_head[8];
	struct day_queue day_queue_data;
	struct seek_block seek_block_data[SECOFDAY];
}__attribute__ ((packed));

//秒头
/*秒头*/
static const char framehead[8]={0x53,0x45,0x43,0x4f,0xa5,0x5a,0x5a,0xa5};
struct hd_frame
{
	char data_head[8];			/*帧数据头的标志位 0x5345434fa55a5aa5*/
	short frontIframe;			/*前一个I帧相对于本帧的偏移地址   如果前一帧在硬盘的最后面，这个值可能是负值*/
	short is_I;		/*本帧是不是I帧*/
	unsigned int size;			/*本帧视频数据块的大小*/
}__attribute__ ((packed));;
/*把前8个字符置成规定的值*/





/****************************************************************************
 * 硬盘读写
 ***************************************************************************/
#define HD_ERR_FD  -1			//硬盘文件描述符出错
#define HD_ERR_SEEK -2			//硬盘seek出错
#define HD_ERR_WRITE -3			//硬盘write出错
#define HD_ERR_OVER  -100  		//!!数据溢出缓冲区
#define HD_ERR_FULL  -101		//硬盘空间不路，不够写入最后一帧数据。
extern int dh_init();
extern int sda_write(char *buf, int bufsize, int buf_size, long long seek);
extern int sda_read (char *buf, int bufsize, int buf_size, long long seek);
extern int hd_getsize(long long *blocks);
enum opera_type{
	head = 0,
	tail = 1
};
extern int block_init();
extern int block_write(char *buf, int bufsize, int buf_size);
extern int block_read(char *buf, int bufsize ,long long seek, enum block_type *this_block_type);
extern int block_year_get();

#define BLOCK_ERR_EMPTY -4		//（年块天块中数据）为空，没有有效数据
#define BLOCK_ERR_FULL -5 		//（年块天块中数据）数据写满了，应该只有年块会写满
#define BLOCK_ERR_DATA_HEAD -6 	//（年块,天块,秒块中数据）数据块头错误
#define BLOCK_ERR_NULL -7		//传入指针参数为NULL
#define BLOCK_ERR_ZERO -8		//传入指针参数有效，但数据区全为0
#define BLOCK_ERR_DAY_SEC_MUT -9		//天块中一个秒块的位置有多个秒块对应。有可能的情况是1、一秒有多个I帧2、系统时间出错，向前走了
#define BLOCK_ERR_READ_TYPE -10	//读取数据块类型出错误
#define BLOCK_ERR_GET_FRAME -11	//获取视频帧错误

#define BLOCK_ERR_UNKNOW_TIME -1000		//未知道的时间错误：可能系统时间回到1970,也可能其它错误
#define BLOCK_ERR_DAY_PASS -1001		//今天过去了。让watch重启吧。
#define BLOCK_INFO_DAY_BLOCK_COVER -1002 //天块要被秒块占用了，要把此天块copy到day_data_bac中；\
										//	想读取早时间内容时，应该:1、先从年块中找到最新的天，再从天块中找到最新秒块的seek；
										//  2、从day_data_bac中找到最接近1中seek的块，并验证有效性！！
#define BLOCK_ERR_YEAR_PRINT -1003		//年块中天块seek_block输出时出错，不符合正常的逻辑

/*******************************************************************************
 * 获取系统时间
 *******************************************************************************/
int get_time();
/*******************************************************************************
 * 由数据的大小返回 此数据在硬盘上所占的块的数量
 *******************************************************************************/
#define get_block_num(buffsize) (( (buffsize) + BLOCKSIZE -1)/BLOCKSIZE )

#define DP(fmt...)	\
	do{\
		printf(fmt);\
		printf("function:[%s],line:%d: \n",__FUNCTION__,__LINE__);\
	}while(0)
#define BLOCK_ERR_READ_NEW_DISK  -200	//读的是新硬盘

#endif /* BLOCK_H_ */


