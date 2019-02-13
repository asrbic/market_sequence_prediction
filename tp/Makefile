# Makefile for Graphics on Linux and generally make.
#
# You should be able to create a makefile project (also known as unmanaged
# makefile) and compile it this way.
#
# Remember to add new object files as you create them!

CC = gcc
TARGET = tp
VPATH = source

_OBJS = sema.o graphics.o tp.o main.o 
OBJDIR =  ./

# Optional to build examples
# VPATH += Examples/Simple
# _OBJS += Simple.o

OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS))
#INCLUDE = -I./Headers -I./Headers/Linux

FLAGS = -std=gnu99 -g3 -Wall -O0 -DDEBUG -fmessage-length=0 
LIBS = -ldl -lpthread -lSDL -lSDLmain -lSDL_ttf -lSDL_image -lm
#LIB_PATH = -L./Libraries/

all: $(TARGET)

.SUFFIXES:
.SUFFIXES: .c .o

$(OBJDIR)/%.o: %.c
	$(CC) $< -c -o $@ $(FLAGS) $(LIBS) $(INCLUDE)
	@echo
	
$(TARGET): $(OBJS)
	$(CC) -o $@ $(FLAGS) $(OBJS) $(INCLUDE) $(LIB_PATH) $(LIBS) $(XLIBS)
	@echo

clean:
	$(RM) $(OBJS)
	$(RM) $(TARGET)
	@echo
