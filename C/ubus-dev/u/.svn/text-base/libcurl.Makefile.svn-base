TOOLCHAIN=/usr/local/bin/hndtools-arm-linux-2.6.36-uclibc-4.5.3
CC=${TOOLCHAIN}/bin/arm-brcm-linux-uclibcgnueabi-gcc
CXX=${TOOLCHAIN}/bin/arm-brcm-linux-uclibcgnueabi-g++
STRIP=${TOOLCHAIN}/bin/arm-brcm-linux-uclibcgnueabi-strip


all: ${CURDIR}/curl-curl-7_37_0 ${CURDIR}/curl-curl-7_37_0/lib/.libs/libcurl.so


# Download
${CURDIR}/curl-7_37_0.tar.gz:
	wget https://github.com/bagder/curl/archive/curl-curl-7_37_0.tar.gz

${CURDIR}/curl-curl-7_37_0: ${CURDIR}/curl-7_37_0.tar.gz
	tar xzvf curl-7_37_0.tar.gz


# Build
${CURDIR}/curl-curl-7_37_0/lib/.libs/libcurl.so:
	. ${CURDIR}/../../setenv.sh;
	cd curl-curl-7_37_0 &&\
	./buildconf &&\
	./configure CC=${CC} --host=arm-brcm-linux-uclibcgnueabi &&\
	make &&\
	cd .. &&\
	mkdir -p include/curl &&\
	cp curl-curl-7_37_0/lib/.libs/libcurl.so ./ &&\
	cp curl-curl-7_37_0/include/curl/*.h include/curl/

clean:
	-make -C curl-curl-7_37_0 clean
