TOOLCHAIN=/usr/local/bin/hndtools-arm-linux-2.6.36-uclibc-4.5.3
#CROSS_COMPILE=${TOOLCHAIN}/arm-uclibc-linux-2.6.36-
CROSS_COMPILE=
CC=${CROSS_COMPILE}gcc
CXX=${CROSS_COMPILE}g++
STRIP=${CROSS_COMPILE}strip


all: ${CURDIR}/ubus ${CURDIR}/libubox ${CURDIR}/ubus/build/ubus

# Download
${CURDIR}/ubus:
	git clone git://nbd.name/luci2/ubus.git &&\
	cd ${CURDIR}/ubus &&\
	echo 'SET(CMAKE_SYSTEM_NAME Linux)'> CMakeLists.txt.edit &&\
	echo "SET(CMAKE_C_COMPILER \"$(CC)\")" >> CMakeLists.txt.edit &&\
	echo "SET(CMAKE_CXX_COMPILER \"$(CXX)\")" >> CMakeLists.txt.edit &&\
	echo "SET(CMAKE_FIND_ROOT_PATH $(CURDIR))" >> CMakeLists.txt.edit &&\
	echo "SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)" >> CMakeLists.txt.edit &&\
	echo "SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)" >> CMakeLists.txt.edit &&\
	echo "LINK_DIRECTORIES($(CURDIR)/libubox/build $(CURDIR)/lib)" >> CMakeLists.txt.edit &&\
	echo "INCLUDE_DIRECTORIES($(CURDIR))" >> CMakeLists.txt.edit &&\
	cat CMakeLists.txt >> CMakeLists.txt.edit &&\
	mv CMakeLists.txt.edit CMakeLists.txt

${CURDIR}/libubox:
	git clone git://nbd.name/luci2/libubox.git &&\
	cd ${CURDIR}/libubox &&\
	echo 'SET(CMAKE_SYSTEM_NAME Linux)'> CMakeLists.txt.edit &&\
	echo "SET(CMAKE_C_COMPILER \"$(CC)\")" >> CMakeLists.txt.edit &&\
	echo "SET(CMAKE_CXX_COMPILER \"$(CXX)\")" >> CMakeLists.txt.edit &&\
	echo "SET(CMAKE_FIND_ROOT_PATH $(CURDIR))" >> CMakeLists.txt.edit &&\
	echo "SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)" >> CMakeLists.txt.edit &&\
	echo "SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)" >> CMakeLists.txt.edit &&\
	echo "ADD_DEFINITIONS(-D_GNU_SOURCE)" >> CMakeLists.txt.edit &&\
	echo "INCLUDE_DIRECTORIES($(CURDIR)/include)" >> CMakeLists.txt.edit &&\
	cat CMakeLists.txt >> CMakeLists.txt.edit &&\
	mv CMakeLists.txt.edit CMakeLists.txt &&\
	sed --in-place -e 's/\#include <json\/json.h>/\#include <json-c\/json.h>\n\t#include <json-c\/bits.h>/g' ./blobmsg_json.h &&\
	sed --in-place -e 's/\#include <json\/json.h>/\#include <json-c\/json.h>\n\t#include <json-c\/bits.h>/g' ./jshn.c

# Build
${CURDIR}/ubus/build/ubus: $(CURDIR)/libubox/build/libubox.so $(CURDIR)/libubox/build/libblobmsg_json.so
	-mkdir ${CURDIR}/ubus/build
	cd ${CURDIR}/ubus/build &&\
	cmake .. -D"CMAKE_INSTALL_PREFIX=$(CURDIR)" &&\
	make && make install/local

$(CURDIR)/libubox/build/libubox.so $(CURDIR)/libubox/build/libblobmsg_json.so:
	-mkdir ${CURDIR}/libubox/build
	cd ${CURDIR}/libubox/build &&\
	cmake .. -D"CMAKE_INSTALL_PREFIX=$(CURDIR)" &&\
	make && make install/local

clean:
	-rm -rf ${CURDIR}/ubus/build
	-rm -rf ${CURDIR}/libubox/build
