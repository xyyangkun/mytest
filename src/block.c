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
static char year_data[    get_block_num( sizeof( struct year_block ) ) *BLOCKSIZE ];
static char day_data_bac[ get_block_num( sizeof( struct day_block )  ) *BLOCKSIZE ];
static char day_data[     get_block_num( sizeof( struct day_block )  ) *BLOCKSIZE ];
static char frame_buff[BUFSIZE];
const static long long first_seek=first_block+ \
		get_block_num(sizeof(year_data)+sizeof(day_data_bac));


static const struct year_block *yearblock =  (struct year_block *)year_data;
static const struct day_block *daydata    =  (struct day_block  *)day_data;


static char *hd_frame_buff=NULL;

#ifndef TEST_RAM
#include "media_api.h"
#include "mshmpool.h"
#include "gtlog.h"

#define MEDIA_VIDEO		0x01		//��Ƶ����
#define MEDIA_AUDIO	0x02		//��Ƶ����

struct NCHUNK_HDR {	//avi��ʽ�����ݿ�ͷ��־�ṹ
#define IDX1_VID  		0x63643030	//AVI����Ƶ�����
#define IDX1_AID  		0x62773130	//AVI����Ƶ���ı��
	unsigned long  chk_id;
	unsigned long  chk_siz;
};
typedef struct{
    ///ѹ�������Ƶ֡
    ///ʹ������ṹʱҪ�ȷ���һ���󻺳���,Ȼ�󽫱��ṹ��ָ��ָ�򻺳���

#define MEDIA_VIDEO		0x01		//��Ƶ����
#define MEDIA_AUDIO	0x02		//��Ƶ����

#define FRAMETYPE_I		0x0		// frame flag - I Frame
#define FRAMETYPE_P		0x1		// frame flag - P Frame
#define FRAMETYPE_B		0x2
#define FRAMETYPE_PCM	0x5		// frame flag - Audio Frame

	struct timeval           tv;			   ///<���ݲ���ʱ��ʱ���
	unsigned long	           channel;	          ///<ѹ��ͨ��
	unsigned short           media;		   ///<media type ��Ƶ����Ƶ
	unsigned short           type;		          ///<frame type	I/P/����...
	long                          len;	                 ///<frame_buf�е���Ч�ֽ���
	struct NCHUNK_HDR chunk;                ///<���ݿ�ͷ��־��Ŀǰʹ��avi��ʽ
	unsigned char            frame_buf[4];    ///<��ű�������Ƶ���ݵ���ʼ��ַ
}enc_frame_t;

media_source_t media;
/*******************************************************************************
 * ��ʼ��Ӳ��
 *******************************************************************************/
extern int init_sda();
/*******************************************************************************
 * ��ȡ��дӲ�̺������ļ�������
 *******************************************************************************/
extern int get_hd_fd();

/********************************************************************************
 * ��Ӳ�����ݣ�  ����32�飬��ֿ���
 * fd,Ӳ���ļ��������� seekҪ�����ݿ�ʼ�Ŀ飬  blk_num,��������� read_buf������
 * ����ֵ�������ֽ����� -1ʧ��
 ********************************************************************************/
extern int hd_read(int fd, long long  seek, unsigned int blk_num, char *read_buf,unsigned int buf_size);
/********************************************************************************
 * дӲ�����ݣ�  ����32�飬��ֿ���
 * fd,Ӳ���ļ��������� seekҪ�����ݿ�ʼ�Ŀ飬  blk_num,��������� read_buf������
 * ����ֵ��д���ֽ����� -1ʧ��
 ********************************************************************************/
extern int hd_write(int fd, long long  seek, unsigned int blk_num, char *write_buf,unsigned int buf_size);
#define MAXREADBLOCK 32
static int get_frame();
#endif
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
#else
	 init_sda();
#endif
	return 0;
}
/***********************************************************
 * дӲ��
 *
 *bufsize ���������ܴ�С
 *buf_size����������Ч���ݵĴ�С
 ***********************************************************/
int sda_write(char *buf, int bufsize, int buf_size, long long seek)
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
	blocks = get_block_num( buf_size );
	//�ж�д�������ǲ��ǹ���
	if(blocks*BLOCKSIZE>bufsize)
		return HD_ERR_OVER;
#ifdef TEST_RAM
	//��ʼд
	if(write(hd_fd, buf, blocks*BLOCKSIZE) < 0)
		return HD_ERR_WRITE;
#else
	//��ʵӲ�̴���
	if( hd_write(get_hd_fd(), seek, blocks,  buf, blocks*BLOCKSIZE) < 0)
		return -1;
#endif
	return 0;
}
/***********************************************************
 * ��Ӳ��
 *bufsize ���������ܴ�С
 *block_numҪ�����ٿ�
 *seek����һ�鿪ʼ
 ***********************************************************/
int sda_read(char *buf, int bufsize, int block_num, long long seek)
{
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
	//�ж϶��������ǲ��ǹ���
	if(block_num*BLOCKSIZE>bufsize)
		return HD_ERR_OVER;
#ifdef TEST_RAM
	//��ʼ��
	if(read(hd_fd, buf, blocks*BLOCKSIZE) < 0)
		return HD_ERR_WRITE;
#else
	//��ʵӲ�̴���
	if( hd_read(get_hd_fd(), seek, block_num,  buf, block_num*BLOCKSIZE) < 0)
		return -1;
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
	//bug fix!!!!!!
	*blocks = hd_blocks = 312581808/144;//160G
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
	if( (err=sda_read(buf, sizeof(buf),1, 0)) < 0 )
	{
	 gtlogerr("hd_read error\n");
	}
	if(memcmp(buf+256, "gtalarm", 7)== 0)
#endif //YEARBLOCKTEST
	{
		printf("this is a new disk!!!!\n");
		struct year_block *yeardata=(struct year_block *) year_data ;
		memset(buf, 0, sizeof(buf));
		memcpy(buf, "gtalarm", 7);
		if( (err=sda_write(buf, sizeof(buf), sizeof(buf), 0)) < 0 )
		{
			gtlogerr("hd_read error\n");
		}
		//д���ͷ
		struct day_block *daydata=(struct day_block *) day_data ;
		memset(year_data , 0 , sizeof(year_data));
		memcpy( yeardata->year_head, year_head, sizeof(year_head) ); //����ͷ
		//yeardata->year_queue_data.queue_size=0;
		if(sda_write(year_data, sizeof(year_data), sizeof(year_data), first_block) < 0)
		{
			gtlogerr("hd_write error\n");
			return -1;
		}
		//д���ͷ
		memset(day_data , 0 , sizeof(day_data));
		memcpy( day_data, day_head, sizeof(day_head) ); //����ͷ
		if(sda_write(day_data, sizeof(day_data), sizeof(day_data), first_seek) < 0)
		{
			gtlogerr("hd_write error\n");
			return -1;
		}
	}
	//year_data
	enum block_type this_block_type;
	if( block_read((char *)year_data, sizeof(year_data) , first_block, &this_block_type ) < 0)
	{
		gtlogerr("block_read\n");
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
	//�ȶ���һ��
	if( (err=sda_read(buf, bufsize,1, seek)) < 0 )
	{
	 printf("hd_read error\n");
	 return err;
	}
	*this_block_type = block_check(buf);
	switch ( *this_block_type )
	{
	case sec_block:
		sec_block_head = (struct hd_frame *)buf;
		if( (err=sda_read(buf+BLOCKSIZE ,bufsize- BLOCKSIZE, \
				get_block_num(sec_block_head->size + sizeof(struct hd_frame) ) - 1, seek+1)) < 0 )
		{
		 printf("hd_read error\n");
		 return err;
		}
		//��������
		//......
		break;
	case day_block:
		//����鿪ʼ�ĵڶ��鿪ʼ�� ������С-1��������
		if( (err=sda_read(buf+BLOCKSIZE ,bufsize- BLOCKSIZE, \
				get_block_num( sizeof(struct day_block) ) - 1, seek+1)) < 0 )
		{
		 printf("hd_read error\n");
		 return err;
		}
		//��������
		//......
		break;
	case year_block:
		//����鿪ʼ�ĵڶ��鿪ʼ�� ������С-1��������
		if( (err=sda_read(buf+BLOCKSIZE ,bufsize- BLOCKSIZE, \
				get_block_num( sizeof(struct year_block) ) - 1, seek+1)) < 0 )
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
	if(block == NULL )
		return BLOCK_ERR_NULL;
	yearblock = (struct year_block *)year_data;
	//�ж�������ǲ��ǿյ�
	if(yearblock->year_queue_data.queue_size==0)return BLOCK_ERR_EMPTY;
	if(get_type == head)
		memcpy(block, &(yearblock->sekk_block_data[yearblock->year_queue_data.queue_head]),\
					sizeof(struct seek_block));
	else
	memcpy(block, &(yearblock->sekk_block_data[yearblock->year_queue_data.queue_tail-1]) ,\
			sizeof(struct seek_block));
	return 0;
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
	if(sda_write(year_data, sizeof(year_data), sizeof(year_data), first_block) < 0)
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
	if(sda_write(year_data, sizeof(year_data), sizeof(year_data), first_block) < 0)
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
/**
 * ���Ժ�����block_year_add��block_year_get
 */
int test_year_data2()
{
	static struct  seek_block block;
	static enum opera_type get_type;
	while(1)
	{
		block.seek=rand();
		block.time=rand()/*%MAXDAY+1*/ /*+1��ֹ����0*/;
		if(block_year_add(&block,get_type) < 0)
		{
			printf("block year add err and exit");
			return -1;
		}
		printf("write:\tblock.seek:%lld,\tblock.time:%d\n",block.seek, block.time);

		if(block_year_get(&block,tail) < 0)
		{
			printf("block year get err and exit");
			return -1;
		}
		printf("read:\tblock.seek:%lld,\tblock.time:%d\n",block.seek, block.time);
	}
	return 0;

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
	int err;
	//block�������Ƿ�Ϸ�
	if(block == NULL )
			return BLOCK_ERR_NULL;
	if(block->time==0 || block->seek==0)
			return BLOCK_ERR_ZERO;

	if(memcmp(day_data, day_head, 8)!=0)
	{
		return BLOCK_ERR_DATA_HEAD;
	}
	//��������
	index = block->time%SECOFDAY;
	printf("index:%d\n",index);
	if(daydata->seek_block_data[index].seek !=0 )
		return BLOCK_ERR_DAY_SEC_MUT;//��ǰ�����λ����ֵ�ˡ�
	else
		memcpy(&(daydata->seek_block_data[index]), block, sizeof(struct  seek_block));


	//ÿtmp == x��д��һ����顣  hd_current_day_seek
	tmp++;
	if(tmp >= 10)
	{
		gtloginfo("day_data write->day.seek:%lld,sec.seek:%lld,index:%d\n", \
				hd_current_day_seek,block->seek,index);
		tmp = 0;
		//day_data
		if( ( err = sda_write(day_data, sizeof(day_data), sizeof(day_data), hd_current_day_seek ) ) < 0 )
		{
			gtlogerr("hd_write day_data error\n");
			return err;
		}
	}
	return 0;

}
/***********************************************************
*����:����day_data�е�ֵ�ҵ�Ӧ��д���λ��;day_data��ֻ��¼��I֡��λ�ã�Ҫ����I֡�����P֡�����ҵ�Ӧ��д���λ��
*���أ�д���λ��
 ***********************************************************/
long long  block_day_read_and_get_seek()
{
	struct day_block *daydata    =  (struct day_block  *)day_data;
	struct seek_block *seek_block_data = daydata->seek_block_data;
	int i;
	for(i = SECOFDAY; i > 0; i--)//
	{
		//printf("i:%d,time:%d,seek:%lld\n",i,seek_block_data[i].time,seek_block_data[i].seek);
		//continue;
		if(seek_block_data[i].seek !=0 )//���㷨Ҫ�������1S����Ƶ����
		{
			printf("find sec of dayindex:%d:%lld\n",i,seek_block_data[i].seek);
			return seek_block_data[i].seek;
		}
	}
	//exit(1);
	printf("!!!!day block is emputy\n");
	return 0;//��ǰ�����û����Ч����
}
/***********************************************************
*����:�����ϵ���鱸�ݵ�day_data_bac��
*���أ�0����
 ***********************************************************/
int block_day_backup()
{
	int err;
	//��ȡ���������seek
	static struct seek_block dayblock;
	if ((err = block_year_get(&dayblock, head)) < 0)
	{
		printf("block year get err and exit");
		gtlogerr("block year get err and exit");
		return err;
	}
	//�������������day_data_bac��
	if ((err = sda_read(day_data_bac, sizeof(day_data_bac),
			get_block_num( sizeof( struct day_block ) ), dayblock.seek))
			< 0)
	{
		printf("hd_read error\n");
		printf("block year get err and exit");
		return err;
	}
	return 0;
}
/***********************************************************
*����:��Ӳ����д�����ݡ�
*
 ***********************************************************/
static int write_disk1()
{
	static struct  seek_block block, front_dayblock/*�������ǰ�����*/;
	static enum opera_type get_type;
	static enum block_type this_block_type;
	static int err;
	static int sys_time;

	int buff_size;
	int buff_blocks;//buffռ�п���
	long long seek_tmp;
	static bool start=false;

	//д����
	while(1)
	{



		//�������е����
		if(hd_blocks< hd_current_sec_seek)
		{
			gtlogerr("hd_blocks:%lld,hd_current_sec_seek:%lld\n",hd_blocks,hd_current_sec_seek );
			return -19;
		}
/**********************************************************************************************************************************************************/
		//1����ϵͳʱ�䣺\
					a�����ڽ��죨ͨ���������\
					b��"����"���ˣ�1���������2�����������ڽ����23��59��59.999999�����еģ�������:���ڴ��е����д��Ӳ�̣�memset(day_data)\
					c��ϵͳʱ��Ƚ���0����ǰ�ˣ�\
											A����ǰ������20���ӣ���Ϊ��ϵͳ��ʱ���󣬻��������󣬴�ʱ��Ӧ��д�����Ŀ飬����������\
											B����ǰ����20���ӣ�������������󣿣�����
		get_type = tail;
		err = block_year_get(&block,get_type);
		if(err < 0)
		{
			gtlogerr("write_disk1 block_year_get err\n");
			return err;
		}
		sys_time = get_time();
		if(sys_time/SECOFDAY == block.time/SECOFDAY)/*ϵͳʱ�����������һ������ʱ����ͬһ��*/
		{
			goto next1;
		}
		else if(sys_time > block.time)//"����"��ȥ��
		{
			gtloginfo("sys_time:%d,block.time:%d\n",sys_time,block.time);
			gtloginfo("hd_current_sec_seek:%lld, hd_current_day_seek:%lld\n",\
					hd_current_sec_seek,hd_current_day_seek);
			return BLOCK_ERR_DAY_PASS;
		}
		else if(block.time - sys_time >60*20)//��ǰ������20����
		{
			printf("go head\n");
			goto next1;
		}
		else
		{
			gtlogerr("BLOCK_ERR_UNKNOW_TIME");
			return BLOCK_ERR_UNKNOW_TIME;
		}
/**********************************************************************************************************************************************************/
next1:

		if( (buff_size = get_frame())<0)
		{
			gtlogerr("get frame err");
			return buff_size;
		}
		if( ( (struct hd_frame *)hd_frame_buff )->is_I == 1)
			start = true;
		if(!start)
			continue;

		if ((err = block_year_get(&front_dayblock, head)) < 0)//��ȡ�������λ��
		{
			printf("block year get err and exit");
			gtlogerr("block year get err and exit");
			return err;
		}
/**********************************************************************************************************************************************************/
		//2���ж�Ӳ�̵�ʣ��ռ��Ƿ�д��һ֡�� \
						a��������hd_current_day_seek��hd_current_sec_seek��ֵ\
						b��δ��
		buff_blocks = get_block_num( buff_size );//����˿���Ӳ����ռ�õĿռ�
		if(hd_blocks - hd_current_sec_seek < buff_blocks)//ʣ�µĿռ䲻�㹻д���֡������
		{
			gtloginfo("debug HD_ERR_FULL\n");
			if( front_dayblock.time/SECOFDAY == block.time/SECOFDAY )//�������ǰ��һ���ʱ��͵�ǰ���ʱ����ͬһ�죬˵��Ӳ�̿ռ䲻���Դ���һ�������
			{
				if ((err = block_day_backup()) < 0)//������������,�����������ɾ���ڵ�
				{
					gtloginfo("debug block_day_backup  %d\n", __LINE__);
					return err;
				}
				hd_current_sec_seek =first_seek+ get_block_num( sizeof( struct day_block )  );//���Ӳ�̿ռ䲻�����һ������ݣ�����鲻���ǡ����ȥ��
			}
			else
				hd_current_sec_seek =first_seek;
			gtloginfo("hd_current_sec_seek:%d\n",hd_current_sec_seek);
			//return HD_ERR_FULL;
		}
/**********************************************************************************************************************************************************/
		//3���ж���һ��Ҫд������ݵ�λ���Ƿ��ǿ�����(��ʵֻ�����˺��Ӧ���ж�)��  \
						a���ǿ�����(������ڶ������ʱ�����) \
						b�����ǿ�����:  \
									A�������:��day_data_bac���������λ��(������ڶ������ʱ�����)\
									B������飺�������day_data_bac�У�ͬʱд�뵽��Ӧ��λ��
/**********************************************************************************************************************************************************/
		if( (front_dayblock.seek - hd_current_sec_seek >= 0)&&
				(buff_blocks > front_dayblock.seek - hd_current_sec_seek) )//����Ҫ����������������ˡ�++ֻ��Ӳ�̿ռ��㹻��һ������ݣ����if��Ϊ�棬������
		{
			printf("%d\tBLOCK_INFO_DAY_BLOCK_COVER:dayblock.seek:%d,hd_current_sec_seek:%d,buff_blocks:%d\n",
					BLOCK_INFO_DAY_BLOCK_COVER,front_dayblock.seek,hd_current_sec_seek,buff_blocks);
			gtloginfo(
					"%d\tBLOCK_INFO_DAY_BLOCK_COVER:dayblock.seek:%d,hd_current_sec_seek:%d,buff_blocks:%d\n",
					BLOCK_INFO_DAY_BLOCK_COVER,front_dayblock.seek,hd_current_sec_seek,buff_blocks);
			if ((err = block_day_backup()) < 0)//������������
			{
				gtloginfo("debug block_day_backup  %d\n", __LINE__);
				return err;
			}
			//�������ɾ���������顣
			block_year_del(head);
		}
		//4��д����
		err=sda_write(hd_frame_buff, sizeof(frame_buff), buff_size, hd_current_sec_seek);
		if(err<0)
		{
			gtlogerr("debug\n");
			return err;
		}
		printf("hd_current_sec_seek:%lld,hd_current_day_seek:%lld\n",hd_current_sec_seek,hd_current_day_seek);
		hd_current_sec_seek = hd_current_sec_seek + buff_blocks;
		if(( (struct hd_frame *)hd_frame_buff )->is_I == 1)
		{
			//gtloginfo("sys_time:%d\n",sys_time);
			//gtloginfo("is_I:hd_current_sec_seek:%lld,hd_current_day_seek:%lld\n",hd_current_sec_seek,hd_current_day_seek);
			block.time = sys_time;
			block.seek = hd_current_sec_seek;
			if( ( ( err = block_day_add(&block) ) < 0 ) && (err != BLOCK_ERR_DAY_SEC_MUT ) )//��I֡��д��
			{
				gtlogerr("debug\n");
				return err;
			}
		}
	}
	return 0;
}
int write_disk()
{
	int err;
	static struct  seek_block block;
	static enum opera_type get_type;
	static enum block_type this_block_type;
	long long seek_tmp;


	printf("day_data:%d,year_data:%d\n",sizeof(day_data),sizeof(year_data));
	printf("block of day_data:%d,block of year_data:%d\n", \
			get_block_num( sizeof(day_data ) ) , \
			get_block_num( sizeof(year_data ) ) );
	//��������ҵ���ǰ����λ��
	if(yearblock->year_queue_data.queue_size ==0)//��Ӳ��
	{
		gtloginfo("new disk1\n");
		block.seek = first_seek;
		block.time = get_time();
		//��������һ��
		if( ( err=block_year_add(&block, get_type) ) < 0)
		{
			gtloginfo("debug\n");
			return err;
		}
		hd_current_day_seek = first_seek;
		memset(day_data, 0, sizeof(day_data));
		hd_current_sec_seek = first_seek+get_block_num( sizeof(day_data) );
		//д���ͷ
		memset(day_data , 0 , sizeof(day_data));
		memcpy( day_data, day_head, sizeof(day_head) ); //����ͷ
		if(sda_write(day_data, sizeof(day_data), sizeof(day_data), first_seek) < 0)
		{
			gtloginfo("debug\n");
			printf("hd_write error\n");
			return -1;
		}

	}else
	{
		get_type = tail;
		err = block_year_get(&block, get_type);
		if (err < 0)
		{
			gtlogerr("block_year_get err\n");
			return err;
		}
		hd_current_day_seek = block.seek; //ָ����ǰ����λ��
		printf("hd_current_day_seek:%lld\n",hd_current_day_seek);
		//��ȡ�������ݵ��ڴ���
		err = block_read(day_data, sizeof(day_data), hd_current_day_seek,
				&this_block_type);
		if (err < 0)
		{
			gtloginfo("debug");
			return err;
		}
		if (this_block_type != day_block)
		{
			gtlogerr("read type err!!!\n");
			return BLOCK_ERR_READ_TYPE;
		}
		//��������ҵ���ǰ����λ��
		seek_tmp = block_day_read_and_get_seek();
		if (seek_tmp == 0)
		{
			printf("read secseek of day err!!!\n");
			//return 0;
			hd_current_sec_seek = hd_current_day_seek
					+ get_block_num( sizeof(day_data) );
		}
		else
			hd_current_sec_seek = seek_tmp;
		gtloginfo("hd_current_sec_seek:%lld\n",hd_current_sec_seek);
//return 0;
	}

	//��ʼ��mediaapi
	//��ʼ������
	memset(&media ,0, sizeof(media_source_t));
	media.dev_stat= -1; //��ʾû������
	err=connect_media_read(&media ,0x30000, "video", /*MSHMPOOL_LOCAL_USR*/1);
	if(err!=0)
	{
		gtlogerr("error in connect media read and exit\n");
		return -1;
	}
	else
		gtloginfo("connect success\n");

	while(1)
	{
		err = write_disk1();
		switch (err)
		{
		case BLOCK_ERR_DAY_PASS:/*Ӧ��д�������*/
			gtloginfo("debug BLOCK_ERR_DAY_PASS\n");
			block.seek = hd_current_sec_seek  ;
			block.time = get_time();
			//��������һ��
			if( ( err=block_year_add(&block, get_type) ) < 0)
			{
				gtloginfo("block_year_add->seek:%lld,time:%d\n",block.seek,block.time);
				return err;
			}
			hd_current_day_seek = hd_current_sec_seek;
			memset(day_data, 0, sizeof(day_data));
			memcpy(day_data, day_head, sizeof(day_head)); //����ͷ
			//д���
			if (sda_write(day_data, sizeof(day_data), sizeof(day_data),
					hd_current_sec_seek) < 0)
			{
				gtlogerr("hd_write error\n");
				return -1;
			}
			hd_current_sec_seek =hd_current_sec_seek + get_block_num( sizeof(day_data) );
			gtloginfo("sizeof day_data:%d\n",sizeof(day_data));
			gtloginfo("hd_current_sec_seek:%lld, hd_current_day_seek:%lld\n",\
								hd_current_sec_seek,hd_current_day_seek);
			break;
		default:
			gtlogerr("unknow err:%d\n",err);
			return err;
			break;
		}
	}
	return 0;
}
/***********************************************************
*����:��ȡ��Ƶ֡����
*���أ�<0����  >0��Ƶ֡�Ĵ�С
 ***********************************************************/
int get_frame()
{
#ifdef TEST_RAM
	int seed;
	while(1)
	{
		seed =rand();
		if(seed>10*1204&&seed<40*1024)
			break;
	}
	struct hd_frame *secdata = (struct hd_frame *)frame_buff;
	memcpy(secdata->data_head, framehead, sizeof(framehead));
	secdata->size=seed;
	if(get_time()%24==0)
	{
		DP("I frame\n");
		secdata->is_I = 1;
		memset(frame_buff+sizeof(struct hd_frame), 0xff, seed);
		return sizeof(struct hd_frame)+seed;
	}
	else
	{
		//DP("P frame\n");
		secdata->is_I = 0;
		memset(frame_buff+sizeof(struct hd_frame), (unsigned char)(get_time()%24) , seed);
		return sizeof(struct hd_frame)+seed;
	}
#else
	int ret;
	static int		seq=-1;                 ///<ý���������
	static int 	flag;
	//��ȡ֡������ֵΪ֡��С ����kframe��ʲô�أ����ѵ�flag �����ǲ���I֡�����
	memset(frame_buff, 0, BUFSIZE);
	seq=-1;flag=-1;
	ret=read_media_resource(&media,frame_buff, BUFSIZE, &seq, &flag);
	if(ret<0)
	{
		printf("error in read media resource\n");
		//exit(1);
	}
	enc_frame_t* the_frame_buffer=(enc_frame_t *)frame_buff;

	int is_i;
	if(flag==1)
		is_i=0;
	else
		is_i=1;
	hd_frame_buff=the_frame_buffer->frame_buf-sizeof(struct hd_frame);
	int len = the_frame_buffer->len;
	//printf("len:%d\n",len);
	memcpy(hd_frame_buff, framehead, sizeof(framehead));
	struct hd_frame *hdbuf = (struct hd_frame *)(hd_frame_buff);
	hdbuf->size=len;
	hdbuf->is_I=is_i;
	//printf("the_frame_buffer->len:%d",the_frame_buffer->len);

	return len +sizeof(struct hd_frame);

#endif
	return BLOCK_ERR_GET_FRAME;
}
inline int get_time()
{
#ifdef TEST_RAM
	static unsigned int time=25;
	time++;
	return time/25;
#else
	return time(NULL);
#endif
}
/*
 * ����д��Ļ�ȡ��Ƶ֡����
 */
int test_getframe_gettime()
{
#ifdef TEST_RAM
	struct hd_frame *secdata = (struct hd_frame *)frame_buff;
	int tmp;
	unsigned char *p=frame_buff + sizeof(struct hd_frame);
	while(1)
	{
		 get_time();
		 if(0 != get_frame())
			 break;
		 if(1 == secdata->is_I)
		 {
			 //printf("I frame\n");
			 for(tmp = 0; tmp < secdata->size; tmp++)
			 {
				 if(0xff != p[tmp])
				 {
					 printf("I frame wrong\n");
					 goto err;
				 }
			 }
		 }
		 else
		 {
			 //printf("P frame\n");
			 for(tmp = 0; tmp < secdata->size; tmp++)
			 {
				if (p[0] != p[tmp])
				{
					printf("P frame wrong\n");
					goto err;
				}
			 }
		 }

	}
#else

#endif
err:
	printf("maybe something is wrong!\n");
	return 0;
}
int test_print_size()
{
	printf("sizeof year_data:%d\n",sizeof(year_data));
	printf("sizeof day_data: %d\n",sizeof(day_data)) ;
	printf("first_seek:    %lld\n",first_seek);
	exit(1);
	return 0;
}
void test_write_read()
{
#define test 1
	int err;
	memset(day_data , 0 , sizeof(day_data));
	memcpy(day_data, day_head, sizeof(day_head));
	printf("first_seek:%lld\n",first_seek);
	printf("block num:%d\n",get_block_num( sizeof(day_data) ) );
	int block_num = get_block_num( sizeof(day_data) );
	int i;
	for(i=8; i< sizeof(day_data); i++)
	{
		day_data[i] = 0xff;
	}
#if test
	if(sda_write(day_data, sizeof(day_data), sizeof(day_data), first_seek) < 0)
	{
		gtlogerr("hd_write error\n");
		exit(1);
	}
#else
	if( hd_write(get_hd_fd(), first_seek, block_num, \
			day_data, block_num*512) < 0)
		exit(1);
#endif
	memset(day_data , 0 , sizeof(day_data));
#if test
	enum block_type this_block_type;
	err = block_read(day_data, sizeof(day_data), first_seek,
			&this_block_type);
	if (err < 0)
	{
		gtloginfo("debug");
		exit(1);
	}
	printf("this type:%d\n",this_block_type);
#else
	//��ʵӲ�̴���
	if( hd_read(get_hd_fd(), first_seek, block_num, \
			day_data, block_num*512) < 0)
		exit(1);
#endif
	for(i=8; i< sizeof(day_data); i++)
	{
		if(day_data[i]!=0xff)
		{
			printf("%d err\n",i);
			exit(1);
		}
	}
}
void test_secseekofday(void *arg)
{
	sleep(4);
	printf("%s\n",arg);
	while(1){
		sleep(1);
		printf("current sec seek:%lld\n",block_day_read_and_get_seek());
	}
}

/********fifo***************************************************************/
static int fd;
/*��ʼ���ܵ�*/
int fifo_init()
{
	mkfifo("test.264", 0777);
	int ret;
	fd = open("test.264", O_WRONLY);
	if (fd < 0)
	{
		perror("openfile error");
		return -1;
	}
	return 0;
}
/*д�ܵ�*/
int fifo_write(char *buf, int size)
{
	int ret = write(fd, buf, size);
	if (ret < 0)
	{
		//perror("write error\n");
		return -1;
	}
	return ret;
}
/*�ͷŹܵ�*/
void fifo_free()
{
	close(fd);
}
/********fifo***************************************************************/


/*************************************************************************
 * ���ܣ���Ӳ���������飬���¼��ʱ��
 *************************************************************************/
int read_disk_print_record_time()
{
	int err;
	int day, sec;
	int tmp=0;
	int bool_tmp=0;
	static enum block_type this_block_type;
	//�ж��ǲ���������
	if( yearblock->year_queue_data.queue_size == 0)
	{
		printf("yearblock is emputy!!!\n");
		gtloginfo("%d\tyearblock is emputy!!!\n",BLOCK_ERR_EMPTY);
		return BLOCK_ERR_EMPTY;
	}
	printf("day of yearblock:%d\n",yearblock->year_queue_data.queue_size);
	//��������е����ݣ�
	for(day=yearblock->year_queue_data.queue_head; \
		day < yearblock->year_queue_data.queue_head + yearblock->year_queue_data.queue_size; \
		day++)
	{
		if( yearblock->sekk_block_data[day].seek==0 || yearblock->sekk_block_data[day].time == 0)
		{
			gtloginfo("%d\tBLOCK_ERR_YEAR_PRINT\n",BLOCK_ERR_YEAR_PRINT);
			return BLOCK_ERR_YEAR_PRINT;
		}
		printf("today's time is:%d --> %s",yearblock->sekk_block_data[day].time, \
				ctime( &( yearblock->sekk_block_data[day].time ) ) );
		printf("today's block:\n");
		//��ȡ�������ݵ��ڴ���
		err = block_read(day_data, sizeof(day_data), yearblock->sekk_block_data[day].seek,
				&this_block_type);
		if (err < 0)
		{
			printf("debug");
			return err;
		}
		//��������ڵ�������
		for( sec =(yearblock->sekk_block_data[day].time)%SECOFDAY ; \
			 sec < SECOFDAY ; \
			 sec++ )
		{
			if(bool_tmp == 0)
			{
				if( daydata->seek_block_data[sec].seek!=0 || daydata->seek_block_data[sec].time != 0 )
				{

					printf("start:\tblock:%lld\ttime:%d-->%s",daydata->seek_block_data[sec].seek , \
							daydata->seek_block_data[sec].time, \
							ctime( &( daydata->seek_block_data[sec].time ) ) );
					bool_tmp = 1;
				}
			}
			else
			{
				if( daydata->seek_block_data[sec].seek==0 && daydata->seek_block_data[sec].time == 0 )
				{
					printf("end:\tblock:%lld\ttime:%d-->%s",daydata->seek_block_data[sec-1].seek , \
							daydata->seek_block_data[sec-1].time, \
							ctime( &( daydata->seek_block_data[sec-1].time ) ) );
					bool_tmp = 0;
					printf("\n");
				}
			}
		}
	}
	return 0;
}
/*************************************************************************
 * ���ܣ���Ӳ��ͨ��rtsp����
 *************************************************************************/
int read_disk(int time_begin)
{
	int err;
	static struct  seek_block block;
	static enum opera_type get_type;
	static enum block_type this_block_type;
	long long seek_tmp;
	int tmp;

	printf("read_disk\n");
	//��������ҵ���ǰ����λ��
	if(yearblock->year_queue_data.queue_size ==0)//��Ӳ��
	{
		printf("new disk and exit!!\n");
		return BLOCK_ERR_READ_NEW_DISK;
	}else
	{
		get_type = tail;
		err = block_year_get(&block, get_type);
		if (err < 0)
		{
			printf("block_year_get err\n");
			return err;
		}
		hd_current_day_seek = block.seek; //ָ����ǰ����λ��
		printf("hd_current_day_seek:%lld\n",hd_current_day_seek);
		//��ȡ�������ݵ��ڴ���
		err = block_read(day_data, sizeof(day_data), hd_current_day_seek,
				&this_block_type);
		if (err < 0)
		{
			printf("debug");
			return err;
		}
		if (this_block_type != day_block)
		{
			printf("read type err!!!\n");
			return BLOCK_ERR_READ_TYPE;
		}
		//��������ҵ���ǰ����λ��
		for(tmp=time_begin%SECOFDAY; tmp < SECOFDAY; tmp++)
		{
			if(daydata->seek_block_data[tmp].seek != 0 && daydata->seek_block_data[tmp].time != 0)
			{
				hd_current_sec_seek = daydata->seek_block_data[tmp].seek;
				printf("seek:%d\n",daydata->seek_block_data[tmp].seek);
				break;
			}
		}
	}

	printf("hd_current_sec_seek:%lld\n",hd_current_sec_seek);

	while(1)
	{
		usleep(40000);		//25֡
		err = block_read(frame_buff, sizeof(frame_buff), hd_current_sec_seek,
				&this_block_type);
		if (err < 0)
		{
			gtloginfo("block read err\n");
			return err;
		}
		else
			printf("read sec frame ok\n");
		if(this_block_type != sec_block)
		{
			printf("block read is not sec_block\n");
			return 0;
		}
		printf("size:%d\n", ((struct hd_frame *)frame_buff)->size);
		if(fifo_write(frame_buff + sizeof(struct hd_frame) , ((struct hd_frame *)frame_buff)->size)<0)
		{
			printf("fifo write err\n");
			return 0;
		}
		//�˿�������Ӳ����ռ�ռ�Ĵ�С
		hd_current_sec_seek = hd_current_sec_seek + \
				get_block_num( ((struct hd_frame *)frame_buff)->size + sizeof(struct hd_frame));
		printf("next sec seek:%lld,",hd_current_sec_seek);
	}
	return 0;
}
