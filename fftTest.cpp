//g++ -std=gnu++11 fftTest.cpp ./kiss_fft130/kiss_fft.c -L /home/l1f3/mylib/lib/ -lAquila -lOoura_fft -o fftTest
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <chrono>
#include <sys/time.h>

#include "aquila/source/WaveFile.h"
#include "kiss_fft130/kiss_fft.h"
#ifndef M_PI
#define M_PI 3.14159265358979324
#endif

#define N 2048

kiss_fft_cpx in[N],out[N];

typedef unsigned long long timestamp_t;
  static timestamp_t
    get_timestamp ()
    {
      struct timeval now;
      gettimeofday (&now, NULL);
      return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
    }

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

void print_vec(const std::vector<int>& vec)
{
    for (auto x: vec) {
         std::cout << ' ' << x;
    }
    std::cout << '\n';
}

int main(int argc, char *argv[]) 
{
 
  if (argc < 2)
    {
        std::cout << "Usage: wave_iteration <FILENAME>" << std::endl;
        return 1;
    }

  Aquila::WaveFile wav(argv[1]);
  std::vector<int> array(0); 
  std::vector<int>::iterator it;

  it = array.begin();
  timestamp_t t0 = get_timestamp();

  int i,j,x;
  int graph[N/2];
  double mag[N/2];
  double roof = wav.getSamplesCount();
  double framePointer = 0;

  while(framePointer < roof  ){

    for( i = framePointer, j = 0; i < (framePointer + N)
                     && framePointer < roof - N ; i++,j++  )
    {

      //Apply window function on the sample
      double multiplier = 0.5 * (1 - cos(2*M_PI*j/(N-1)));
      in[j].r = multiplier * wav.sample(i);
      in[j].i = 0;  //stores N samples 
    }
    
  if(framePointer < roof-N ){
    framePointer = i;

  }
  else {
    
    timestamp_t t1 = get_timestamp();
    double secs = (t1 - t0) / 1000000.0L;

   // print_vec(array);
    std::cout<<"Total exec time: "<<secs<<std::endl;
    break;
   
  }

 //std::cout<<"Framepointer = "<<framePointer<<std::endl;

  getFft(in,out);

  // calculate magnitude of first n/2 FFT
  for(i = 0; i < N/2; i++ ){
    int val;
    mag[i] = sqrt((out[i].r * out[i].r) + (out[i].i * out[i].i));
  
  
  // N/2 Log magnitude values.
  //for (i = 0; i < N/2 ; ++i){
  //  x =   10 * log10(mag[i]) ;
  //  printf("  log x= %g ", log(x));
   val = graph[i] = log(mag[i]) *10; 
  //  std::cout<<graph[i]<<std::endl;
    it = array.end();
    it = array.insert(it,val);
    
  }

}
//std::cout<<array[3]<<std::endl;;
//print_vec(array);

  std::vector<int>::size_type sz = array.size();

//for(size_t i = 0; i< sz; i++)
 // std::cout<<array[i]<<std::endl;

}
