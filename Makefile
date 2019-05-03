# main make file

# library directories
LDIR=-Llib -Ilib

# libraries
ifdef SYSTEMROOT
	#windows libraries
	LIBS= -lglfw3_win -lgdi32 -lopengl32
else
	#linux libraries
	LIBS= -lglfw3_linux -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm -lXxf86vm -lXinerama -lXcursor -lrt
endif

IDIR=src
CC=gcc
CFLAGS=-Wall -msse3 -g -I$(IDIR) $(LDIR) $(LIBS)

# hide .o files in obj directory
ODIR=obj

_DEPS = camera.h sprite.h shader.h texman.h circle.h list.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o shader.o sprite.o glad.o camera.o texman.o circle.o list.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

# tells make to check include directory for dependencies
# without this, you get a 'cannot find name.o' error
VPATH = $(IDIR)

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ models/**/*.vrt

