COBJS = $(patsubst %.c,%.o,$(wildcard *.c))
CXXOBJS = $(patsubst %.cpp,%.o,$(wildcard *.cpp))

OBJS = $(COBJS) $(CXXOBJS)
TARGET_OBJS = $(addprefix $(OBJECT_PATH),$(OBJS))
BASENAME = $(shell basename $(CURDIR))

# libhttp.so
TARGET = lib$(BASENAME).so
LIB_TARGET = $(LIB_PATH)/$(TARGET)

CC = gcc
CXX = g++

CFLAGS  = -Wall -O2 -c
CXXFLAGS = -std=c++14 -O2 -Wall -g -c
INCLUDES = -I$(INCLUDE_PATH)
DEP_LIB = $(LIB_PATH)/liblog.so $(LIB_PATH)/libbuffer.so $(LIB_PATH)/libpool.so
LIB = -pthread -llog -lmysqlclient -lbuffer -lpool
LIBS = -L $(LIB_PATH)

.PHONY: all clean test
all : $(LIB_TARGET)

$(LIB_TARGET) : $(OBJS)
	# ar cr $@ $(OBJS)
	$(CXX) -fPIC -shared -o $@ $(OBJS) $(LIB) $(LIBS)
	# rm -f $(OBJS)

%.o:%.cpp
	$(CXX) -fPIC -o $@ $(CXXFLAGS) $< $(INCLUDES)
	cp $@ $(OBJECT_PATH)

%.d: %.cpp
	@set -e; rm -f $@; \
	$(CXX) -MM $(INCLUDES) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

ifneq ($(MAKECMDGOALS),clean)
	include $(OBJS:.o=.d)
endif

clean:
	-rm -f $(TARGET_OBJS) $(LIB_TARGET) *.d *.o