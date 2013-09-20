/*
 * main.c
 *
 *  Created on: 2013-9-12
 *      Author: yangkun
 */
#include "block.h"

//#define STDTEST
int main()
{
	long long blocks;
	static char buf[512];
	int tmp;
	if(dh_init()<0)
	{
		perror("dh init");
		return -1;
	}
	if(hd_getsize(&blocks)<0)
	{
		perror("get size");
		return -1;
	}else
		printf("the disk size:%lld\n",blocks);

#ifdef STDTEST
	//构建一定的结构
	for(tmp=0; tmp < 512; tmp++)
	{
		buf[tmp]=tmp;
	}
	//写
	for(tmp = 0; tmp < blocks; tmp++)
	{
		if(hd_write(buf, sizeof(buf), 512, tmp) < 0 )
		{
			printf("write error\n");
			return -1;
		}
	}
	printf("write ok\n");
	//读
	char buf1[512];
	for(tmp = 0; tmp < blocks; tmp++)
	{
		if(hd_read(buf1, sizeof(buf1), 512, tmp) < 0 )
		{
			printf("write error\n");
			return -1;
		}
		//校验
		if(memcmp(buf, buf1, 512)!=0)
		{
			printf("blcoks:%d",tmp);
			printf("check error!\n");
			return -1;
		}
	}
	printf("check ok\n");
#endif //STDTEST
	block_init();
	test_year_data();
}
