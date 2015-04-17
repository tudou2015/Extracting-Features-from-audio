/*
 * Copyright (C) 2012 Jamie Bullock
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

#include "xtract/libxtract.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <sndfile.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

/*typedef enum waveform_type_
{
    SINE,
    SAWTOOTH,
    SQUARE,
    NOISE
} 
waveform_type;
*/
#define BLOCKSIZE 1024
#define HALF_BLOCKSIZE BLOCKSIZE >> 1
#define SAMPLERATE 44100
#define PERIOD 102
#define MFCC_FREQ_BANDS 13
#define MFCC_FREQ_MIN 20
#define MFCC_FREQ_MAX 20000


/*double wavetable[BLOCKSIZE];

void fill_wavetable(const float frequency, waveform_type type)
{

    int samples_per_period = SAMPLERATE / frequency;

    for (int i = 0; i < BLOCKSIZE; ++i)
    {
        int phase = i % samples_per_period;

        switch (type)
        {
            case SINE:
                wavetable[i] = sin((phase / (double)PERIOD) * 2 * M_PI);
                break;
            case SQUARE:
                if (phase < (samples_per_period / 2.f))     
                {
                    wavetable[i] = -1.0;
                }
                else
                {
                    wavetable[i] = 1.0;
                }
                break;
            case SAWTOOTH:
                wavetable[i] = ((phase / (double)PERIOD) * 2) - 1.;
                break;
            case NOISE:
                wavetable[i] = ((random() % 1000) / 500.0) - 1;
                break; 
        }
    }
}

void print_wavetable(void)
{
    for (int i = 0; i < BLOCKSIZE; ++i)
    {
        printf("%f\n", wavetable[i]);
    }
}
*/

long int powtwo(double n){
	long int ans=1;
	while(ans<n){
		ans *= 2;
	}
	return ans;
}

int main(int argc, char *argv[])
{

   printf("Wav Read Test\n");
   //if (argc != 2) {
     // fprintf(stderr, "Expecting wav file as argument\n");
     // return 1;
  // }
   // Open sound file
   SF_INFO sndInfo;
   
   SNDFILE *sndFile = sf_open(argv[1], SFM_READ, &sndInfo);

   if (sndFile == NULL) {
      fprintf(stderr, "Error reading source file '%s': %s\n", argv[1], sf_strerror(sndFile));
      return 1;
   }
   // Check format - 16bit PCM
   if (sndInfo.format != (SF_FORMAT_WAV | SF_FORMAT_PCM_16)) {
      fprintf(stderr, "Input should be 16bit Wav\n");
      sf_close(sndFile);
      return 1;
   }

   long int powerTwo = powtwo(sndInfo.frames);
   printf("\nHello there %d\n",powerTwo);
   // Allocate memory
   double *wavetable = malloc(sndInfo.channels * powerTwo * sizeof(double));   


   if (wavetable == NULL) {
      fprintf(stderr, "Could not allocate memory for data\n");
      sf_close(sndFile);
      return 1;
   }

   // Load data
   long numFrames_initial = sf_readf_double(sndFile, wavetable, sndInfo.channels*sndInfo.frames);
   long numFrames = powtwo(numFrames_initial);
   printf("\nHello there %d\n",numFrames);
   int i;
   for(i=0;i<sndInfo.frames;i+=10000){
	printf("Sample %d :\t%f \n",i,wavetable[i]);
   }
   for(i=sndInfo.frames;i<numFrames;i++){
	wavetable[i]=0;
   }

   /*// Check correct number of samples loaded
   if (numFrames != sndInfo.frames) {
      fprintf(stderr, "Did not read enough frames for source\n");
      sf_close(sndFile);
      free(wavetable);
      return 1;
   }*/
   // Output Info
   printf("Read %ld frames from %s, Sample rate: %d, Length: %fs\n",
      numFrames, argv[1], sndInfo.samplerate, (float)numFrames/sndInfo.samplerate);










    double mean = 0.0; 
    double f0 = 0.0;
    double midicents = 0.0;
    double flux = 0.0;
    double centroid = 0.0;
    double lowest = 0.0;
    double spectrum[numFrames];// = {0};
    double windowed[numFrames];// = {0};
    double peaks[numFrames];// = {0};
//    printf("\nHello1\n");
    double harmonics[numFrames];// = {0};
    double subframes[numFrames];// = {0};
    double difference[numFrames/2];// = {0};
    double *window = NULL;
    double mfccs[MFCC_FREQ_BANDS] = {0};
    double argd[4] = {0};
    double samplerate = sndInfo.samplerate;
    int n;
    int rv = XTRACT_SUCCESS;
    xtract_mel_filter mel_filters;



   sf_close(sndFile);

    // fill_wavetable(344.53125f, NOISE); // 344.53125f = 128 samples @ 44100 Hz
//    fill_wavetable(344.53125f, SINE); // 344.53125f = 128 samples @ 44100 Hz

    /* 
    print_wavetable();
    */

    /* get the F0 */
    xtract[XTRACT_WAVELET_F0](wavetable, numFrames, &samplerate, &f0);
    printf("\nF0: %f\n", f0);

    //* get the F0 as a MIDI note 
    xtract[XTRACT_MIDICENT](NULL, 0, &f0, &midicents);
    printf("\nMIDI cents: %f\n", midicents);

    //* get the mean of the input 
    xtract[XTRACT_MEAN](wavetable, numFrames, NULL, &mean);
    printf("\nInput mean = %.2f\n\n", mean); //* We expect this to be zero for a square wave 

    //* get the lowest value in the input 
    argd[0] = -.5;
    rv = xtract[XTRACT_LOWEST_VALUE](wavetable, numFrames, argd, &lowest);
    
    if (rv == XTRACT_SUCCESS)
    {
        printf("\nLowest value = %.6f\n\n", lowest);
    }
    else
    {
        printf("\nUnable to get lowest value, all values below threshold?\n\n");
    }

    //* create the window function 
    window = xtract_init_window(numFrames, XTRACT_HANN);
    xtract_windowed(wavetable, numFrames, window, windowed);
    xtract_free_window(window);

    //* get the spectrum 
    argd[0] = SAMPLERATE / (double)numFrames;
    argd[1] = XTRACT_MAGNITUDE_SPECTRUM;
    argd[2] = 0.f; //* DC component - we expect this to zero for square wave 
    argd[3] = 0.f; //* No Normalisation 

    xtract_init_fft(numFrames, XTRACT_SPECTRUM);
    xtract[XTRACT_SPECTRUM](windowed, numFrames, &argd[0], spectrum);
    xtract_free_fft();

    xtract[XTRACT_SPECTRAL_CENTROID](spectrum, numFrames, NULL, &centroid);
    printf("\nSpectral Centroid: %f\n", centroid);

    argd[1] = 10.0; //* peak threshold as %  of maximum peak 
    xtract[XTRACT_PEAK_SPECTRUM](spectrum, numFrames / 2, argd, peaks);

    argd[0] = f0;
    argd[1] = .3; //* harmonic threshold 
    xtract[XTRACT_HARMONIC_SPECTRUM](peaks, numFrames, argd, harmonics);

    //* print the spectral bins
    printf("\nSpectrum:\n");
    for(n = 0; n < (numFrames >> 1); ++n)
    {
        printf("freq: %.1f\tamp: %.6f", spectrum[n + (numFrames >> 1)], spectrum[n]);
        if (peaks[n + (numFrames >> 1)] != 0.f)
        {
            printf("\tpeak:: freq: %.1f\tamp: %.6f\n", peaks[n + (numFrames >> 1)], peaks[n]);
        }
        else
        {
            printf("\n");
        }
    }
    printf("\n");

    //* compute the MFCCs 
    mel_filters.n_filters = MFCC_FREQ_BANDS;
    mel_filters.filters   = (double **)malloc(MFCC_FREQ_BANDS * sizeof(double *));
    for(n = 0; n < MFCC_FREQ_BANDS; ++n)
    {
        mel_filters.filters[n] = (double *)malloc(numFrames * sizeof(double));
    }

    xtract_init_mfcc(numFrames >> 1, SAMPLERATE >> 1, XTRACT_EQUAL_GAIN, MFCC_FREQ_MIN, MFCC_FREQ_MAX, mel_filters.n_filters, mel_filters.filters);
    xtract_mfcc(spectrum, numFrames >> 1, &mel_filters, mfccs);

    //* print the MFCCs 
    printf("MFCCs:\n");
    for(n = 0; n < MFCC_FREQ_BANDS; ++n)
    {
        printf("band: %d\t", n);
        if(n < 10) {
            printf("\t");
        }
        printf("coeff: %f\n", mfccs[n]);
    }

    //* compute Spectral Flux 
    argd[0] = SAMPLERATE / (numFrames/2);
    argd[1] = XTRACT_MAGNITUDE_SPECTRUM;
    argd[2] = 0.f; //* DC component 
    argd[3] = 0.f; //* No Normalisation 
    
    xtract_init_fft(numFrames/2, XTRACT_SPECTRUM);
    xtract_features_from_subframes(wavetable, numFrames, XTRACT_SPECTRUM, argd, subframes);
    xtract_difference_vector(subframes, numFrames, NULL, difference);
    
    argd[0] = 1.0; //* norm order 
    argd[1] = XTRACT_POSITIVE_SLOPE; //* positive slope 
    
    xtract_flux(difference, numFrames/2, argd, &flux);
    
    printf("Flux: %f\n", flux);

    // cleanup 
    for(n = 0; n < MFCC_FREQ_BANDS; ++n)
    {
        free(mel_filters.filters[n]);
    }
    free(mel_filters.filters);

    return 0;

}
