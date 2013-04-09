include ../common.mk

SRCS =	create_window.c destroy_window.c setflag.c \
	test_mapping.c testctr.c  testctr_ces.c \
	testctr2.c testvd80.c testvd802.c \
	vd80spd-dma.c mm6390-dma.c \
	hsm-dma.c berrtest.c doublemap.c vd80spd.c 
RANLIB=	ranlib

#include /acc/src/dsc/co/Make.auto

CFLAGS		= -Wall -DDEBUG -D_GNU_SOURCE -g -I/root/tsi148vmebridge/include \
		  -I/root/tsi148vmebridge/lib -I./
LIBVMEBUS	= ../$(LIBDIR)/libvmebus.$(CPU).a
LIBVD80		= libvd80.$(CPU).a
LDLIBS		= -lrt

all: $(SRCS:.c=.$(CPU))

.PHONY: all sources doc clean libvd80doc

%.$(CPU).o:	%.c
	$(CC) -o $@ -c $< $(CFLAGS) 

$(LIBVMEBUS):
	$(MAKE) -C ../$(LIBDIR) CPU=$(CPU)

sources: $(SRCS) $(SCRIPTS)

testctr_ces.$(CPU): testctr_ces.c $(LIBVMEBUS)
	$(CC)	-o $@ $^ $(CFLAGS)

testctr2.$(CPU): testctr2.c $(LIBVMEBUS) 
	$(CC)	-o $@ $^ $(CFLAGS)

testvd80.$(CPU): testvd80.c $(LIBVD80) $(LIBVMEBUS)
	$(CC)	-o $@ $^ $(CFLAGS) $(LDLIBS)

testvd802.$(CPU): testvd802.c $(LIBVMEBUS)
	$(CC)	-o $@ $^ $(CFLAGS) $(LDLIBS)

vd80spd.$(CPU): vd80spd.c $(LIBVD80) $(LIBVMEBUS)
	$(CC)	-o $@ $^ $(CFLAGS) $(LDLIBS)

vd80spd-dma.$(CPU): vd80spd-dma.c $(LIBVD80) $(LIBVMEBUS)
	$(CC)	-o $@ $^ $(CFLAGS) $(LDLIBS)

libvd80.$(CPU).o: libvd80.c libvd80.h

libvd80.$(CPU).a: libvd80.$(CPU).o
	$(AR) rv $(LIBVD80) $^
	$(RANLIB) $(LIBVD80)

libvd80doc: libvd80.c libvd80.h vd80.h
	mkdir -p doc
	doxygen libvd80.dox

mm6390-dma.$(CPU): mm6390-dma.c $(LIBVMEBUS)
	$(CC)	-o $@ $^ $(CFLAGS) $(LDLIBS)

hsm-dma.$(CPU): hsm-dma.c $(LIBVMEBUS)
	$(CC)	-o $@ $^ $(CFLAGS) $(LDLIBS)

berrtest.$(CPU): berrtest.c $(LIBVMEBUS)
	$(CC)	-o $@ $^ $(CFLAGS) $(LDLIBS)

doublemap.$(CPU): doublemap.c $(LIBVMEBUS)
	$(CC)	-o $@ $^ $(CFLAGS) $(LDLIBS)

doc: libvd80doc

clean:
	$(RM) *.$(CPU) *.o *.$(CPU).a $(BAKS)
	$(RM) -r doc
