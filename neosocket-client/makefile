TARGET = client

CC = g++
MACHINE = $(shell $(CC) -dumpmachine)
# machine should be as follow:
#   x86_64-linux-gnu
#   mingw32
ifneq (,$(or $(findstring mingw, $(MACHINE)), $(findstring cygwin, $(MACHINE))))
	PLATFORM = WIN
	LIBS = -lwsock32 
	RM = del
	TARGET := $(TARGET).exe
	RUNSCRIPT := cmd \/C $(TARGET)
	RM := rm
else
	PLATFORM = LINUX
	LIBS =
	RM = rm
	RUNSCRIPT := ./$(TARGET)
endif
#-fpermissive -Wall
CFLAGS :=-std=c++11 -I../common
SRCS := $(wildcard *.cpp)
SRCS += $(wildcard ../common/*.cpp)

OBJS := $(SRCS:.cpp=.o)

all:$(TARGET)

%.o: %.cpp
	@echo --[CC]-- $@
	$(CC) -c -o"$@" "$<" $(CFLAGS)

$(TARGET): $(OBJS)
	@echo --[TARGET]-- $@
	$(CC) -o "$(TARGET)" $(OBJS) $(LIBS) $(CFLAGS)
	@echo [Finished]

clean:
	-$(RM) $(OBJS) $(TARGET)

run:
	@echo --[RUN]--
	$(RUNSCRIPT)
	@echo --[Finished RUN]--

.PHONY: all clean run
