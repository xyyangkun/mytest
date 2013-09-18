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
	if(lseek(hd_fd, seek*BLOCKSIZE, SEEK_SET)<0){
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
 *block_numҪ�����ٿ�
 *seek����һ�鿪ʼ
 ***********************************************************/
int hd_read(char *buf, int bufsize, int block_num, long long seek)
{
	//ÿ��512�ֽڣ������ж��ٿ顣
	int blocks;
#ifdef TEST_RAM
	if(hd_fd<0){
		return HD_ERR_FD;
	}
	if(lseek(hd_fd,  seek*BLOCKSIZE, SEEK_SET)<0){
		return HD_ERR_SEEK;
	}
#else
	//��ʵӲ�̴���

#endif
	//��������ռ�е�512�ֽڵĿ���
	blocks = ( block_num + BLOCKSIZE -1)/BLOCKSIZE;
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
 * �Ա����ݵ�ǰ8���ֽڣ��ж����ݿ�����
 ***********************************************************/
enum block_type block_check(char *buf)
{
	//�ж�ͷ����
	if(memcmp(buf, framehead , sizeof(framehead)) == 0)//���
	{
		return sec_block;
	}
	else if(memcmp(buf, day_head , sizeof(day_head)) == 0)//���
	{
		return day_block;
	}
	else if(memcmp(buf, year_head , sizeof(year_head)) == 0)//���
	{
		return 	year_block;
	}
	else
		return no_block;
}
/***********************************************************
 * ��ʼ����
 ***********************************************************/
int block_init()
{


	return 0;
}

/***********************************************************
 * ��飬��飬����д��
 * buf:Ҫд��������
 * bufsize:�������Ĵ�С
 * buf_size:Ҫд�����ݵĴ�С
 ***********************************************************/
int block_write(char *buf, int bufsize, int buf_size)
{
#ifdef TEST_RAM



#else
	//��ʵӲ�̴���
#endif
	return 0;
}
/***********************************************************
 * ��飬��飬���Ķ�ȡ
 * buf:Ҫд��������
 * bufsize:�������Ĵ�С
 * seek ��ȡ���λ��
 ***********************************************************/
int block_read(char *buf, int bufsize ,long long seek)
{
	int err;
	static char  block_head[512];
#ifdef TEST_RAM
	//�ȶ�һ��
	//printf("size of year_block:%d\n",(sizeof(struct year_block) + BLOCKSIZE -1)/BLOCKSIZE);
	//struct year_block *year_block_data=(struct year_block*)year_data;
	//�ȶ���һ��
	memset((void *)block_head, 0, sizeof(block_head));
	if( (err=hd_read(block_head, sizeof(block_head),1, first_block)) < 0 )
	{
	 printf("hd_read error\n");
	}
	switch ( block_check(block_head) )
	{
	case sec_block:
		memcpy(year_data, block_head, sizeof(block_head));
		//����鿪ʼ�ĵڶ��鿪ʼ�� ������С-1��������
		if( (err=hd_read(year_data+BLOCKSIZE ,sizeof(year_data)- BLOCKSIZE, \
				(sizeof(struct year_block) + BLOCKSIZE -1)/BLOCKSIZE - 1, first_block+1)) < 0 )
		{
		 printf("hd_read error\n");
		}
		//��������
		//......
		break;
	case day_block:
		memcpy(day_data, block_head, sizeof(block_head));
		//����鿪ʼ�ĵڶ��鿪ʼ�� ������С-1��������
		if( (err=hd_read(day_data+BLOCKSIZE ,sizeof(day_data)- BLOCKSIZE, \
				(sizeof(struct day_block) + BLOCKSIZE -1)/BLOCKSIZE - 1, first_block+1)) < 0 )
		{
		 printf("hd_read error\n");
		}
		//��������
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
