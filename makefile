TARGET = neo

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
	RUNSCRIPT := cmd \/C neo.exe
	RM := rm
else
	PLATFORM = LINUX
	LIBS =
	RM = rm
	RUNSCRIPT := ./$(TARGET)
endif

CFLAGS :=-std=c++11
SRCS := $(wildcard *.cpp)
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
