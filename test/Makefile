CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = test
OBJS = ../src/log/*.cpp \
       ../src/buffer/*.cpp ../test/*.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $(TARGET)  -lpthread -lmysqlclient -I../include

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)
