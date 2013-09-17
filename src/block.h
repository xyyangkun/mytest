/*
 * block.h
 *
 *  Created on: 2013-9-12
 *      Author: yangkun
 */

#ifndef BLOCK_H_
#define BLOCK_H_
#include <stdio.h>
#include <fcntl.h>
//在内存中建立文件测试
#define TEST_RAM

#define DEBUG_LOG
/*硬盘默认块大小512字节  这个值可以说是不能改的*/
#define BLOCKSIZE 512

/*本程序最多能管理800天*/
#define MAXDAY 800
/*年循环队列中头的偏移*/
#define YEAR_OFFSET 8
/*天循环队列中头的偏移*/
#define DATE_OFFSET 8

#ifdef DEBUG_LOG
	/*60*12/512=2*/
	#define YEAR_HEAD_BLOCK_SIZE 20
	#define DATE_HEAD_BLOCK_SIZE 2
/*测试程序逻辑时，就让硬盘有100000*512，也就50M左右那么大，4Mbit码率可以写100S，也就是一天多一点了*/
#define MAXBLOCKS 100000
#else
/*
 * 表示年的数据占20块，表示天的数据点2026块
 * 最大800天 20->800*12/512
 * 2026->24*3600*12/512
 */
	#define YEAR_HEAD_BLOCK_SIZE 20
	#define DATE_HEAD_BLOCK_SIZE 2026
/*160G的硬盘有这么多个块*/
#define MAXBLOCKS 312581808
#endif



//年块
static unsigned char year_head[8]={0x59,0x45,0x41,0x52,0xa5,0xa5,0xa5,0xa5};
struct year_queue_data{	unsigned int queue_size ,queue_head,queue_tail;};
struct year_block{ int time; long long seek;}__attribute__ ((packed));
//天块
static unsigned char day_head[8]={0x4d,0x4f,0x4e,0x54,0x5a,0xa5,0xa5,0x5a};
struct day_block_data{/*尾*/int index;int size;};
struct day_block{ int time; long long seek;}__attribute__ ((packed));

//秒头
/*秒头*/
static char framehead[8]={0x53,0x45,0x43,0x4f,0xa5,0x5a,0x5a,0xa5};
struct hd_frame
{
	char data_head[8];			/*帧数据头的标志位 0x5345434fa55a5aa5*/
	short frontIframe;			/*前一个I帧相对于本帧的偏移地址   如果前一帧在硬盘的最后面，这个值可能是负值*/
	short is_I;		/*本帧是不是I帧*/
	unsigned int size;			/*本帧视频数据块的大小*/
};
/*把前8个字符置成规定的值*/





/****************************************************************************
 * 硬盘读写
 ***************************************************************************/
#define HD_ERR_FD  -1//硬盘文件描述符出错
#define HD_ERR_SEEK -2//硬盘seek出错
#define HD_ERR_WRITE -3//硬盘write出错
#define HD_ERR_OVER  -100  //!!数据溢出缓冲区
extern int dh_init();
extern int hd_write(char *buf, int bufsize, int buf_size, long long seek);
extern int hd_read (char *buf, int bufsize, int buf_size, long long seek);
extern int hd_getsize(long long *blocks);
#endif /* BLOCK_H_ */
