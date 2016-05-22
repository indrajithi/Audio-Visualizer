LDLIBS=-lm -lglut -lGLEW -lGL -lfreetype
CXXFLAGS=-I/usr/include/freetype2
all: text2
clean:
	rm -f *.o text2
text: common/shader_utils.o
.PHONY: all clean
