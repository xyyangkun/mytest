#����ʱ�� sudo mount tmpfs /mnt/ramfs -t tmpfs -o size=1024m
#��/mnt/ramfs��д���ļ�
#
#

CC=gcc
all:test
	gcc src/test.c -o test