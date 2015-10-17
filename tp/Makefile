# Makefile for Graphics on Linux and generally make.
#
# You should be able to create a makefile project (also known as unmanaged
# makefile) and compile it this way.
#
# Remember to add new object files as you create them!

CC = gcc
TARGET = tp
VPATH = source

_OBJS = main.o tp.o sema.o graphics.o
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
	$(CC) $< -c -o $@ $(FLAGS) $(INCLUDE)
	@echo
	
$(TARGET): $(OBJS)
	$(CC) -o $@ $(FLAGS) $(OBJS) $(INCLUDE) $(LIB_PATH) $(LIBS) $(XLIBS)
	@echo

dsp: DefinitiveSpatial.c
	gcc -std=gnu99 -lm -o ./dsp ./source/DefinitiveSpatial.c
	
sp: spatial_v12.c
	gcc -std=gnu99 -lm -o ./sp ./source/spatial_v12.c
	
lsp: spatial_0.3.4.c
	gcc -std=gnu99 -lm -mcmodel=large -o ./lsp ./source/spatial_0.3.4.c
clean:
	$(RM) $(OBJS)
	$(RM) $(TARGET)
	@echo
