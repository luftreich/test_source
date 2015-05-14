# 生成的可执行文件模块名
BIN = ping

# 生成的库文件模块名
LIB_STATIC =
LIB_DYNAMIC =

# 本模块需要的头文件目录
INCLUDE_DIRS = $(ROOT)/include \
			   $(ROOT)/include/common \
			   $(ROOT)/include/msg \
			   $(ROOT)/modules/msglib/include \
			   $(ROOT)/modules/network/include

# 本模块需要链接的库名
# 注意: 形如 libxxx.a(so) 的库只写 xxx
# 空格分隔不同的库
LINK_LIBS = pthread  
include $(ROOT)/make.rule
