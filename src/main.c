/*
 * main.c
 *
 *  Created on: 2013-9-12
 *      Author: yangkun
 */
#include "block.h"
/************************************************************
 * �������ã�������д��Ӳ��
 * ����bug��Ӳ�̴�С�����ܴ�һ�������
 *
 ************************************************************/
//#define STDTEST
int main()
{
	long long blocks;
	static char buf[512];
	int tmp;
	gtopenlog("hd_write");							//����־��¼

	//test_print_size();
	if(dh_init()<0)
	{
		perror("dh init");
		gtlogerr("dh init");
		return -1;
	}
#if 1
	if(hd_getsize(&blocks)<0)
	{
		gtlogerr("get size");
		return -1;
	}else
		gtloginfo("the disk size:%lld\n",blocks);

#ifdef STDTEST
	//����һ���Ľṹ
	for(tmp=0; tmp < 512; tmp++)
	{
		buf[tmp]=tmp;
	}
	//д
	for(tmp = 0; tmp < blocks; tmp++)
	{
		if(hd_write(buf, sizeof(buf), 512, tmp) < 0 )
		{
			printf("write error\n");
			return -1;
		}
	}
	printf("write ok\n");
	//��
	char buf1[512];
	for(tmp = 0; tmp < blocks; tmp++)
	{
		if(hd_read(buf1, sizeof(buf1), 512, tmp) < 0 )
		{
			printf("write error\n");
			return -1;
		}
		//У��
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
#endif
	//test_getframe_gettime();
	//test_write_read();
#if 1
	pthread_t ntid;
	extern void test_secseekofday(void *arg);
	if(pthread_create(&ntid, NULL, test_secseekofday, "new thread: ")<0)
	{
		printf("create pthread err!!\n");
		return -1;
	}
#endif
	write_disk();
}
