###############################################
# 说明: 递归编译全部模块		          	  #
# 	本文件不要轻易修改						  #
# 				  							  #
# 作者: xuhuilin 							  #
# 版本: 1.0								      #
# 日期: 2013-12-03							  #
###############################################

.PHONY: all clean

# 按照目录先后顺序进行构建, 每一行指向该模块的makfile
DIRS = $(ROOT)/cjson 

RM = rm
RMFLAGS = -fr
RMS = $(ROOT)/build/bin $(ROOT)/build/lib

all:
	@set -e;\
	for dir in $(DIRS);\
	do \
		cd $$dir && $(MAKE); \
	done
	@echo ""
	@echo -e "\033[32m:-) Great! Build successfully!\033[0m"
	@echo ""

release:
	@set -e;\
	for dir in $(DIRS);\
	do \
		cd $$dir && $(MAKE) COMPILE_MODE=release; \
	done
	@echo ""
	@echo -e "\033[32m:-) Great! Build successfully!\033[0m"
	@echo ""

clean:
	@set -e;\
	for dir in $(DIRS);\
	do \
		cd $$dir && $(MAKE) clean; \
	done
	$(RM) $(RMFLAGS) $(RMS)
	@echo ""
	@echo -e "\033[33mClean Completed.\033[0m"
	@echo ""

