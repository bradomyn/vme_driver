CFLAGS += -I/root/tsi148vmebridge/driver -O0 -m32

CC = gcc

all: libvmebus.a libvmebus.so pp.so

libvmebus.a: libvmebus.o
	ar r $@ $^
libvmebus.so: libvmebus.o
	$(CC) -m32 -shared $^ -o $@
pp.so: pp.o
	$(CC) -m32 -shared $^ -o $@

clean:
	rm -f *.o *.a
