/*
 * block.c
 *
 *  Created on: 2013-9-12
 *      Author: yangkun
 */
#include "block.h"
static int hd_fd;
static long long hd_blocks;
/**********************************************************
 * ��ʼ��Ӳ�̶�д����
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
 * дӲ��
 *
 *bufsize ���������ܴ�С
 *buf_size����������Ч���ݵĴ�С
 ***********************************************************/
int hd_write(char *buf, int bufsize, int buf_size, long long seek)
{
	//ÿ��512�ֽڣ������ж��ٿ顣
	int blocks;
#ifdef TEST_RAM
	if(hd_fd<0){
		return HD_ERR_FD;
	}
	if(lseek(hd_fd, 0, SEEK_SET)<0){
		return HD_ERR_SEEK;
	}
#else
	//��ʵӲ�̴���

#endif
	//��������ռ�е�512�ֽڵĿ���
	blocks = ( buf_size + BLOCKSIZE -1)/BLOCKSIZE;
	//�ж�д�������ǲ��ǹ���
	if(blocks*BLOCKSIZE>bufsize)
		return HD_ERR_OVER;
#ifdef TEST_RAM
	//��ʼд
	if(write(hd_fd, buf, blocks*BLOCKSIZE) < 0)
		return HD_ERR_WRITE;
#else
	//��ʵӲ�̴���

#endif
	return 0;
}
/***********************************************************
 * ��Ӳ��
 *bufsize ���������ܴ�С
 *buf_size����������Ч���ݵĴ�С
 ***********************************************************/
int hd_read(char *buf, int bufsize, int buf_size, long long seek)
{
	//ÿ��512�ֽڣ������ж��ٿ顣
	int blocks;
#ifdef TEST_RAM
	if(hd_fd<0){
		return HD_ERR_FD;
	}
	if(lseek(hd_fd, 0, SEEK_SET)<0){
		return HD_ERR_SEEK;
	}
#else
	//��ʵӲ�̴���

#endif
	//��������ռ�е�512�ֽڵĿ���
	blocks = ( buf_size + BLOCKSIZE -1)/BLOCKSIZE;
	//�ж϶��������ǲ��ǹ���
	if(blocks*BLOCKSIZE>bufsize)
		return HD_ERR_OVER;
#ifdef TEST_RAM
	//��ʼ��
	if(read(hd_fd, buf, blocks*BLOCKSIZE) < 0)
		return HD_ERR_WRITE;
#else
	//��ʵӲ�̴���

#endif
	return 0;
}
/***********************************************************
 * ��ȡӲ�̴�С
 *������blocksӲ�̵Ŀ���
 * ����0�ɹ�
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
	//��ʵӲ�̴���

#endif
	return 0;
}



/******************************************************************************
 * ���´�������Կ�д��ģ���飬��飬��飬��ͬ�Ĵ���
 *
 ******************************************************************************/




/***********************************************************
 * ��д��
 * buf:Ҫд��������
 * bufsize:�������Ĵ�С
 * buf_size:Ҫд�����ݵĴ�С
 ***********************************************************/
int block_write(char *buf, int bufsize, int buf_size)
{

	return 0;
}
