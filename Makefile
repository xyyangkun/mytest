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
CFLAGS += -Isrc -L$(PWD)/lib -lsda -l sgutils2 -l media_api -lpthread
endif

all: block.o main.o
	$(CC)  $(OUT)/block.o $(OUT)/main.o $(CFLAGS)  -o  $(OUT)/main
	cp $(OUT)/main /mnt/yk
	
block.o: src/block.c src/block.h
	$(CC) $(CFLAGS) -c  src/block.c -o $(OUT)/block.o
	
main.o: src/main.c src/block.h
	$(CC) $(CFLAGS) -c src/main.c -o $(OUT)/main.o
	
clean:
	rm $(OUT)/*