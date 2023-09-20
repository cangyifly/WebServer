COBJS = $(patsubst %.c,%.o,$(wildcard *.c))
CXXOBJS = $(patsubst %.cpp,%.o,$(wildcard *.cpp))

OBJS = $(COBJS) $(CXXOBJS)
OBJS = $(CXXOBJS)
BASENAME = $(shell basename $(CURDIR))

# libbuffer.so
TARGET = lib$(BASENAME).so

CC = gcc
CXX = g++

CFLAGS  = -Wall -O2 -c
CXXFLAGS = -std=c++14 -O2 -Wall -g -c
INCLUDES = -I$(INCLUDE_PATH)
LIB =
LIBS = $(LIB_PATH)

.PHONY: all clean test
all : $(TARGET)

$(TARGET) : $(OBJS)
	rm -f $(LIB_PATH)/$@
	# ar cr $@ $(OBJS)
	$(CXX) -fPIC -shared -o $(LIB_PATH)/$(TARGET) $(patsubst %.o,$(OBJECT_PATH)/%.o, $(OBJS))
	# rm -f $(OBJS)

%.o:%.cpp
	$(CXX) -fPIC -o $@ $(CXXFLAGS) $< $(INCLUDES)
	mv $@ $(OBJECT_PATH)

%.d: %.cpp
	@set -e; rm -f $@; \
	$(CXX) -MM $(INCLUDES) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

ifneq ($(MAKECMDGOALS),clean)
	include $(OBJS:.o=.d)
endif

clean:
	-rm -f $(OBJECT_PATH)/$(OBJS) $(LIB_PATH)/$(TARGET) *.d