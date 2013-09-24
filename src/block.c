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
static char day_data[    (sizeof(struct day_block) + BLOCKSIZE -1)/BLOCKSIZE*BLOCKSIZE];
static char frame_buff[BUFSIZE];
static long long first_seek=first_block+(sizeof(year_data)+sizeof(day_data_bac) + BLOCKSIZE - 1 )/BLOCKSIZE;


static const struct year_block *yearblock =  (struct year_block *)year_data;
static const struct day_block *daydata    =  (struct day_block  *)day_data;
static const struct hd_frame  *secdata    =  (struct hd_frame    *)frame_buff;
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
		memcpy( day_data, day_head, sizeof(day_head) ); //数据头
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
	if(block == NULL )
		return BLOCK_ERR_NULL;
	yearblock = (struct year_block *)year_data;
	//判断年块中是不是空的
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
/**
 * 测试函数：block_year_add和block_year_get
 */
int test_year_data2()
{
	static struct  seek_block block;
	static enum opera_type get_type;
	while(1)
	{
		block.seek=rand();
		block.time=rand()/*%MAXDAY+1*/ /*+1防止出现0*/;
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
*功能:向天块中添加新块，每有一个秒（帧）块写入时。添加新块。
*
 ***********************************************************/
int block_day_add(struct  seek_block *block)
{
	struct day_block *daydata = (struct day_block *)day_data;
	int index;//今天的第多少秒
	static int tmp = 0;
	int err;
	//block中数据是否合法
	if(block == NULL )
			return BLOCK_ERR_NULL;
	if(block->time==0 || block->seek==0)
			return BLOCK_ERR_ZERO;

	if(memcmp(day_data, day_head, 8)!=0)
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
	if(tmp >= 300)
	{
		tmp = 0;
		enum block_type this_block_type;
		//day_data
		if( ( err = hd_write(day_data, sizeof(day_data), sizeof(day_data), first_seek ) ) < 0 )
		{
			printf("hd_write error\n");
			DP("debug");
			return err;
		}
	}
	return 0;

}
/***********************************************************
*功能:根据day_data中的值找到应该写入的位置;day_data中只记录了I帧的位置，要读完I帧后面的P帧，并找到应该写入的位置
*返回：写入的位置
 ***********************************************************/
long long  block_day_read_and_get_seek()
{
	struct day_block *daydata    =  (struct day_block  *)day_data;
	struct seek_block *seek_block_data = daydata->seek_block_data;
	int i;
	for(i = SECOFDAY; i > 0; i--)//
	{
		if(seek_block_data[i].seek !=0 )//此算法要牺牲最后1S的视频数据
			return seek_block_data[i].seek;
	}
	printf("day block is emputy\n");
	return 0;//不前天块中没有有效数据
}
/***********************************************************
*功能:向硬盘中写入数据。
*
 ***********************************************************/
int write_disk1()
{
	static struct  seek_block block;
	static enum opera_type get_type;
	static enum block_type this_block_type;
	static int err;
	static int sys_time;

	int buff_size;
	int buff_blocks;//buff占有块数
	long long seek_tmp;
	static bool start=false;


	//printf("debug2\n");
	//写数据
write_sec:
	while(1)
	{
		if(hd_blocks< hd_current_sec_seek)
		{
			return -19;
		}
/**********************************************************************************************************************************************************/
		//1、读系统时间：\
					a、还在今天（通常情况）：\
					b、"今天"过了（1、正常情况2、程序正好在今天的23点59分59.999999秒运行的！！！）:把内存中的天块写入硬盘，memset(day_data)\
					c、系统时间比今天0点提前了：\
											A、提前不超过20分钟：认为是系统对时错误，或其它错误，此时间应该写入今天的块，而不是昨天\
											B、提前超过20分钟：：这是神马错误？？？？
		get_type = tail;
		//printf("debug2.1\n");
		err = block_year_get(&block,get_type);
		if(err < 0)
		{
			DP("debug\n");
			printf("block_year_get err\n");
			return err;
		}
		sys_time = get_time();
		//printf("debug: sys_time:%d\n",sys_time);
		if(sys_time/SECOFDAY == block.time/SECOFDAY)/*系统时间和年块中最后一块天块的时间在同一天*/
		{
			goto next1;
		}
		else if(sys_time > block.time)//"今天"过去了
		{
			DP("sys_time:%d,block.time:%d\n",sys_time,block.time);
			printf("hd_current_sec_seek:%lld, hd_current_day_seek:%lld\n",\
					hd_current_sec_seek,hd_current_day_seek);
			//hd_current_sec_seek;
			return BLOCK_ERR_DAY_PASS;
		}
		else if(block.time - sys_time >60*20)//提前不超过20分钟
		{
			goto next1;
		}
		else
		{
			DP("debug\n");
			return BLOCK_ERR_UNKNOW_TIME;
		}
		//printf("debug3\n");
/**********************************************************************************************************************************************************/


next1:
//printf("next1\n");
		if( (buff_size = get_frame())<0)
		{
			DP("debug");
			return buff_size;
		}
		if(secdata->is_I == 1)
			start = true;
		if(!start)
			continue;
/**********************************************************************************************************************************************************/
		//2、判断硬盘的剩余空间是否够写这一帧： \
						a、满：改hd_current_day_seek和hd_current_sec_seek的值\
						b、未满
		buff_blocks = ( buff_size + BLOCKSIZE - 1 ) / BLOCKSIZE;
		//printf("buff_blocks:%d\n",buff_blocks);
		if(hd_blocks - hd_current_sec_seek < buff_blocks)//剩下的空间不足够写入此帧数据了
		{
			DP("DEBUG\n");
			return HD_ERR_FULL;
		}
		//printf("debug3.1\n");
/**********************************************************************************************************************************************************/
		//3、判断下一块要写入的数据的位置是否是空数据(其实只有满了后才应该判断)：  \
						a、是空数据 \
						b、不是空数据:  \
									A、是秒块:从day_data_bac，中清除此位置\
									B、是天块：把天读到day_data_bac中，同时写入到对应的位置
/**********************************************************************************************************************************************************/
		//些步暂时没有。。

		//4、写数据
		err=hd_write(frame_buff, sizeof(frame_buff), buff_size, hd_current_sec_seek);
		if(err<0)
		{
			DP("debug\n");
			return err;
		}
		long long tmp_seek = (buff_size + BLOCKSIZE -1)/BLOCKSIZE;
		hd_current_sec_seek = hd_current_sec_seek + tmp_seek;
		//printf("hd_current_sec_seek:%lld,hd_current_day_seek:%lld\n",hd_current_sec_seek,hd_current_day_seek);
		if(secdata->is_I == 1)
		{
			printf("sys_time:%d\n",sys_time);
			printf("is_I:hd_current_sec_seek:%lld,hd_current_day_seek:%lld\n",hd_current_sec_seek,hd_current_day_seek);
			block.time = sys_time;
			block.seek = hd_current_sec_seek;
			if( ( ( err = block_day_add(&block) ) < 0 ) && (err != BLOCK_ERR_DAY_SEC_MUT ) )//是I帧，写入
			{
				DP("debug\n");
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

	//在年块中找到当前天块的位置
	if(yearblock->year_queue_data.queue_size ==0)//新硬盘
	{
		printf("new disk1\n");
		block.seek = first_seek;
		block.time = get_time();
		//年块中添加一块
		if( ( err=block_year_add(&block, get_type) ) < 0)
		{
			DP("debug\n");
			return err;
		}
		hd_current_day_seek = first_seek;
		memset(day_data, 0, sizeof(day_data));
		hd_current_sec_seek = first_seek+( sizeof(day_data) + BLOCKSIZE - 1 ) / BLOCKSIZE;
		//写天块头
		memset(day_data , 0 , sizeof(day_data));
		memcpy( day_data, day_head, sizeof(day_head) ); //数据头
		if(hd_write(day_data, sizeof(day_data), sizeof(day_data), first_seek) < 0)
		{
			DP("debug\n");
			printf("hd_write error\n");
			return -1;
		}

	}else
	{
		get_type = tail;
		err = block_year_get(&block, get_type);
		if (err < 0)
		{
			printf("block_year_get err\n");
			return err;
		}
		hd_current_day_seek = block.seek; //指定当前天块的位置
		//读取天块的内容到内存中
		//printf("debug1\n");
		err = block_read(day_data, sizeof(day_data), hd_current_day_seek,
				&this_block_type);
		if (err < 0)
		{
			DP("debug");
			return err;
		}
		if (this_block_type != day_block)
		{
			printf("read type err!!!\n");
			return BLOCK_ERR_READ_TYPE;
		}
		//在天块中找到当前秒块的位置
		//printf("debug1.1\n");
		seek_tmp = block_day_read_and_get_seek();
		if (seek_tmp == 0)
			hd_current_sec_seek = hd_current_day_seek
					+ (sizeof(day_data) + BLOCKSIZE - 1) / BLOCKSIZE;
		else
			hd_current_sec_seek = seek_tmp;

	}
	while(1)
	{
		err = write_disk1();
		switch (err)
		{
		case BLOCK_ERR_DAY_PASS:/*应该写入天块了*/
			printf("debug BLOCK_ERR_DAY_PASS\n");
			block.seek = hd_current_sec_seek  ;
			block.time = get_time();
			//年块中添加一块
			if( ( err=block_year_add(&block, get_type) ) < 0)
			{
				DP("debug");
				return err;
			}
			hd_current_day_seek = hd_current_sec_seek;
			memset(day_data, 0, sizeof(day_data));
			memcpy(day_data, day_head, sizeof(day_head)); //数据头
			//写天块
			if (hd_write(day_data, sizeof(day_data), sizeof(day_data),
					hd_current_sec_seek) < 0)
			{
				printf("hd_write error\n");
				return -1;
			}
			hd_current_sec_seek =hd_current_sec_seek + (sizeof(day_data) + BLOCKSIZE - 1)/BLOCKSIZE;
			printf("sizeof day_data:%d\n",sizeof(day_data));
			printf("hd_current_sec_seek:%lld, hd_current_day_seek:%lld\n",\
								hd_current_sec_seek,hd_current_day_seek);
			//return -1;
			break;
		case HD_ERR_FULL:
			printf("debug HD_ERR_FULL\n");
			hd_current_sec_seek =first_seek;
			//这应该复制年块了。！！！！！
			//return -1;
			break;
		default:
			printf("unknow err:%d\n",err);
			return err;
			break;
		}
	}
	return 0;
}
/***********************************************************
*功能:获取视频帧数据
*返回：<0错误  >0视频帧的大小
 ***********************************************************/
int get_frame()
{
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
	return BLOCK_ERR_GET_FRAME;
}
inline int get_time()
{
	static unsigned int time=25;
	time++;
	return time/25;
}
/*
 * 测试写入的获取视频帧函数
 */
int test_getframe_gettime()
{
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
