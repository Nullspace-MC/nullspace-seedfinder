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
debug: libcubiomes find_lower_bits find_origin_quads find_multi_bases find_final_seeds
release: CFLAGS += -O3 -march=native
release: libcubiomes find_lower_bits find_origin_quads find_multi_bases find_final_seeds

libcubiomes: CFLAGS += -fPIC
libcubiomes: layers.o generator.o finders.o util.o
	$(AR) $(ARFLAGS) libcubiomes.a $^

find_lower_bits: find_lower_bits.o layers.o generator.o finders.o
	$(CC) -o $@ $^ $(LDFLAGS)

find_lower_bits.o: find_lower_bits.c
	$(CC) -c $(CFLAGS) $<

find_origin_quads: find_origin_quads.o layers.o generator.o finders.o 
	$(CC) -o $@ $^ $(LDFLAGS)

find_origin_quads.o: find_origin_quads.c
	$(CC) -c $(CFLAGS) $<

find_multi_bases: find_multi_bases.o layers.o generator.o finders.o nullspace_finders.o
	$(CC) -o $@ $^ $(LDFLAGS)

find_multi_bases.o: find_multi_bases.c
	$(CC) -c $(CFLAGS) $<

find_final_seeds: find_final_seeds.o layers.o generator.o finders.o nullspace_finders.o
	$(CC) -o $@ $^ $(LDFLAGS)

find_final_seeds.o: find_final_seeds.c
	$(CC) -c $(CFLAGS) $<


nullspace_finders.o: nullspace_finders.c nullspace_finders.h
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
	$(RM) *.o libcubiomes.a find_lower_bits find_origin_quads find_multi_bases find_final_seeds

