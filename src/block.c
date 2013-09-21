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
//Ӳ���ϵ��ܿ�����Ҳ������
static long long hd_blocks;
//Ӳ���ϵ�ǰ����λ�ã���дʱ���õ�
static long long hd_current_day_seek=0;
//��ǰ���Ӧ��д���λ��
static long long hd_current_sec_seek = 0;
static char year_data[(sizeof(struct year_block) + BLOCKSIZE -1)/BLOCKSIZE*BLOCKSIZE];
static char day_data_bac[(sizeof(struct day_block) + BLOCKSIZE -1)/BLOCKSIZE*BLOCKSIZE];
static char day_data[(sizeof(struct day_block) + BLOCKSIZE -1)/BLOCKSIZE*BLOCKSIZE];
static long long first_seek=first_block+sizeof(year_data)+sizeof(day_data_bac);
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
	char buf[512]={0};
	int err;
#ifndef YEARBLOCKTEST
	//�ӵ�0�鿪ʼ��1�飬�ô�С512�ֽڵĻ�����
	if( (err=hd_read(buf, sizeof(buf),1, 0)) < 0 )
	{
	 printf("hd_read error\n");
	}
	if(memcmp(buf+256, "gtalarm", 7)== 0)
#endif //YEARBLOCKTEST
	{
		printf("this is a new disk\n");
		struct year_block *yeardata=(struct year_block *) year_data ;
		memset(buf, 0, sizeof(buf));
		memcpy(buf, "gtalarm", 7);
		if( (err=hd_write(buf, sizeof(buf), sizeof(buf), 0)) < 0 )
		{
		 printf("hd_read error\n");
		}
		//д���ͷ
		struct day_block *daydata=(struct day_block *) day_data ;
		memset(year_data , 0 , sizeof(year_data));
		memcpy( yeardata->year_head, day_head, sizeof(day_head) ); //����ͷ
		//yeardata->year_queue_data.queue_size=0;
		if(hd_write(year_data, sizeof(year_data), sizeof(year_data), first_block) < 0)
		{
			printf("hd_write error\n");
			return -1;
		}
		//д���ͷ
		memset(day_data , 0 , sizeof(day_data));
		memcpy( day_data, day_data, sizeof(day_data) ); //����ͷ
		if(hd_write(day_data, sizeof(day_data), sizeof(day_data), first_seek) < 0)
		{
			printf("hd_write error\n");
			return -1;
		}
	}
	//year_data
	enum block_type this_block_type;
	if( block_read((char *)year_data, sizeof(year_data) , first_block, &this_block_type ) < 0)
	{
		printf("block_read\n");
		return -1;
	}
	//day_data
	if( block_read((char *)day_data, sizeof(day_data) , first_seek, &this_block_type ) < 0)
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
	 return err;
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
		 return err;
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
		 return err;
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
		 return err;
		}
		//��������
		//......
		break;
	default:
		printf("read block head error\n");
		return -1;
		break;
	}


#else
#endif//TEST_RAM
	return 0;
}







/***********************************************************
*���ܣ���ȡyear block�洢������λ��
*
 ***********************************************************/
int block_year_get(struct  seek_block *block,enum opera_type get_type)
{
	struct year_block *yearblock = NULL;
	//block�������Ƿ�Ϸ�
	if(block == NULL  )
		return BLOCK_ERR_NULL;
	yearblock = (struct year_block *)year_data;
	if(get_type == get_start)
		memcpy(block, &(yearblock->sekk_block_data[yearblock->year_queue_data.queue_head]),\
					sizeof(struct seek_block));
	else
	memcpy(block, &(yearblock->sekk_block_data[yearblock->year_queue_data.queue_tail]) ,\
			sizeof(struct seek_block));
}
/***********************************************************
*����:�����������¿飨ֻ�����д��Ŀ飩
*
 ***********************************************************/
int block_year_add(struct  seek_block *block,enum opera_type get_type)
{
	struct year_block *yearblock  = (struct year_block *)year_data;;
	//block�������Ƿ�Ϸ�
	if(block == NULL )
			return BLOCK_ERR_NULL;
	if(block->time==0 || block->seek==0)
			return BLOCK_ERR_ZERO;
	//�ж϶������Ƿ��пռ�
	if(yearblock->year_queue_data.queue_tail+1 == yearblock->year_queue_data.queue_head)
	{
#ifdef YEARBLOCKTEST
		printf("head:%d\n",yearblock->year_queue_data.queue_head);
		printf("tail:%d\n",yearblock->year_queue_data.queue_tail);
		printf("size:%d\n",yearblock->year_queue_data.queue_size);
#endif
		printf("year block full\n");
		return BLOCK_ERR_FULL;//����
	}
	memcpy( &(yearblock->sekk_block_data[yearblock->year_queue_data.queue_tail]) ,\
			block, sizeof(struct seek_block));
	yearblock->year_queue_data.queue_tail = (yearblock->year_queue_data.queue_tail+1)%MAXDAY;
	yearblock->year_queue_data.queue_size++;
	//д������
	if(hd_write(year_data, sizeof(year_data), sizeof(year_data), first_block) < 0)
	{
		printf("hd_write error\n");
		return HD_ERR_WRITE;
	}
	return 0;
}
/***********************************************************
*���ܣ��������ɾ���飨ֻɾ�����ϵĿ飬��������������ɾ����
*
 ***********************************************************/
int block_year_del(enum opera_type get_type)
{
	struct year_block *yearblock = NULL;
	yearblock = (struct year_block *)year_data;
	if(yearblock->year_queue_data.queue_tail == yearblock->year_queue_data.queue_head)
	{
		printf("emputy\n");
		return BLOCK_ERR_EMPTY;//����
	}
	memset( &yearblock->sekk_block_data[yearblock->year_queue_data.queue_head],0 ,sizeof(struct  seek_block) );
	yearblock->year_queue_data.queue_head = (yearblock->year_queue_data.queue_head+1)%MAXDAY;
	yearblock->year_queue_data.queue_size--;
	//д������
	if(hd_write(year_data, sizeof(year_data), sizeof(year_data), first_block) < 0)
	{
		printf("hd_write error\n");
		return HD_ERR_WRITE;
	}
	return 0;
}
//�����Ķ�д�����߼��Ĳ���
int test_year_data()
{
#ifdef YEARBLOCKTEST
	//������0->MAXDAY֮��������������ʹ����е����ݴﵽ������䡣������ݶ��˵���del,���˵�add
	int i,times;
	struct year_block *yearblock = (struct year_block *)year_data;
	static struct  seek_block block;
	static enum opera_type get_type;
	printf("head:%d\n",yearblock->year_queue_data.queue_head);
	printf("tail:%d\n",yearblock->year_queue_data.queue_tail);
	printf("size:%d\n",yearblock->year_queue_data.queue_size);
	while(1)//�⵽������Ϊֹ
	{
		int seed = rand()%MAXDAY;
		//1����������
		printf("seed:queue_size:%d\t%d\n",seed,yearblock->year_queue_data.queue_size);
		if(seed == yearblock->year_queue_data.queue_size )
		{
			printf("eque\n");
			continue;
		}
		else if(seed > yearblock->year_queue_data.queue_size )
		{
			times = seed - yearblock->year_queue_data.queue_size;
			printf("year add:%d\n",times);
			for(i=0; i < times; i++)
			{
				block.seek=rand();
				block.time=rand()/*%MAXDAY+1*/ /*+1��ֹ����0*/;
				if(block_year_add(&block,get_type) < 0)
				{
					printf("block year add err\n and exit");
					return -1;
				}
			}
		}else
		{
			times = yearblock->year_queue_data.queue_size - seed ;
			printf("year del:%d\n",times);
			for(i=0; i < times; i++)
			{
				if(block_year_del(get_type) < 0)
				{
					printf("block year del err\n and exit");
					return -1;
				}
			}
		}
		//2����֤���ݣ�
		//printf("block head time:%d\nblock tail time:%d\n",\
				yearblock->sekk_block_data[yearblock->year_queue_data.queue_head].time,\
				yearblock->sekk_block_data[yearblock->year_queue_data.queue_tail].time);
		//if ( yearblock->year_queue_data.queue_size < 6 )//С���������������
		{
			int j;
			//printf("\n");
			//printf("\tsize:%d\n",yearblock->year_queue_data.queue_size);
			//�����в�Ϊ0������Ϊ0������߼��ϵĴ���
			for( j = 0; j < yearblock->year_queue_data.queue_size; j++)
			{
				//printf("time[%d]:\t%d\n",(yearblock->year_queue_data.queue_head+j)%MAXDAY, \
						yearblock->sekk_block_data[(yearblock->year_queue_data.queue_head+j)%MAXDAY].time);
				if( yearblock->sekk_block_data[(yearblock->year_queue_data.queue_head+j)%MAXDAY].time == 0)
				{
					printf("logic wrong 1 and exit\n");
					exit(1);
				}
			}
			//���ڶ����е�����Ӧ��ȫ��Ϊ0����Ϊ0��������߼�����
			for( j = 0 ; j < (MAXDAY-yearblock->year_queue_data.queue_size); j++)
			{
				//printf("time[%d]:\t%d\n",(yearblock->year_queue_data.queue_tail+j)%MAXDAY, \
						yearblock->sekk_block_data[(yearblock->year_queue_data.queue_tail+j)%MAXDAY].time);
				if( yearblock->sekk_block_data[(yearblock->year_queue_data.queue_tail+j)%MAXDAY].time != 0)
				{
					printf("logic wrong 2 and exit\n");
					exit(1);
				}
			}
		}

	}
#endif//YEARBLOCKTEST
}
/***********************************************************
*����:�����������¿飬ÿ��һ���루֡����д��ʱ������¿顣
*
 ***********************************************************/
int block_day_add(struct  seek_block *block)
{
	struct day_block *daydata = (struct day_block *)day_data;
	int index;//����ĵڶ�����
	static int tmp = 0;
	//block�������Ƿ�Ϸ�
	if(block == NULL )
			return BLOCK_ERR_NULL;
	if(block->time==0 || block->seek==0)
			return BLOCK_ERR_ZERO;
	//memcmp(daydata, day_head, 8)!=0,
	if(*(long long *)daydata != *(long long *)day_head)
	{
		return BLOCK_ERR_DATA_HEAD;
	}
	//��������
	index = block->time%SECOFDAY;
	if(daydata->seek_block_data[index].seek !=0 )
		return BLOCK_ERR_DAY_SEC_MUT;//��ǰ�����λ����ֵ�ˡ�
	else
		memcpy(&daydata->seek_block_data[index], block, sizeof(struct  seek_block));


	//ÿtmp == x��д��һ����顣  hd_current_day_seek
	tmp++;
	if(tmp == 300)
	{
		tmp = 0;
		enum block_type this_block_type;
		//day_data
		if( block_read((char *)day_data, sizeof(day_data) , hd_current_day_seek, &this_block_type ) < 0)
		{
			printf("block_read\n");
			return -1;
		}
	}


}

/***********************************************************
*����:��Ӳ����д�����ݡ�
*
 ***********************************************************/
int write_disk()
{
	struct year_block *yearblock =  (struct year_block *)year_data;
	struct  seek_block block;
	enum opera_type get_type;
	enum block_type this_block_type;
	int err;
	//��������ҵ���ǰ����λ��
	if(yearblock->year_queue_data.queue_size ==0)//��Ӳ��
	{
		block.seek = first_seek;
		block.time = time(NULL);
		//��������һ��
		if( ( err=block_year_add(&block, get_type) ) < 0)
			return err;
		hd_current_day_seek = first_seek;
		memset(day_data, 0, sizeof(day_data));
		hd_current_sec_seek = first_seek+sizeof(day_data);
		goto write_sec;
	}
	get_type = head;
	err = block_year_get(&block,get_type);
	if(err < 0)
	{
		printf("block_year_get err\n");
	}
	hd_current_day_seek = block.seek;
	//��ȡ��������
	err = block_read(day_data, sizeof(day_data) ,hd_current_day_seek, &this_block_type);
	if(err < 0)
		return err;
	if(this_block_type != day_block)
	{
		printf("read type err!!!\n");
		return BLOCK_ERR_READ_TYPE;
	}
	//��������ҵ���ǰ����λ��
			/*�˴�Ӧ���Ƕ���ģ�ֱ�Ӷ�д�ͺ��ˡ�*/
	//д����
	write_sec:
	while(1)
	{
		//1��ϵͳʱ�䣺\
					a�����ڽ��죨ͨ���������\
					b��������ˣ�1���������2�����������ڽ����23��59��59.999999�����еģ�������:���ڴ��е����д��Ӳ�̣�memset(day_data)\
					c��ϵͳʱ��Ƚ���0����ǰ�ˣ�\
											A����ǰ������20���ӣ���Ϊ��ϵͳ��ʱ���󣬻��������󣬴�ʱ��Ӧ��д�����Ŀ飬����������\
											B����ǰ����20���ӣ�������������󣿣�����
		//2���ж�Ӳ���Ƿ����� \
						a��������hd_current_day_seek��hd_current_sec_seek��ֵ\
						b��δ��

		//3���ж���һ��Ҫд������ݵ�λ���Ƿ��ǿ�����(��ʵֻ�����˺��Ӧ���ж�)��  \
						a���ǿ����� \
						b�����ǿ�����:  \
									A�������:��day_data_bac���������λ��\
									B������飺�������day_data_bac�У�ͬʱд�뵽��Ӧ��λ��

		//д����
	}
	return 0;
}
