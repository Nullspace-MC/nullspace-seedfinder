CC      = gcc
AR      = ar
ARFLAGS = cr
override LDFLAGS = -lm
override CFLAGS += -Wall -fwrapv

ifeq ($(OS),Windows_NT)
	override CFLAGS += -D_WIN32
	RM = del
else
	override LDFLAGS += -lX11 -pthread
	#RM = rm
endif

.PHONY : all release debug libcubiomes clean

all: release

debug: CFLAGS += -DDEBUG -O0 -ggdb3
debug: libcubiomes
release: CFLAGS += -O3 -march=native
release: libcubiomes find_origin_structures find_double_bases

libcubiomes: CFLAGS += -fPIC
libcubiomes: layers.o generator.o finders.o util.o
	$(AR) $(ARFLAGS) libcubiomes.a $^

find_origin_structures: find_origin_structures.o layers.o generator.o finders.o
	$(CC) -o $@ $^ $(LDFLAGS)

find_origin_structures.o: find_origin_structures.c
	$(CC) -c $(CFLAGS) $<

find_double_bases: find_double_bases.o layers.o generator.o finders.o
	$(CC) -o $@ $^ $(LDFLAGS)

find_double_bases.o: find_double_bases.c
	$(CC) -c $(CFLAGS) $<


xmapview.o: xmapview.c xmapview.h
	$(CC) -c $(CFLAGS) $<

finders.o: finders.c finders.h
	$(CC) -c $(CFLAGS) $<

generator.o: generator.c generator.h
	$(CC) -c $(CFLAGS) $<

layers.o: layers.c layers.h
	$(CC) -c $(CFLAGS) $<

util.o: util.c util.h
	$(CC) -c $(CFLAGS) $<

clean:
	$(RM) *.o libcubiomes.a find_origin_structures find_double_bases

