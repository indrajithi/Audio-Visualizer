	//g++ -std=c++11 -c draw.cpp
//g++ -std=gnu++11 finalDraw.o ../kiss_fft130/kiss_fft.c  -L /home/l1f3/mylib/lib/ -lAquila -lOoura_fft -lm -lglut -lGLEW -lGL -lsfml-audio ../common/shader_utils.o -o finalDraw
#include "visualizer.hpp"
#include <unistd.h>

//#define N 2048 //14700
//#define N 32768
#define N 10000
//#define N 16384
//int  N;
//TIME * FREQ(44) = N
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
//float scale_x = 1.0/(1.5*10)/(1.5*7);
float scale_x =1.0;
bool interpolate = false;
bool clamp = false;
bool showpoints = true;

GLuint vbo;
int graph[N/2]; 
int framePointer = 0;
char fileName[50];
bool calledFromInit = true;
bool dataEnd = false;
bool playFlag = true;
bool muteFlag = false;
bool soundStatFirstCall = true;
sf::Time totalMusicDuration;
sf::Time timePlay;



sf::Music music;

kiss_fft_cpx in[N],out[N];
timestamp_t tmain;



void getData();
void display();

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


void moveWav()
{
	//std::this_thread::sleep_for(std::chrono::milliseconds(100));
	//offset_x -= 0.07;	
	//if(offset_x < -1.0)
	//	offset_x = 0.0;

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
int plotPtr = 0;
int pltGraph[100];



void getData()
{
	timestamp_t t0 = get_timestamp();
	int i,j,x;
	
	Aquila::WaveFile wav(fileName);
	double mag[N/2];
	double roof = wav.getSamplesCount();
	
	
	//check if audio is still playing.
	
/*	sf::SoundSource::Status soundStat = music.getStatus();

	if(soundStatFirstCall)
		soundStatFirstCall != soundStatFirstCall;
	
	std::cout<<"soundStat "<<soundStat<<std::endl;


	if(!soundStatFirstCall)
	switch (soundStat)
	{
		case 0: std::cout<<"N = "<<N<<std::endl;
				std::cout<<"Music stopped" <<std::endl;
				std::cout<<"total Music Duration = "<<totalMusicDuration.asMilliseconds()<<std::endl;
			    std::cout<<"Frame Pointer = "<<framePointer<<std::endl;
			    std::cout<<"Frames Left = "<<roof - framePointer<<std::endl;
				std::cout<<"SFML Sample Rate = "<<music.getSampleRate()<<std::endl; 
				exit(0);
	}*/



	//Get first N samples
	for( i = framePointer, j = 0; i < (framePointer + N)
										 && framePointer < roof - N ; i++,j++  ){

		//Apply window function on the sample
		double multiplier = 0.5 * (1 - cos(2*M_PI*j/(N-1)));
		in[j].r = multiplier * wav.sample(i);
		in[j].i = 0;  //stores N samples 
	}
	
		
	if(framePointer < roof-N -1){
		framePointer = i;

	}
	else {
		
		timestamp_t t1 = get_timestamp();
		double secs = (t1 - tmain) / 1000000.0L;

		sf::Time musicPlayingOffset = music.getPlayingOffset();
		
		unsigned int musicSampleRate = music.getSampleRate();

		int musicLeftToPlay = totalMusicDuration.asMilliseconds() - musicPlayingOffset.asMilliseconds();

		std::cout<<"N = "<<N<<std::endl;
		std::cout<<"Frame pointer > roof - N"<<std::endl;
		std::cout<<"Framepointer = "<<framePointer<<std::endl;
		std::cout<<"Frames Left = "<<roof - framePointer<<std::endl;
		std::cout<<"Total exec time: "<<secs<<std::endl;
		std::cout<<"Total Music Played Duration = "<<musicPlayingOffset.asMilliseconds()<<std::endl;
		std::cout<<"Music left to play = "<<musicLeftToPlay<<std::endl;
		std::cout<<"SFML Sample Rate = "<<musicSampleRate<<std::endl;  
		
		exit(0);
	}

	if(framePointer >= roof) {
		dataEnd = true; 
		return ;
	}

	std::cout<<"Framepointer = "<<framePointer<<std::endl;

	getFft(in,out);

	// calculate magnitude of first n/2 FFT
	for(i = 0; i < N/2; i++ ){
		mag[i] = sqrt((out[i].r * out[i].r) + (out[i].i * out[i].i));
	
	
	// N/2 Log magnitude values.
	//for (i = 0; i < N/2 ; ++i){
	//	x =   10 * log10(mag[i]) ;
	//	printf("  log x= %g ", log(x));
		graph[i] = log(mag[i]) *10;	
	}
	
	if(!calledFromInit)
	{
	
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
	} 

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


	 

	//gets N/2 values in to graph
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

	//sleep(1);
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

	//glFlush();
	//glutSwapBuffers();

	if(checkEnd() > 0){
		
		exit(0);
	}

	if(dataEnd != true){
		getData();
	//	display();
		glFlush();
	glutSwapBuffers();
	}
	else return;

	
}

void special(int key, int x, int y) {
	float t;
	switch (key) {
	case GLUT_KEY_F7:
		interpolate = !interpolate;
		printf("Interpolation is now %s\n", interpolate ? "on" : "off");
		break;
	case GLUT_KEY_F8:
		clamp = !clamp;
		printf("Clamping is now %s\n", clamp ? "on" : "off");
		break;
	case GLUT_KEY_F9:
		showpoints = !showpoints;
		printf("Showing points is now %s\n", showpoints ? "on" : "off");
		break;
	case GLUT_KEY_LEFT:
		offset_x -= 0.1;
		timePlay = music.getPlayingOffset();
		t = timePlay.asSeconds(); 
		music.setPlayingOffset(sf::seconds(t - 5));
		break;
	case GLUT_KEY_RIGHT:
		offset_x += 0.1;
		timePlay = music.getPlayingOffset();
		t = timePlay.asSeconds(); 
		music.setPlayingOffset(sf::seconds(t + 5));
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
	case GLUT_KEY_F10:
		exit(0);


	}

	glutPostRedisplay();
}

void key(unsigned char k,int,int)
{
	if(k == 'p'){
		
		if(playFlag){
			music.pause();
			playFlag = !playFlag;
		}
		else
		{
			music.play();
			playFlag = !playFlag;
		}
	}
	
	if(k == 'm'){
		if(!muteFlag){
			music.setVolume(0);
			muteFlag=!muteFlag;
		}
		else{
			music.setVolume(100);
			muteFlag=!muteFlag;
		}
	}
	if(k == 'r')//reload audio
	{
	
		music.setPlayingOffset(sf::seconds(0));
	}


	if(k == 'q')
		exit(0);

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
	tmain = get_timestamp();
   //sfm play music
 	if (!music.openFromFile(fileName))
       		return -1; 

// init N
   // Aquila::WaveFile wav(argv[1]);
    //N = (int)((wav.getAudioLength()/100) * (wav.getSampleFrequency()/100));

	totalMusicDuration = music.getDuration ();

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

	printf("------------------------------------------------------\n");	
	printf("AUDIO SPECTRUM VISUALIZER\nSubmitted in partial fulfilment of the ");
	printf("requirements for the Computer Graphics\nLaboratory(10CSL67) course of the 6th semester.");
	printf("\nBachelor of Engineering In Computer science & Engineering\nSubmitted by: INDRAJITH I (4AI12CS042)\n");
	printf("------------------------------------------------------\n\n");
	printf("Use left/right to move horizontally.And seek audio by +/-5 sec\n");
	printf("Use up/down to change the horizontal scale.\n");
	printf("Press home to reset the position and scale.\n");
	printf("Press F7 to toggle interpolation.\n");
	printf("Press F8 to toggle clamping.\n");
	printf("Press F9 to toggle drawing points.\n");
	printf("Press q to exit.\n");
	printf("Press p to toggle Play/Pause audio.\n");
	printf("Press r to reload and play audio.\n");

	
	getData();

	music.play();

	if (init_resources()) {

		glutDisplayFunc(display);
		glutSpecialFunc(special);
		glutIdleFunc(moveWav);
		glutKeyboardFunc(key);
		glutMainLoop();
	}

	free_resources();
	return 0;
}
