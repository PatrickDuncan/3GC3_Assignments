LDFLAGS = -lGL -lGLU -lglut
CFLAGS=-g -Wall -std=c++11
CC=g++
EXEEXT=
RM=rm

# Windows (cygwin)
ifeq "$(OS)" "Windows_NT"
	EXEEXT=.exe
	RM=del
    LDFLAGS = -lfreeglut -lglu32 -lopengl32
else
# OS X
	OS := $(shell uname)
	ifeq ($(OS), Darwin)
	        LDFLAGS = -framework Carbon -framework OpenGL -framework GLUT
	endif
endif

MODELLER = Modeller.x

all: modeller

modeller: $(MODELLER)
	./$(MODELLER)$(EXEEXT)

$(MODELLER): Modeller.o SceneObject.o
	$(CC) -Wno-deprecated -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	$(RM) *.o $(MODELLER)$(EXEEXT)
