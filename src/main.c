/*
 * main.c
 *
 *  Created on: 2013-9-12
 *      Author: yangkun
 */
#include "block.h"

int main()
{
	long long seek;
	if(dh_init()<0)
	{
		perror("dh init");
		return -1;
	}
	if(hd_getsize(&seek)<0)
	{
		perror("get size");
		return -1;
	}else
		printf("the disk size:%lld",seek);



}
