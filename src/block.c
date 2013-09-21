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
//硬盘上的总块数，也是容量
static long long hd_blocks;
//硬盘上当前天块的位置，读写时会用到
static long long hd_current_day_seek=0;
//当前秒块应该写入的位置
static long long hd_current_sec_seek = 0;
static char year_data[(sizeof(struct year_block) + BLOCKSIZE -1)/BLOCKSIZE*BLOCKSIZE];
static char day_data_bac[(sizeof(struct day_block) + BLOCKSIZE -1)/BLOCKSIZE*BLOCKSIZE];
static char day_data[(sizeof(struct day_block) + BLOCKSIZE -1)/BLOCKSIZE*BLOCKSIZE];
static long long first_seek=first_block+sizeof(year_data)+sizeof(day_data_bac);
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
	char buf[512]={0};
	int err;
#ifndef YEARBLOCKTEST
	//从第0块开始读1块，用大小512字节的缓冲区
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
		//写年块头
		struct day_block *daydata=(struct day_block *) day_data ;
		memset(year_data , 0 , sizeof(year_data));
		memcpy( yeardata->year_head, day_head, sizeof(day_head) ); //数据头
		//yeardata->year_queue_data.queue_size=0;
		if(hd_write(year_data, sizeof(year_data), sizeof(year_data), first_block) < 0)
		{
			printf("hd_write error\n");
			return -1;
		}
		//写天块头
		memset(day_data , 0 , sizeof(day_data));
		memcpy( day_data, day_data, sizeof(day_data) ); //数据头
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
		//秒块读到了
		//......
		break;
	case day_block:
		//从天块开始的第二块开始读 （天块大小-1）块数据
		if( (err=hd_read(buf+BLOCKSIZE ,bufsize- BLOCKSIZE, \
				(sizeof(struct day_block) + BLOCKSIZE -1)/BLOCKSIZE - 1, seek+1)) < 0 )
		{
		 printf("hd_read error\n");
		 return err;
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
		 return err;
		}
		//年块读到了
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
*功能：获取year block存储较早块的位置
*
 ***********************************************************/
int block_year_get(struct  seek_block *block,enum opera_type get_type)
{
	struct year_block *yearblock = NULL;
	//block中数据是否合法
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
*功能:向年块中添加新块（只添加新写入的块）
*
 ***********************************************************/
int block_year_add(struct  seek_block *block,enum opera_type get_type)
{
	struct year_block *yearblock  = (struct year_block *)year_data;;
	//block中数据是否合法
	if(block == NULL )
			return BLOCK_ERR_NULL;
	if(block->time==0 || block->seek==0)
			return BLOCK_ERR_ZERO;
	//判断队列中是否还有空间
	if(yearblock->year_queue_data.queue_tail+1 == yearblock->year_queue_data.queue_head)
	{
#ifdef YEARBLOCKTEST
		printf("head:%d\n",yearblock->year_queue_data.queue_head);
		printf("tail:%d\n",yearblock->year_queue_data.queue_tail);
		printf("size:%d\n",yearblock->year_queue_data.queue_size);
#endif
		printf("year block full\n");
		return BLOCK_ERR_FULL;//满了
	}
	memcpy( &(yearblock->sekk_block_data[yearblock->year_queue_data.queue_tail]) ,\
			block, sizeof(struct seek_block));
	yearblock->year_queue_data.queue_tail = (yearblock->year_queue_data.queue_tail+1)%MAXDAY;
	yearblock->year_queue_data.queue_size++;
	//写入数据
	if(hd_write(year_data, sizeof(year_data), sizeof(year_data), first_block) < 0)
	{
		printf("hd_write error\n");
		return HD_ERR_WRITE;
	}
	return 0;
}
/***********************************************************
*功能：向年块中删除块（只删除最老的块，除非你想做任意删除）
*
 ***********************************************************/
int block_year_del(enum opera_type get_type)
{
	struct year_block *yearblock = NULL;
	yearblock = (struct year_block *)year_data;
	if(yearblock->year_queue_data.queue_tail == yearblock->year_queue_data.queue_head)
	{
		printf("emputy\n");
		return BLOCK_ERR_EMPTY;//空了
	}
	memset( &yearblock->sekk_block_data[yearblock->year_queue_data.queue_head],0 ,sizeof(struct  seek_block) );
	yearblock->year_queue_data.queue_head = (yearblock->year_queue_data.queue_head+1)%MAXDAY;
	yearblock->year_queue_data.queue_size--;
	//写入数据
	if(hd_write(year_data, sizeof(year_data), sizeof(year_data), first_block) < 0)
	{
		printf("hd_write error\n");
		return HD_ERR_WRITE;
	}
	return 0;
}
//对年块的读写，和逻辑的测试
int test_year_data()
{
#ifdef YEARBLOCKTEST
	//产生在0->MAXDAY之间的随机数，操作使年块中的数据达到随机数间。如果数据多了调用del,少了调add
	int i,times;
	struct year_block *yearblock = (struct year_block *)year_data;
	static struct  seek_block block;
	static enum opera_type get_type;
	printf("head:%d\n",yearblock->year_queue_data.queue_head);
	printf("tail:%d\n",yearblock->year_queue_data.queue_tail);
	printf("size:%d\n",yearblock->year_queue_data.queue_size);
	while(1)//测到你满意为止
	{
		int seed = rand()%MAXDAY;
		//1、创建数据
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
				block.time=rand()/*%MAXDAY+1*/ /*+1防止出现0*/;
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
		//2、验证数据：
		//printf("block head time:%d\nblock tail time:%d\n",\
				yearblock->sekk_block_data[yearblock->year_queue_data.queue_head].time,\
				yearblock->sekk_block_data[yearblock->year_queue_data.queue_tail].time);
		//if ( yearblock->year_queue_data.queue_size < 6 )//小数据输出队列内容
		{
			int j;
			//printf("\n");
			//printf("\tsize:%d\n",yearblock->year_queue_data.queue_size);
			//队列中不为0正常，为0则可能逻辑上的错误
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
			//不在队列中的数据应该全部为0，不为0则可能有逻辑错误
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
*功能:向天块中添加新块，每有一个秒（帧）块写入时。添加新块。
*
 ***********************************************************/
int block_day_add(struct  seek_block *block)
{
	struct day_block *daydata = (struct day_block *)day_data;
	int index;//今天的第多少秒
	static int tmp = 0;
	//block中数据是否合法
	if(block == NULL )
			return BLOCK_ERR_NULL;
	if(block->time==0 || block->seek==0)
			return BLOCK_ERR_ZERO;
	//memcmp(daydata, day_head, 8)!=0,
	if(*(long long *)daydata != *(long long *)day_head)
	{
		return BLOCK_ERR_DATA_HEAD;
	}
	//复制数据
	index = block->time%SECOFDAY;
	if(daydata->seek_block_data[index].seek !=0 )
		return BLOCK_ERR_DAY_SEC_MUT;//当前的秒块位置有值了。
	else
		memcpy(&daydata->seek_block_data[index], block, sizeof(struct  seek_block));


	//每tmp == x次写入一次天块。  hd_current_day_seek
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
*功能:向硬盘中写入数据。
*
 ***********************************************************/
int write_disk()
{
	struct year_block *yearblock =  (struct year_block *)year_data;
	struct  seek_block block;
	enum opera_type get_type;
	enum block_type this_block_type;
	int err;
	//在年块中找到当前天块的位置
	if(yearblock->year_queue_data.queue_size ==0)//新硬盘
	{
		block.seek = first_seek;
		block.time = time(NULL);
		//年块中添加一块
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
	//读取天块的内容
	err = block_read(day_data, sizeof(day_data) ,hd_current_day_seek, &this_block_type);
	if(err < 0)
		return err;
	if(this_block_type != day_block)
	{
		printf("read type err!!!\n");
		return BLOCK_ERR_READ_TYPE;
	}
	//在天块中找到当前秒块的位置
			/*此处应该是多余的，直接读写就好了。*/
	//写数据
	write_sec:
	while(1)
	{
		//1读系统时间：\
					a、还在今天（通常情况）：\
					b、今天过了（1、正常情况2、程序正好在今天的23点59分59.999999秒运行的！！！）:把内存中的天块写入硬盘，memset(day_data)\
					c、系统时间比今天0点提前了：\
											A、提前不超过20分钟：认为是系统对时错误，或其它错误，此时间应该写入今天的块，而不是昨天\
											B、提前超过20分钟：：这是神马错误？？？？
		//2、判断硬盘是否满： \
						a、满：改hd_current_day_seek和hd_current_sec_seek的值\
						b、未满

		//3、判断下一块要写入的数据的位置是否是空数据(其实只有满了后才应该判断)：  \
						a、是空数据 \
						b、不是空数据:  \
									A、是秒块:从day_data_bac，中清除此位置\
									B、是天块：把天读到day_data_bac中，同时写入到对应的位置

		//写数据
	}
	return 0;
}
