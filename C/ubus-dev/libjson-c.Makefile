
TOOLCHAIN=/usr/local/bin/hndtools-arm-linux-2.6.36-uclibc-4.5.3
#CROSS_COMPILE=${TOOLCHAIN}/arm-uclibc-linux-2.6.36-
CROSS_COMPILE=
CC=${CROSS_COMPILE}gcc
CXX=${CROSS_COMPILE}g++
STRIP=${CROSS_COMPILE}strip

all: ${CURDIR}/json-c $(CURDIR)/lib/libjson-c.so

${CURDIR}/json-c:
	git clone https://github.com/json-c/json-c.git

$(CURDIR)/lib/libjson-c.so:
	. ${CURDIR}/../../setenv.sh; \
	cd ${CURDIR}/json-c &&\
	./autogen.sh &&\
	sed --in-place -e 's/AC_FUNC_MALLOC/\#AC_FUNC_MALLOC/g' ./configure.ac &&\
	sed --in-place -e 's/AC_FUNC_REALLOC/\#AC_FUNC_REALLOC/g' ./configure.ac &&\
	./configure CC=${CC} --host=arm-brcm-linux-uclibcgnueabi --prefix=${CURDIR} &&\
	make LDFLAGS=-lm && make install

clean:
	-make -C ${CURDIR}/json-c clean; \
	-rm $(CURDIR)/lib/libjson-c.so
