
GLuint program;
GLint attribute_coord1d;
GLint uniform_offset_x;
GLint uniform_scale_x;
GLuint texture_id;
GLint uniform_mytexture;
GLuint vbo;

float offset_x = 0.0;
//float scale_x = 1.0/(1.5*10)/(1.5*7);
float scale_x =1.0;
bool interpolate = false;
bool clamp = false;
bool showpoints = true;


int graph[N/2]; 
int framePointer = 0;
char fileName[50];
bool calledFromInit = true;
bool dataEnd = false;
bool playFlag = true;
bool muteFlag = false;

sf::Time totalMusicDuration;
sf::Time timePlay;

sf::Music music;
kiss_fft_cpx in[N],out[N];
timestamp_t tmain;

void getData();
void display();

typedef unsigned long long timestamp_t;
  static timestamp_t
    get_timestamp ()
    {
      struct timeval now;
      gettimeofday (&now, NULL);
      return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
    }