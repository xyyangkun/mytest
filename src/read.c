/*
 * read.c
 *
 *  Created on: 2013-9-27
 *      Author: yangkun
 */
#include "block.h"

int main()
{
	long long blocks;
	int time_begin;//你想要从什么时间开始读
	int ret;
	gtopenlog("hd_read");							//打开日志记录
	if(dh_init()<0)
	{
		perror("dh init");
		gtlogerr("dh init");
		return -1;
	}

	if(hd_getsize(&blocks)<0)
	{
		gtlogerr("get size");
		return -1;
	}else
		gtloginfo("the disk size:%lld\n",blocks);
	block_init();
	if( ( ret= read_disk_print_record_time() ) < 0 )
	{
		printf("read err!!");
		exit(1);
	}
	printf("please input the time:\n");
	scanf("%d", &time_begin);
	printf("time:%d\n",time_begin);


	printf("init fifo\n");
	if(fifo_init()<0)
	{
		gtlogerr("fifo err and exit\n");
		exit(1);
	}
	read_disk(time_begin);


}
