/*
 * read.c
 *
 *  Created on: 2013-9-27
 *      Author: yangkun
 */
#include "block.h"
/*0.01 基本测试可以读出正常。
 *
 *
 */
#define READ_VERSION "0.01"
int main()
{
	long long blocks;
	int time_begin;//你想要从什么时间开始读
	int ret;
	gtopenlog("hd_read");							//打开日志记录
	printf("VERSION:%s\n",READ_VERSION);
	gtloginfo("VERSION:%s\n\n\n",READ_VERSION);
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
