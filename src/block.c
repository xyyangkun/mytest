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
 * ���ݴ��� ���� ���ض���������ʱ�䣬����ǰ��ʱ��
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
 * ��ʼ����
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
 * �Զ�ʶ����飬��飬��鲢��ȡ
 * buf:Ҫд��������
 * bufsize:�������Ĵ�С
 * seek ��ȡ���λ��
 ***********************************************************/
int block_read(char *buf, int bufsize ,long long seek, enum block_type *this_block_type)
{
	int err;
	struct hd_frame *sec_block_head;
#ifdef TEST_RAM
	//�ȶ���һ��
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
		//��������
		//......
		break;
	case day_block:
		//����鿪ʼ�ĵڶ��鿪ʼ�� ������С-1��������
		if( (err=hd_read(buf+BLOCKSIZE ,bufsize- BLOCKSIZE, \
				(sizeof(struct day_block) + BLOCKSIZE -1)/BLOCKSIZE - 1, seek+1)) < 0 )
		{
		 printf("hd_read error\n");
		}
		//��������
		//......
		break;
	case year_block:
		//����鿪ʼ�ĵڶ��鿪ʼ�� ������С-1��������
		if( (err=hd_read(buf+BLOCKSIZE ,bufsize- BLOCKSIZE, \
				(sizeof(struct year_block) + BLOCKSIZE -1)/BLOCKSIZE - 1, seek+1)) < 0 )
		{
		 printf("hd_read error\n");
		}
		//��������
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
