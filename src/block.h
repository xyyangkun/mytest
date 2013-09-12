/*
 * block.h
 *
 *  Created on: 2013-9-12
 *      Author: yangkun
 */

#ifndef BLOCK_H_
#define BLOCK_H_
/*Ӳ��Ĭ�Ͽ��С512�ֽ�  ���ֵ����˵�ǲ��ܸĵ�*/
#define BLOCKSIZE 512

/*����������ܹ���800��*/
#define MAXDAY 800
/*��ѭ��������ͷ��ƫ��*/
#define YEAR_OFFSET 8
/*��ѭ��������ͷ��ƫ��*/
#define DATE_OFFSET 8

#ifdef DEBUG_LOG
	/*60*12/512=2*/
	#define YEAR_HEAD_BLOCK_SIZE 20
	#define DATE_HEAD_BLOCK_SIZE 2
/*���Գ����߼�ʱ������Ӳ����100000*512��Ҳ��50M������ô��4Mbit���ʿ���д100S��Ҳ����һ���һ����*/
#define MAXBLOCKS 100000
#else
/*
 * ��ʾ�������ռ20�飬��ʾ������ݵ�2026��
 * ���800�� 20->800*12/512
 * 2026->24*3600*12/512
 */
	#define YEAR_HEAD_BLOCK_SIZE 20
	#define DATE_HEAD_BLOCK_SIZE 2026
/*160G��Ӳ������ô�����*/
#define MAXBLOCKS 312581808
#endif




extern media_source_t media;
#define BUFFER_SIZE 400*1024 //���4M������֡Ӧ�ò��ᳬ��100K





/******************************************************************
 * ���
 * *****************************************************************/
struct year_block{ int time; long long seek;}__attribute__ ((packed));






#endif /* BLOCK_H_ */
