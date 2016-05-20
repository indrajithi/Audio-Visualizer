#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "kiss_fft.h"

#include "aquila/global.h"
#include "aquila/source/WaveFile.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979324
#endif

#define N 16

void TestFft(const char* title, const kiss_fft_cpx in[N], kiss_fft_cpx out[N])
{
  kiss_fft_cfg cfg;

  printf("%s\n", title);

  if ((cfg = kiss_fft_alloc(N, 0/*is_inverse_fft*/, NULL, NULL)) != NULL)
  {
    size_t i;

    kiss_fft(cfg, in, out);
    free(cfg);

    for (i = 0; i < N; i++)
      printf(" in[%2zu] = %+f , %+f    "
             "out[%2zu] = %+f , %+f\n",
             i, in[i].r, in[i].i,
             i, out[i].r, out[i].i);
  }
  else
  {
    printf("not enough memory?\n");
    exit(-1);
  }
}

int main(int argc, char*argv[])
{
  kiss_fft_cpx in[N], out[N];
  size_t i;
  int tmp[16] = {31,57,50,34,24,14,17,26,29,39,54,48,27,5,-15,-31};

  for (i = 0; i < N; i++)
    in[i].r = in[i].i = 0;
  TestFft("Zeroes (complex)", in, out);

  for (i = 0; i < N; i++)
    in[i].r = tmp[i], in[i].i = 0;
  TestFft("Ones (complex)", in, out);

  for (i = 0; i < N; i++){
    in[i].r = sin(2 * M_PI * 4 * i / N), in[i].i = 0;
    
  }
  //TestFft("SineWave (complex)", in, out);

  return 0;
}