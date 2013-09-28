#测试时： sudo mount tmpfs /mnt/ramfs -t tmpfs -o size=1024m
#sudo mount -t ramfs -o size=1000M,maxsize=1100M ramfs /mnt/ramfs
#sudo dd if=/dev/zero of=a.img bs=1000M count=1
#在/mnt/ramfs中写大文件
#1048576000
#

OUT=$(PWD)/debug
ifdef TEST_RAM
CC=gcc
#CFLAGS += -D YEARBLOCKTEST
#CFLAGS += -D STDTEST
else
CC=arm-hisiv200-linux-gcc
CFLAGS += -g
CFLAGS += -Isrc -Iextern_src/include -L$(PWD)/lib -lsda -l sgutils2 -l media_api -lgtlog -lpthread 
endif
all:main read
	cp $(OUT)/main /mnt/yk
	cp $(OUT)/read /mnt/yk
main: block.o main.o
	$(CC)  $(OUT)/block.o $(OUT)/main.o $(CFLAGS)  -o  $(OUT)/main

	
block.o: src/block.c src/block.h
	$(CC) $(CFLAGS) -c  src/block.c -o $(OUT)/block.o
	
main.o: src/main.c src/block.h
	$(CC) $(CFLAGS) -c src/main.c -o $(OUT)/main.o
	
read:read.o block.o
	$(CC)  $(OUT)/block.o $(OUT)/read.o $(CFLAGS)  -o  $(OUT)/read
	
read.o:
	$(CC) $(CFLAGS) -c src/read.c -o $(OUT)/read.o
clean:
	rm $(OUT)/*