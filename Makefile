#测试时： sudo mount tmpfs /mnt/ramfs -t tmpfs -o size=1024m
#sudo mount -t ramfs -o size=1000M,maxsize=1100M ramfs /mnt/ramfs
#sudo dd if=/dev/zero of=a.img bs=1000M count=1
#在/mnt/ramfs中写大文件
#
#

OUT=$(PWD)/debug
CC=gcc
CFLAGS += -g
CFLAGS += -D YEARBLOCKTEST
#CFLAGS += -D STDTEST
all: block.o main.o
	$(CC)  $(OUT)/block.o $(OUT)/main.o $(CFLAGS)  -o  $(OUT)/main
	
block.o: src/block.c src/block.h
	$(CC) $(CFLAGS) -c  src/block.c -o $(OUT)/block.o
	
main.o: src/main.c src/block.h
	$(CC) $(CFLAGS) -c src/main.c -o $(OUT)/main.o
	
clean:
	rm $(OUT)/*