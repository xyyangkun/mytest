#测试时： sudo mount tmpfs /mnt/ramfs -t tmpfs -o size=1024m
#在/mnt/ramfs中写大文件
#
#

CC=gcc
all:test
	gcc src/test.c -o test