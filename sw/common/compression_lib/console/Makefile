PROJECT = heatshrink
OPTIMIZE = -O3
WARN = -Wall -Wextra -pedantic #-Werror
WARN += -Wmissing-prototypes
WARN += -Wstrict-prototypes
WARN += -Wmissing-declarations

# If libtheft is available, build additional property-based tests.
# Uncomment these to use it in test_heatshrink_dynamic.
#CFLAGS += -DHEATSHRINK_HAS_THEFT
#THEFT_PATH=	/usr/local/
#THEFT_INC=	-I${THEFT_PATH}/include/
#LDFLAGS += -L${THEFT_PATH}/lib -ltheft

CFLAGS += -std=c99 -g ${WARN} ${THEFT_INC} ${OPTIMIZE}

all: heatshrink

clean:
	rm -rf heatshrink *.o *.os *.od *.core *.a {dec,enc}_sm.png TAGS

# Internal targets and rules

OBJS = ../common/heatshrink_encoder.o ../common/heatshrink_decoder.o

STATIC_OBJS=  $(OBJS:.o=.os)

STATIC_LDFLAGS= ${LDFLAGS} -L. -lheatshrink_static

CFLAGS_STATIC = ${CFLAGS} -DHEATSHRINK_DYNAMIC_ALLOC=0

heatshrink: heatshrink.os  ../decompress/deCompressorTypeA.os ../compress/CompressorTypeA.os ../common/delta.os ../common/rle.os ../common/configuration.os libheatshrink_static.a
	${CC} -o $@ $^ ${CFLAGS_STATIC} -L. -lheatshrink_static

libheatshrink_static.a: ${STATIC_OBJS}
	ar -rcs $@ $^

%.os: %.c
	${CC} -c -o $@ $< ${CFLAGS_STATIC}

*.os: Makefile *.h

