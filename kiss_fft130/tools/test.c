#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "kiss_fft.h"

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

int main(void)
{
  kiss_fft_cpx in[N], out[N];
  size_t i;

  for (i = 0; i < N; i++)
    in[i].r = in[i].i = 0;
  TestFft("Zeroes (complex)", in, out);

  for (i = 0; i < N; i++)
    in[i].r = 1, in[i].i = 0;
  TestFft("Ones (complex)", in, out);

  for (i = 0; i < N; i++){
    in[i].r = sin(2 * M_PI * 4 * i / N), in[i].i = 0;
    
  }
  TestFft("SineWave (complex)", in, out);

  return 0;
}