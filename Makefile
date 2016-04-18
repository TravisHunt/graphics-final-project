#!smake
#
#

MACHINE= $(shell uname -s)

ifeq ($(MACHINE),Darwin)
	OPENGL_INC= -FOpenGL -I/usr/X11R6/include
	OPENGL_LIB= -framework OpenGL -framework GLUT -framework Cocoa
else
	OPENGL_INC= -I/usr/lib64 -I/usr/include
	OPENGL_LIB= -L/usr/lib64 -lglut -lGLU -lGL -lXmu -lXext -lX11 -lm
endif

CXX=g++
COMPILER_FLAGS= -g -Wno-deprecated-declarations

INCLUDE= $(OPENGL_INC)
LLDLIBS= $(OPENGL_LIB) -I ./libs/

TARGETS = sketching

default : $(TARGETS)

%.o: %.cpp
	$(CXX) -c -o $@ $(COMPILER_FLAGS)  $< $(INCLUDE)

sketching: sketching.cpp view.o
	$(CXX) $(COMPILER_FLAGS) $(LLDLIBS) $^ -o $@

run:
	./sketching

test: derp.cpp
	$(CXX) $(COMPILER_FLAGS) $(LLDLIBS) $^ -o $@ 

clean:
	rm -f *.o $(TARGETS) *~ .*.swp .*.swo
	rm -rf *.dSYM
