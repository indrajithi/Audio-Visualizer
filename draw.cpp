//g++ -std=c++11 -c draw.cpp
//g++ -std=gnu++11 finalDraw.o ../kiss_fft130/kiss_fft.c  -L /home/<username>/mylib/lib/ -lAquila -lOoura_fft -lm -lglut -lGLEW -lGL -lsfml-audio ../common/shader_utils.o -o finalDraw
#include "visualizer.hpp"
#include <unistd.h>

//#define N 2048 

#define N 10000

#define BANDS 64         // log-spaced frequency bands, one bar each
#define MIN_FREQ 50.0    // lower edge of the first band, in Hz; lower bands would be
                         // narrower than one FFT bin and repeat their neighbors
#define MAX_FREQ 16000.0 // upper edge of the last band; compressed audio is
                         // usually empty above this, which would leave dead bars
#define DB_FLOOR -80.0   // level drawn at the bottom of the bars, in dB

#define BAR_RELEASE 9.0      // how fast bars ease back down (per second)
#define PEAK_HOLD 0.35       // seconds a peak marker waits before falling
#define PEAK_FALL 0.45       // peak marker fall speed, in bar heights per second
#define BASS_MAX_FREQ 150.0  // bands below this drive the background pulse, in Hz

// Progress bar layout in pixels; must match the constants in graph.f.glsl
#define TRACK_Y 28
#define TRACK_MARGIN 70

typedef unsigned long long timestamp_t;
  static timestamp_t
    get_timestamp ()
    {
      struct timeval now;
      gettimeofday (&now, NULL);
      return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
    }

GLuint program;
GLint attribute_coord2d;
GLint uniform_offset_x;
GLint uniform_scale_x;
GLuint texture_id;
GLint uniform_mytexture;
GLint uniform_resolution;
GLint uniform_bands;
GLint uniform_progress;
GLint uniform_bass;
GLint uniform_smooth_mode;
GLint uniform_show_peaks;

float offset_x = 0.0;
//float scale_x = 1.0/(1.5*10)/(1.5*7);
float scale_x =1.0;
bool interpolate = false;
bool clamp = false;
bool showPeaks = true;

GLuint vbo;
// Two bytes per band (bar level, peak level), matching the GL_LUMINANCE_ALPHA upload
unsigned char graph[BANDS * 2];
float barLevel[BANDS];    // displayed bar height, 0..1
float peakLevel[BANDS];   // falling peak marker height, 0..1
float peakHold[BANDS];    // seconds left before each peak marker starts falling
float bassLevel = 0;      // 0..1 low-frequency energy for the background pulse
timestamp_t lastFrame = 0;
int framePointer = 0;
std::string fileName;
std::unique_ptr<Aquila::WaveFile> wav;
bool playFlag = true;
bool muteFlag = false;
bool soundStatFirstCall = true;
sf::Time totalMusicDuration;
sf::Time timePlay;



sf::Music music;

kiss_fft_cpx in[N],out[N];
timestamp_t tmain;



void getData();
void uploadGraph();
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
	// Exit once playback finishes; a paused song keeps its last spectrum on screen
	if (music.getStatus() == sf::Music::Stopped)
		exit(0);

	getData();
	uploadGraph();
	display();

	// ~60 fps is enough for the spectrum and keeps the idle loop from spinning a core
	usleep(16000);
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
	int i, j;
	int sampleCount = wav->getSamplesCount();
	double sampleRate = wav->getSampleFrequency();

	// Center the analysis window on the sample being played right now, so the
	// spectrum follows playback, pause and seeking instead of free-running.
	framePointer = music.getPlayingOffset().asSeconds() * sampleRate - N / 2;
	if (framePointer > sampleCount - N)
		framePointer = sampleCount - N;
	if (framePointer < 0)
		framePointer = 0;

	for (i = framePointer, j = 0; j < N; i++, j++) {
		//Apply Hann window on the sample
		double multiplier = 0.5 * (1 - cos(2*M_PI*j/(N-1)));
		in[j].r = multiplier * wav->sample(i);
		in[j].i = 0;
	}

	getFft(in,out);

	// Animation runs on wall-clock time so its speed doesn't depend on frame rate
	timestamp_t now = get_timestamp();
	double dt = lastFrame ? (now - lastFrame) / 1000000.0 : 0;
	if (dt > 0.1)
		dt = 0.1;
	lastFrame = now;
	double release = 1 - exp(-dt * BAR_RELEASE);

	// A full-scale 16-bit sine under a Hann window peaks at 32768 * N / 4,
	// so normalizing by it puts the loudest possible bin at 0 dB.
	double fullScale = 32768.0 * N / 4;
	double binHz = sampleRate / N;
	double maxFreq = fmin(MAX_FREQ, sampleRate / 2);
	double bassSum = 0;
	int bassBands = 0;

	for (int band = 0; band < BANDS; band++) {
		// Log-spaced band edges give every octave the same width on screen
		double lo = MIN_FREQ * pow(maxFreq / MIN_FREQ, (double)band / BANDS);
		double hi = MIN_FREQ * pow(maxFreq / MIN_FREQ, (double)(band + 1) / BANDS);
		// Half-open bin ranges so neighboring bands never share a bin
		int first = round(lo / binHz);
		int last = round(hi / binHz) - 1;
		if (last < first)
			last = first;
		if (last >= N/2)
			last = N/2 - 1;

		// Peak rather than average, so a pure tone keeps its full height
		double peak = 0;
		for (i = first; i <= last; i++) {
			double mag = sqrt((out[i].r * out[i].r) + (out[i].i * out[i].i));
			if (mag > peak)
				peak = mag;
		}

		// Map DB_FLOOR..0 dB onto 0..1, clamped so quiet bins sit at the bottom
		double db = 20 * log10(peak / fullScale + 1e-12);
		double level = (db - DB_FLOOR) / -DB_FLOOR;
		if (level < 0)
			level = 0;
		if (level > 1)
			level = 1;

		// Bars jump up instantly and ease back down, so they move without flicker
		if (level > barLevel[band])
			barLevel[band] = level;
		else
			barLevel[band] += (level - barLevel[band]) * release;

		// Peak markers hold for a moment, then drop at a steady speed
		if (barLevel[band] >= peakLevel[band]) {
			peakLevel[band] = barLevel[band];
			peakHold[band] = PEAK_HOLD;
		}
		else if (peakHold[band] > 0)
			peakHold[band] -= dt;
		else
			peakLevel[band] = fmax(barLevel[band], peakLevel[band] - PEAK_FALL * dt);

		if (hi <= BASS_MAX_FREQ) {
			bassSum += barLevel[band];
			bassBands++;
		}

		graph[band * 2] = barLevel[band] * 255;
		graph[band * 2 + 1] = peakLevel[band] * 255;
	}

	// Bass sits high on most music, so stretch its upper range to make the pulse visible
	double bass = bassBands ? bassSum / bassBands : 0;
	bassLevel = fmin(fmax((bass - 0.45) / 0.45, 0.0), 1.0);
}

void uploadGraph()
{
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, BANDS, 1, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, graph);
}

int init_resources() {

	timestamp_t t0 = get_timestamp();
	program = create_program("graph.v.glsl", "graph.f.glsl");
	if (program == 0)
		return 0;

	attribute_coord2d = get_attrib(program, "coord2d");
	uniform_offset_x = get_uniform(program, "offset_x");
	uniform_scale_x = get_uniform(program, "scale_x");
	uniform_mytexture = get_uniform(program, "mytexture");
	uniform_resolution = get_uniform(program, "resolution");
	uniform_bands = get_uniform(program, "bands");
	uniform_progress = get_uniform(program, "progress");
	uniform_bass = get_uniform(program, "bass");
	uniform_smooth_mode = get_uniform(program, "smooth_mode");
	uniform_show_peaks = get_uniform(program, "show_peaks");

	if (attribute_coord2d == -1 || uniform_offset_x == -1 || uniform_scale_x == -1 || uniform_mytexture == -1
			|| uniform_resolution == -1 || uniform_bands == -1 || uniform_progress == -1
			|| uniform_bass == -1 || uniform_smooth_mode == -1 || uniform_show_peaks == -1)
		return 0;
 

	//gets the first spectrum in to graph
	getData();
	/* Upload the texture with our datapoints */
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, BANDS, 1, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, graph);

	// Create the vertex buffer object
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// A single quad covering the window; the fragment shader draws everything on it
	GLfloat quad[] = { -1, -1,   1, -1,   -1, 1,   1, 1 };

	// Tell OpenGL to copy our array to the buffer object
	glBufferData(GL_ARRAY_BUFFER, sizeof quad, quad, GL_STATIC_DRAW);

	timestamp_t t1 = get_timestamp();
	double secs = (t1 - t0) / 1000000.0L;
	std::cout<<"iinit init_resources total time: "<<secs<<std::endl;
	return 1;
}
int checkEnd()
{

return -1;	
}

// Formats a duration as m:ss
std::string formatTime(float seconds)
{
	int total = seconds;
	char buf[16];
	snprintf(buf, sizeof buf, "%d:%02d", total / 60, total % 60);
	return buf;
}

int textWidth(void *font, const std::string &text)
{
	int width = 0;
	for (char c : text)
		width += glutBitmapWidth(font, c);
	return width;
}

void drawText(void *font, int x, int y, const std::string &text, float r, float g, float b)
{
	// The raster color is latched by glWindowPos, so it must be set first
	glColor3f(r, g, b);
	glWindowPos2i(x, y);
	for (char c : text)
		glutBitmapCharacter(font, c);
}

void drawOverlay(int width, int height)
{
	glUseProgram(0);

	void *small = GLUT_BITMAP_HELVETICA_12;
	std::string elapsed = formatTime(music.getPlayingOffset().asSeconds());
	std::string total = formatTime(totalMusicDuration.asSeconds());
	drawText(small, TRACK_MARGIN - 14 - textWidth(small, elapsed), TRACK_Y - 4, elapsed, 0.80, 0.80, 0.90);
	drawText(small, width - TRACK_MARGIN + 14, TRACK_Y - 4, total, 0.55, 0.55, 0.65);

	// Song name along the top, with a pause badge and the key hints
	std::string title = fileName.substr(fileName.find_last_of('/') + 1);
	drawText(GLUT_BITMAP_HELVETICA_18, 22, height - 32, title, 0.95, 0.93, 1.0);
	if (!playFlag)
		drawText(small, 34 + textWidth(GLUT_BITMAP_HELVETICA_18, title), height - 31, "PAUSED", 1.0, 0.72, 0.30);

	std::string hints = "p pause    r restart    click bar to seek    q quit";
	drawText(small, width - 22 - textWidth(small, hints), height - 31, hints, 0.45, 0.45, 0.55);
}

void display() {
	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);
	glViewport(0, 0, width, height);

	float duration = totalMusicDuration.asSeconds();
	float progress = duration > 0 ? music.getPlayingOffset().asSeconds() / duration : 0;

	glUseProgram(program);
	glUniform1i(uniform_mytexture, 0);

	glUniform1f(uniform_offset_x, offset_x);
	glUniform1f(uniform_scale_x, scale_x);
	glUniform2f(uniform_resolution, width, height);
	glUniform1f(uniform_bands, BANDS);
	glUniform1f(uniform_progress, progress);
	glUniform1f(uniform_bass, bassLevel);
	glUniform1f(uniform_smooth_mode, interpolate ? 1.0 : 0.0);
	glUniform1f(uniform_show_peaks, showPeaks ? 1.0 : 0.0);

	/* Set texture wrapping mode */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);

	/* Set texture interpolation mode */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interpolate ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interpolate ? GL_LINEAR : GL_NEAREST);

	/* Draw the full-window quad from our vertex buffer object */
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(attribute_coord2d);
	glVertexAttribPointer(attribute_coord2d, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableVertexAttribArray(attribute_coord2d);

	drawOverlay(width, height);

	glFlush();
	glutSwapBuffers();
}

// Seeks to the song position under window x when it lands on the progress bar
void seekToX(int x)
{
	int width = glutGet(GLUT_WINDOW_WIDTH);
	float fraction = (float)(x - TRACK_MARGIN) / (width - 2 * TRACK_MARGIN);
	// Stop just short of the end: seeking onto it would stop playback and exit
	fraction = fmin(fmax(fraction, 0.0f), 0.999f);
	music.setPlayingOffset(sf::seconds(fraction * totalMusicDuration.asSeconds()));
}

bool draggingProgress = false;

void mouse(int button, int state, int x, int y)
{
	if (button != GLUT_LEFT_BUTTON)
		return;
	if (state == GLUT_UP) {
		draggingProgress = false;
		return;
	}

	// GLUT reports y from the top; the track is laid out from the bottom
	int width = glutGet(GLUT_WINDOW_WIDTH);
	int fromBottom = glutGet(GLUT_WINDOW_HEIGHT) - y;
	if (abs(fromBottom - TRACK_Y) <= 12 && x >= TRACK_MARGIN - 10 && x <= width - TRACK_MARGIN + 10) {
		draggingProgress = true;
		seekToX(x);
	}
}

void motion(int x, int y)
{
	if (draggingProgress)
		seekToX(x);
}

void special(int key, int x, int y) {
	float t;
	switch (key) {
	case GLUT_KEY_F7:
		interpolate = !interpolate;
		printf("Smooth curve is now %s\n", interpolate ? "on" : "off");
		break;
	case GLUT_KEY_F8:
		clamp = !clamp;
		printf("Clamping is now %s\n", clamp ? "on" : "off");
		break;
	case GLUT_KEY_F9:
		showPeaks = !showPeaks;
		printf("Peak markers are now %s\n", showPeaks ? "on" : "off");
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
    fileName = argv[1];
	tmain = get_timestamp();
   //sfm play music
 	if (!music.openFromFile(fileName))
       		return -1;

	totalMusicDuration = music.getDuration ();

	// Decode the samples once up front; re-reading the file every frame
	// made each frame cost a full load of the WAV
	wav.reset(new Aquila::WaveFile(fileName));
	double expectedSamples = totalMusicDuration.asSeconds() * wav->getSampleFrequency();
	if (wav->getSamplesCount() < N || wav->getSamplesCount() < 0.9 * expectedSamples) {
		// The sample reader expects the audio data right after a plain 44-byte
		// header; files with extra chunks (e.g. metadata) before it are misread
		fprintf(stderr, "Error: read only %u samples from %s; re-save it as plain 16-bit PCM WAV "
				"(e.g. ffmpeg -i in.wav -map_metadata -1 -fflags +bitexact out.wav)\n",
				(unsigned)wav->getSamplesCount(), fileName.c_str());
		return 1;
	}

	glutInit(&argc, argv);
	// Double buffered so each frame appears whole, without tearing or flicker
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(1000, 560);
	glutCreateWindow("Audio Spectrum Visualizer");

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
	printf("Press F7 to toggle bars / smooth curve.\n");
	printf("Press F8 to toggle clamping.\n");
	printf("Press F9 to toggle peak markers.\n");
	printf("Click or drag the progress bar to seek.\n");
	printf("Press q to exit.\n");
	printf("Press p to toggle Play/Pause audio.\n");
	printf("Press r to reload and play audio.\n");

	music.play();

	if (init_resources()) {

		glutDisplayFunc(display);
		glutSpecialFunc(special);
		glutIdleFunc(moveWav);
		glutKeyboardFunc(key);
		glutMouseFunc(mouse);
		glutMotionFunc(motion);
		glutMainLoop();
	}

	free_resources();
	return 0;
}
