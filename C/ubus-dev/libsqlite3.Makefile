TOOLCHAIN=/usr/local/bin/hndtools-arm-linux-2.6.36-uclibc-4.5.3
CC=${TOOLCHAIN}/bin/arm-brcm-linux-uclibcgnueabi-gcc
CXX=${TOOLCHAIN}/bin/arm-brcm-linux-uclibcgnueabi-g++
STRIP=${TOOLCHAIN}/bin/arm-brcm-linux-uclibcgnueabi-strip


all: ${CURDIR}/sqlite-autoconf-3080403/.libs/libsqlite3.so


# Download
${CURDIR}/sqlite-autoconf-3080403.tar.gz:
	wget http://www.sqlite.org/2014/sqlite-autoconf-3080403.tar.gz

${CURDIR}/sqlite-autoconf-3080403: ${CURDIR}/sqlite-autoconf-3080403.tar.gz
	tar xzvf sqlite-autoconf-3080403.tar.gz


# Build
${CURDIR}/sqlite-autoconf-3080403/.libs/libsqlite3.so: ${CURDIR}/sqlite-autoconf-3080403
	. ${CURDIR}/../../setenv.sh;
	cd sqlite-autoconf-3080403 &&\
	./configure CC=${CC} --host=arm-brcm-linux-uclibcgnueabi &&\
	make &&\
	cd .. &&\
	cp sqlite-autoconf-3080403/.libs/libsqlite3.so ./ &&\
	cp sqlite-autoconf-3080403/sqlite3*.h include

clean:
	-make -C sqlite-autoconf-3080403 clean
