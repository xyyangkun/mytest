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
	printf("init fifo\n");
	if(fifo_init()<0)
	{
		gtlogerr("fifo err and exit\n");
		exit(1);
	}
	read_disk();


}
