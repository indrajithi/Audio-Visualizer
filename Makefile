LDLIBS=-lm -lglut -lGLEW -lGL
all: graph
clean:
	rm -f *.o graph
graph: ../common/shader_utils.o
.PHONY: all clean
