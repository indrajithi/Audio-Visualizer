//g++ -std=c++11 -c draw.cpp
//g++ -std=gnu++11 finalDraw.o ../kiss_fft130/kiss_fft.c  -L /home/l1f3/mylib/lib/ -lAquila -lOoura_fft -lm -lglut -lGLEW -lGL -lsfml-audio ../common/shader_utils.o -o finalDraw
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glut.h>

#include "common/shader_utils.h"

#include <SFML/Audio.hpp>
//#include <unistd.h>

//#include "aquila/global.h"
#include "aquila/source/WaveFile.h"
//#include <algorithm>
//#include <cstdlib>
#include <iostream>
//#include "aquila/source/generator/SineGenerator.h"
//#include "aquila/transform/FftFactory.h"
//#include "aquila/tools/TextPlot.h"
#include <functional>
#include <memory>

#include <chrono>
#include <thread>
#include <string.h>
#include <sys/time.h>

//kissFFT
#include "kiss_fft130/kiss_fft.h"
#ifndef M_PI
#define M_PI 3.14159265358979324
#endif
