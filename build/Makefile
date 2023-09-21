# CXX = g++
# CFLAGS = -std=c++14 -O2 -Wall -g 

# .PHONY: all clean test cpin

# TARGET = webserver
# OBJS = ../src/log/*.cpp ../src/pool/*.cpp ../src/timer/*.cpp \
#        ../src/http/*.cpp ../src/server/*.cpp \
#        ../src/buffer/*.cpp ../src/main.cpp

# all: $(OBJS)
# 	$(CXX) $(CFLAGS) $(OBJS) -o ../bin/$(TARGET)  -pthread -lmysqlclient -I$(INCLUDE_PATH)

# clean:
# 	rm -rf $(TARGET) $(INCLUDE_PATH)/*.h $(OBJECT_PATH)/*.o $(LIB_PATH)/*.so

# cpin:
# 	cp ../src/*/*.h ../include

# 当前项目路径
PROJECT_PATH = $(shell realpath $(dir $(PWD)))
# 当前build目录
BUILD_PATH = $(PROJECT_PATH)/build
# src目录
SRC_PATH = $(PROJECT_PATH)/src
# include目录
INCLUDE_PATH = $(PROJECT_PATH)/include
# lib目录
LIB_PATH = $(PROJECT_PATH)/lib
# object目录
OBJECT_PATH = $(PROJECT_PATH)/object
# bin目录
BIN_PATH = $(PROJECT_PATH)/bin
# log目录
LOG_PATH = $(PROJECT_PATH)/log

export PROJECT_PATH
export BUILD_PATH
export SRC_PATH
export INCLUDE_PATH
export LIB_PATH
export OBJECT_PATH
export BIN_PATH


# lib文件下模块
LIB_DIR_NAME = buffer log timer pool http server
## src下的模块路径
SRC_MODULE = $(addprefix $(SRC_PATH)/,$(LIB_DIR_NAME))
# 目标目录
TARGET_PATH = $(SRC_PATH)
# # 链接使用的动态库
# # LIB = $(addprefix $(addsuffix /,$(LIB_FILE_PATH)),$(addprefix lib,$(addsuffix .so,$(LIB_DIR_NAME))))
LIB = libserver.so

# # 安装命令
# INSTALL_FILE = install -m 644 -p

CC = gcc
CXX = g++
LINK = g++

CFLAGS  = -Wall -O2 -c
CXXFLAGS = -std=c++14 -O2 -Wall -g -c
INCLUDES = -I$(INCLUDE_PATH)
DEP_LIB = $(LIB_PATH)/libserver.so
LIB = -lserver
LIBS = -L $(LIB_PATH)

COBJS = $(patsubst %.c,%.o,$(wildcard $(TARGET_PATH)/*.c))
CXXOBJS = $(patsubst %.cpp,%.o,$(wildcard $(TARGET_PATH)/*.cpp))

OBJS = $(COBJS) $(CXXOBJS)
TARGET = webserver
BIN_TARGET = $(BIN_PATH)/$(TARGET)

.PHONY: all clean install test header

all : $(BIN_PATH)/$(TARGET)

$(BIN_TARGET): $(OBJS) $(DEP_LIB)
	$(LINK) $(INCLUDES) $(OBJS) -o $@ $(LIB) $(LIBS)

%.o: %.c
	$(CC) -o $@ $(CFLAGS) $< $(INCLUDES)
	cp $@ $(OBJECT_PATH)

%.o: %.cpp
	$(CXX) -o $@ $(CXXFLAGS) $< $(INCLUDES)
	cp $@ $(OBJECT_PATH)

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -MM $(INCLUDES) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.d: %.cpp
	@set -e; rm -f $@; \
	$(CXX) -MM $(INCLUDES) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(DEP_LIB) :
	@for subdir in $(SRC_MODULE); do \
		make -C $$subdir; \
	done


-include $(OBJS:.o=.d)

install :
	-$(INSTALL_FILE) main ../

clean:
	-rm -f $(OBJS) $(BIN_TARGET) $(TARGET_PATH)/*.d.* $(TARGET_PATH)/*.d $(TARGET_PATH)/*.o
	-rm -f $(INCLUDE_PATH)/*.*
	@for subdir in $(SRC_MODULE); do \
		make clean -C $$subdir; \
	done

test:
	for subdir in $(SRC_MODULE); do \
		echo $$subdir; \
		cp -rf $$subdir/*.h $(INCLUDE_PATH); \
	done
	# @echo $(SRC_MODULE)
	# @echo $(PWD)
	# @echo $(PROJECT_PATH)

header:
	@find $(SRC_PATH) -name "*.h" -type f -exec cp {} $(INCLUDE_PATH) \;


