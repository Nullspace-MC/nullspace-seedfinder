CC      = gcc
AR      = ar
ARFLAGS = cr
override LDFLAGS = -lm
override CFLAGS += -Wall -fwrapv -march=native
#override CFLAGS += -DUSE_SIMD

ifeq ($(OS),Windows_NT)
	override CFLAGS += -D_WIN32
	RM = del
else
	override LDFLAGS += -lX11 -pthread
	#RM = rm
endif

.PHONY : all debug libcubiomes clean

all: CFLAGS += -O3 -march=native
all: libcubiomes find_origin_structures clean

debug: CFLAGS += -DDEBUG -O0 -ggdb3
debug: find_origin_structures clean

libcubiomes: CFLAGS += -O3 -fPIC
libcubiomes: layers.o generator.o finders.o util.o
	$(AR) $(ARFLAGS) libcubiomes.a $^

find_origin_structures: find_origin_structures.o layers.o generator.o finders.o
	$(CC) -o $@ $^ $(LDFLAGS)

find_origin_structures.o: find_origin_structures.c
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
	$(RM) *.o libcubiomes.a find_origin_structures

