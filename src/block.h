/*
 * block.h
 *
 *  Created on: 2013-9-12
 *      Author: yangkun
 */

#ifndef BLOCK_H_
#define BLOCK_H_
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




extern media_source_t media;
#define BUFFER_SIZE 400*1024 //最大4M，但个帧应该不会超过100K





/******************************************************************
 * 年块
 * *****************************************************************/
struct year_block{ int time; long long seek;}__attribute__ ((packed));






#endif /* BLOCK_H_ */
