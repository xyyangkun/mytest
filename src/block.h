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
//���ڴ��н����ļ�����
#define TEST_RAM

#define DEBUG_LOG
/*Ӳ��Ĭ�Ͽ��С512�ֽ�  ���ֵ����˵�ǲ��ܸĵ�*/
#define BLOCKSIZE 512

/*����������ܹ���800��*/
#define MAXDAY 800
/*��ѭ��������ͷ��ƫ��*/
#define YEAR_OFFSET 8
/*��ѭ��������ͷ��ƫ��*/
#define DATE_OFFSET 8

#ifdef DEBUG_LOG
	/*60*12/512=2*/
	#define YEAR_HEAD_BLOCK_SIZE 20
	#define DATE_HEAD_BLOCK_SIZE 2
/*���Գ����߼�ʱ������Ӳ����100000*512��Ҳ��50M������ô��4Mbit���ʿ���д100S��Ҳ����һ���һ����*/
#define MAXBLOCKS 100000
#else
/*
 * ��ʾ�������ռ20�飬��ʾ������ݵ�2026��
 * ���800�� 20->800*12/512
 * 2026->24*3600*12/512
 */
	#define YEAR_HEAD_BLOCK_SIZE 20
	#define DATE_HEAD_BLOCK_SIZE 2026
/*160G��Ӳ������ô�����*/
#define MAXBLOCKS 312581808
#endif



//���
static unsigned char year_head[8]={0x59,0x45,0x41,0x52,0xa5,0xa5,0xa5,0xa5};
struct year_queue_data{	unsigned int queue_size ,queue_head,queue_tail;};
struct year_block{ int time; long long seek;}__attribute__ ((packed));
//���
static unsigned char day_head[8]={0x4d,0x4f,0x4e,0x54,0x5a,0xa5,0xa5,0x5a};
struct day_block_data{/*β*/int index;int size;};
struct day_block{ int time; long long seek;}__attribute__ ((packed));

//��ͷ
/*��ͷ*/
static char framehead[8]={0x53,0x45,0x43,0x4f,0xa5,0x5a,0x5a,0xa5};
struct hd_frame
{
	char data_head[8];			/*֡����ͷ�ı�־λ 0x5345434fa55a5aa5*/
	short frontIframe;			/*ǰһ��I֡����ڱ�֡��ƫ�Ƶ�ַ   ���ǰһ֡��Ӳ�̵�����棬���ֵ�����Ǹ�ֵ*/
	short is_I;		/*��֡�ǲ���I֡*/
	unsigned int size;			/*��֡��Ƶ���ݿ�Ĵ�С*/
};
/*��ǰ8���ַ��óɹ涨��ֵ*/





/****************************************************************************
 * Ӳ�̶�д
 ***************************************************************************/
#define HD_ERR_FD  -1//Ӳ���ļ�����������
#define HD_ERR_SEEK -2//Ӳ��seek����
#define HD_ERR_WRITE -3//Ӳ��write����
#define HD_ERR_OVER  -100  //!!�������������
extern int dh_init();
extern int hd_write(char *buf, int bufsize, int buf_size, long long seek);
extern int hd_read (char *buf, int bufsize, int buf_size, long long seek);
extern int hd_getsize(long long *blocks);
#endif /* BLOCK_H_ */
