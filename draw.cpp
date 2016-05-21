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
#include <unistd.h>

#include "aquila/global.h"
#include "aquila/source/WaveFile.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include "aquila/source/generator/SineGenerator.h"
#include "aquila/transform/FftFactory.h"
#include "aquila/tools/TextPlot.h"
#include <functional>
#include <memory>

<<<<<<< HEAD
//#include <chrono>
//#include <thread>
#include <string.h>

//kissFFT
#include "kiss_fft130/kiss_fft.h"
#ifndef M_PI
#define M_PI 3.14159265358979324
#endif

=======
#include <chrono>
#include <thread>
#include <string.h>

//kissFFT
#include "kiss_fft130/kiss_fft.h"
#ifndef M_PI
#define M_PI 3.14159265358979324
#endif

>>>>>>> develop
#define N 2048
#include <sys/time.h>
typedef unsigned long long timestamp_t;
  static timestamp_t
    get_timestamp ()
    {
      struct timeval now;
      gettimeofday (&now, NULL);
      return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
    }

GLuint program;
GLint attribute_coord1d;
GLint uniform_offset_x;
GLint uniform_scale_x;
GLuint texture_id;
GLint uniform_mytexture;

float offset_x = 0.0;
float scale_x = 1.0/(1.5*10)/(1.5*6);

bool interpolate = false;
bool clamp = false;
bool showpoints = true;

GLuint vbo;
GLbyte graph[N/2]; 
double framePointer;
char fileName[50];
bool calledFromInit = true;
bool dataEnd = false;

kiss_fft_cpx in[N],out[N];


void getData();
<<<<<<< HEAD
=======
void display();

>>>>>>> develop
void getFft(const kiss_fft_cpx in[N], kiss_fft_cpx out[N])
{
  kiss_fft_cfg cfg;

 

  if ((cfg = kiss_fft_alloc(N, 0/*is_inverse_fft*/, NULL, NULL)) != NULL)
  {
    size_t i;

    kiss_fft(cfg, in, out);
    free(cfg);

   } 
  else
  {
    printf("not enough memory?\n");
    exit(-1);
  }

}


<<<<<<< HEAD
void getData()
{
	timestamp_t t0 = get_timestamp();

	int i,j,x;
	Aquila::WaveFile wav(fileName);
	double mag[N/2];
	
	//Get first 2048 samples
	for( i = framePointer, j = 0; i < (framePointer + N)
										 && framePointer < wav.getSamplesCount(); i++,j++  ){

		in[j].r = (float)wav.sample(i), in[j].i = 0;  //stores 2048 samples

	}

	framePointer = i;
	if(framePointer >= wav.getSamplesCount())
		dataEnd = true; 

	getFft(in,out);

	// calculate magnitude of first n/2 FFT
	for(i = 0; i < N/2; i++ )
		mag[i] = sqrt((out[i].r * out[i].r) + (out[i].i * out[i].i));

	// N/2 Log magnitude values.
	for (i = 0; i < N/2 ; ++i){
		x =   20 * log10(mag[i]) ;
		graph[i] = log(mag[i]);	
	}
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 2048, 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, graph);
//Executes from second call
	if(!calledFromInit)
	{
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 2048, 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, graph);

	// Create the vertex buffer object
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// Create an array with only x values.
	GLfloat line[101];

=======
void moveWav()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	//offset_x -= 0.07;	
	//if(offset_x < -1.0)
		offset_x = 0.0;

	getData();
	display();
	glFlush();
	glutSwapBuffers();
}

float windoFunction(float freq)
{
	float a = 0.54, b = 0.46;
	return a - b * cos((2*M_PI)/(freq)-1);

}
int graphPtr = 0;
int tmpGraph[N/2];
int magN(int n)
{	
	int max = tmpGraph[0];
	for(int i=1; i<n; i++)
	{
		graph[i]>max;
		max = tmpGraph[i];

	}
	
	graphPtr ++;
	return max;
}

void getData()
{
	timestamp_t t0 = get_timestamp();

	int i,j,x;
	Aquila::WaveFile wav(fileName);
	double mag[N/2];
	double roof = wav.getSamplesCount();
	//Get first 2048 samples
	for( i = framePointer, j = 0; i < (framePointer + N)
										 && framePointer < roof; i++,j++  ){

		in[j].r = windoFunction((wav.sample(i)));
		in[j].i = 0;  //stores 2048 samples

	}
	if(framePointer<roof){
		framePointer = i;

	}
	else 
		exit(0);

	if(framePointer >= wav.getSamplesCount()){
		dataEnd = true; 
		return ;
	}
	std::cout<<"Framepointer = "<<framePointer<<std::endl;
	getFft(in,out);

	// calculate magnitude of first n/2 FFT
	for(i = 0; i < N/2; i++ )
		mag[i] = sqrt((out[i].r * out[i].r) + (out[i].i * out[i].i));
	
	// N/2 Log magnitude values.
	for (i = 0; i < N/2 ; ++i){
		x =   20 * log10(mag[i]) ;
		graph[i] = log(x);	
	}
	//graph [graphPtr]=magN(tmpGraph,i);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 2048, 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, graph);
//Executes from second call
	if(!calledFromInit)
	{
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 2048, 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, graph);

	// Create the vertex buffer object
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// Create an array with only x values.
	GLfloat line[101];

>>>>>>> develop
	// Fill it in just like an array
	for (int i = 0; i < 101; i++) {
		line[i] = (i - 50) / 50.0;
	}

	// Tell OpenGL to copy our array to the buffer object
	glBufferData(GL_ARRAY_BUFFER, sizeof line, line, GL_STATIC_DRAW);

	// Enable point size control in vertex shader
#ifndef GL_ES_VERSION_2_0
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#endif
	} 

/*	for (std::size_t i = 5805, j = 0; i<6805; i++ )
    {	
    	    float x = (wav.sample(i) - 1024.0) / 100.0;
    	    float y = sin(x * 10.0) / (1.0 + x * x);

    	    graph[j++] = roundf(y * 128 + 128);
    }

*/	timestamp_t t1 = get_timestamp();
	double secs = (t1 - t0) / 1000000.0L;

	std::cout<<"getdata total time: getData"<<secs<<std::endl;

	//std::cout<<"getdata total time: "<<secs<<std::endl;


}

int init_resources() {

	timestamp_t t0 = get_timestamp();
	program = create_program("graph.v.glsl", "graph.f.glsl");
	if (program == 0)
		return 0;

	attribute_coord1d = get_attrib(program, "coord1d");
	uniform_offset_x = get_uniform(program, "offset_x");
	uniform_scale_x = get_uniform(program, "scale_x");
	uniform_mytexture = get_uniform(program, "mytexture");

	if (attribute_coord1d == -1 || uniform_offset_x == -1 || uniform_scale_x == -1 || uniform_mytexture == -1)
		return 0;

	// Create our datapoints, store it as bytes
	//GLbyte graph[2048];  thi is made as global

	/* these points are not used
	for (int i = 0; i < 2048; i++) {
		float x = (i - 1024.0) / 100.0;
		float y = sin(x * 10.0) / (1.0 + x * x);

		graph[i] = roundf(y * 128 + 128);
	} */
	getData();
	calledFromInit = !calledFromInit;
	/* Upload the texture with our datapoints */
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 2048, 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, graph);

	// Create the vertex buffer object
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// Create an array with only x values.
	GLfloat line[101];

	// Fill it in just like an array
	for (int i = 0; i < 101; i++) {
		line[i] = (i - 50) / 50.0;
	}

	// Tell OpenGL to copy our array to the buffer object
	glBufferData(GL_ARRAY_BUFFER, sizeof line, line, GL_STATIC_DRAW);

	// Enable point size control in vertex shader
#ifndef GL_ES_VERSION_2_0
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#endif

	//return 1;

	timestamp_t t1 = get_timestamp();
	double secs = (t1 - t0) / 1000000.0L;
	std::cout<<"iinit init_resources total time: "<<secs<<std::endl;
	return 1;
}
int checkEnd()
{

return -1;	
}

void display() {
	glUseProgram(program);
	glUniform1i(uniform_mytexture, 0);

	glUniform1f(uniform_offset_x, offset_x);
	glUniform1f(uniform_scale_x, scale_x);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Set texture wrapping mode */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);

	/* Set texture interpolation mode */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interpolate ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interpolate ? GL_LINEAR : GL_NEAREST);

	/* Draw using the vertices in our vertex buffer object */
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glEnableVertexAttribArray(attribute_coord1d);
	glVertexAttribPointer(attribute_coord1d, 1, GL_FLOAT, GL_FALSE, 0, 0);

	/* Draw the line */
	glDrawArrays(GL_LINE_STRIP, 0, 101);

	/* Draw points as well, if requested */
	if (showpoints)
		glDrawArrays(GL_POINTS, 0, 101);

	glFlush();
	glutSwapBuffers();

<<<<<<< HEAD
	if(dataEnd != true){
		getData();
		//display();
=======
	if(checkEnd() > 0)
		exit(0);

	if(dataEnd != true){
		getData();
	//	display();
>>>>>>> develop
		glFlush();
	glutSwapBuffers();
	}
	else return;

	
}

void special(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_F1:
		interpolate = !interpolate;
		printf("Interpolation is now %s\n", interpolate ? "on" : "off");
		break;
	case GLUT_KEY_F2:
		clamp = !clamp;
		printf("Clamping is now %s\n", clamp ? "on" : "off");
		break;
	case GLUT_KEY_F3:
		showpoints = !showpoints;
		printf("Showing points is now %s\n", showpoints ? "on" : "off");
		break;
	case GLUT_KEY_LEFT:
		offset_x -= 0.1;
		break;
	case GLUT_KEY_RIGHT:
		offset_x += 0.1;
		break;
	case GLUT_KEY_UP:
		scale_x *= 1.5;
		break;
	case GLUT_KEY_DOWN:
		scale_x /= 1.5;
		break;
	case GLUT_KEY_HOME:
		offset_x = 0.0;
		scale_x = 1.0;
		break;
	}

	glutPostRedisplay();
}

void free_resources() {
	glDeleteProgram(program);
}

int main(int argc, char *argv[]) 
{
	if (argc < 2)
    {
        std::cout << "Usage: wave_iteration <FILENAME>" << std::endl;
        return 1;
    }
    strcpy(fileName, argv[1]);

    //SFML Plays Audio

    sf::Music music;
    if (!music.openFromFile(argv[1]))
       return -1; // error
  //  music.play();


	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB);
	glutInitWindowSize(640, 480);
	glutCreateWindow("My Graph");

	GLenum glew_status = glewInit();

	if (GLEW_OK != glew_status) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
		return 1;
	}

	if (!GLEW_VERSION_2_0) {
		fprintf(stderr, "No support for OpenGL 2.0 found\n");
		return 1;
	}

	GLint max_units;

	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &max_units);
	if (max_units < 1) {
		fprintf(stderr, "Your GPU does not have any vertex texture image units\n");
		return 1;
	}

	GLfloat range[2];

	glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, range);
	if (range[1] < 5.0)
		fprintf(stderr, "WARNING: point sprite range (%f, %f) too small\n", range[0], range[1]);

	printf("Use left/right to move horizontally.\n");
	printf("Use up/down to change the horizontal scale.\n");
	printf("Press home to reset the position and scale.\n");
	printf("Press F1 to toggle interpolation.\n");
	printf("Press F2 to toggle clamping.\n");
	printf("Press F3 to toggle drawing points.\n");
	getData();
	if (init_resources()) {
		glutDisplayFunc(display);
		glutSpecialFunc(special);
<<<<<<< HEAD
=======
		glutIdleFunc(moveWav);
>>>>>>> develop
		glutMainLoop();
	}

	free_resources();
	return 0;
}
