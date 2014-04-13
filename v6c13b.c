#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <portsf.h>
#include "v6c13b.h"


int main(int argc, char *argv[])
{
  GLOBAL 	global = {1, argv};
  GRAIN 	grain;
  GRANSOUND 	output = {0,0};

  if(argc!=ARGC)
  {
    printf("Please use v6c13.out as: v6c13.out INPUT_WAV OUTPUT_WAV OUTPUT_DURATION(seconds) MINGRAINDURATION(seconds) MAXGRAINDURATION(seconds) GRAINATTACKDURATION(percent) GRAINDECAYDURATION(percent) GRAINDENSITY(grains per second)\n");
    return 1;
  }
  
  setupVariables(&grain, &output, &global);
  //setupVariables(&duration, &minGrainDur, &maxGrainDur, &grainAttackPercent, &grainDecayPercent, &grainDensity, &stepSize, &inprop, &outprop, argv);
  //allocateOutputMem(&duration, &outputNumFrames, &output, &outprop);
   /*
  while(spaceLeft)
  {
    allocateGrainMem(&grainDur, &minGrainDur, &maxGrainDur, &numFrames, &grain, &inprop);
    setupSeek(&infile, &numFrames, &maxSeek, &seekOffset);
    psf_sndSeek(infile, seekOffset, PSF_SEEK_SET);						//sets seek of input file 
    psf_sndReadFloatFrames(infile, grain, numFrames);						//Read grain into grain from input 
    impAttackEnv(&grainAttackPercent, &numFrames, grain);
    impDecayEnv(&grainDecayPercent, &numFrames, grain);
    setGrainX(&grainX);
    printf("Variables: \t \n \n grainDur \t %f \t numFrames \t %d \t grain(length of) \t %lu \t \n maxSeek \t %d \t seekOffset \t %d \t grainX \t %f \t \n \n duration \t %f \t outputNumFrames \t %d \t \n stepSize \t %ld \t output(length of) \t %ld \t \n \n inprop: \t \n \n props.chans \t %d \t \n props.srate \t %d \t \n \n (size of float = %lu ) \t \n leftChannel \t %f \t rightChannel \t %f \t channelSum \t %f \t \n \n", grainDur, numFrames, numFrames*inprop.chans*sizeof(float), maxSeek, seekOffset, grainX, duration, outputNumFrames, stepSize, outputNumFrames*inprop.chans*sizeof(float), inprop.chans, inprop.srate, sizeof(float), (sqrt(2.0)/2) * (cos(grainX)+sin(grainX)) * 0.5, (sqrt(2.0)/2) * (cos(grainX)-sin(grainX)) * 0.5, ((sqrt(2.0)/2) * (cos(grainX)+sin(grainX)) * 0.5 ) + ((sqrt(2.0)/2) * (cos(grainX)-sin(grainX)) * 0.5 ));
    //printf("Seek offset %d \t NumFrames %d \t FileSize %d\t grainDur %f \n", seekOffset, numFrames,psf_sndSize(infile), grainDur);
    
    if(outputNumFrames - step > numFrames)
    {
      spaceLeft = 1;
      float *grainStart = output + step*outprop.chans;
      for(int i = 0; i < numFrames; i++)
      {
	//printf("outputNumFrames: \t %d \t stepSize \t %ld \t numFrames: \t %d \t outputNumFrames - stepSize: \t %ld \t \n", outputNumFrames, stepSize, numFrames, outputNumFrames - stepSize);
	grainStart[i] = (sqrt(2.0)/2) * (cos(grainX)+sin(grainX)) * grain[i] * 0.5;
	grainStart[i+1] = (sqrt(2.0)/2) * (cos(grainX)-sin(grainX)) * grain[i] * 0.5;
	//printf("frame: %d \t numFrames: %d \t outputNumFrames: %d \t \n",i, numFrames, outputNumFrames);
      }
      step += stepSize;
    } else
    {
      spaceLeft = 0;
    }
    free(grain);
    //printf("Memory freed\n");
  }
 
  printf("loop ended\n");

  

  if(psf_sndWriteFloatFrames(outfile, output, outputNumFrames) != outputNumFrames)
  {
    printf("Warning: error writing %s\n", argv[ARG_OUTPUT]);
    return 1;
  }
  //printf("finishedWriting\n");

  free(output);

  closePSF(&infile, &outfile, argv);
  */
  return 0;
}
 /*
int setGrainX(float *grainX)
{
  *grainX = (-1.0f)+ (float) rand() / ((float) (RAND_MAX/(1.0-(-1.0))));
  return 0;
}
*/
 /*
int closePSF(int *infile, int *outfile, char **argv)
{
  if(*infile >= 0)
  {
    if(psf_sndClose(*infile))
    {
      printf("Warning error closing %s\n", argv[ARG_INPUT]);
    }
  }
  if(*outfile >= 0)
  {
    if(psf_sndClose(*outfile))
    {
      printf("Warning error closing %s\n", argv[ARG_OUTPUT]);
    }
    printf("Closed fine \n");
  }

  psf_finish();
  return 0;
}
*/

int initialisePSF(GRAIN *grain, GRANSOUND *output, GLOBAL *global)
{
  if(psf_init())
  {
    printf("Error: unable to open portsf\n");
    return 1;
  }

  grain->inputFile = psf_sndOpen(global->argv[ARG_INPUT], grain->inprop, 0);
  if(grain->inputFile<0)
  {
    printf("Error, unable to read %s\n", global->argv[ARG_INPUT]);
    return 1;
  }

  output->outprop = grain->inprop;
  output->outprop->chans = 2;
  output->outputFile = psf_sndCreate(global->argv[ARG_OUTPUT], output->outprop, 0, 0, PSF_CREATE_RDWR);
  if(output->outputFile<0)
  {
    printf("Error, unable to create %s\n", global->argv[ARG_OUTPUT]);
    return 1;
  }
  return 0;
}
/*
int setupVariables(float *duration, float *minGrainDur, float *maxGrainDur, int *grainAttackPercent, int *grainDecayPercent, float *grainDensity, long *stepSize, PSF_PROPS *inprop, PSF_PROPS *outprop, char **argv)
{
  *duration = atof(argv[ARG_DUR]);
  *maxGrainDur = atof(argv[ARG_MAX_GRAINDUR]);
  *minGrainDur = atof(argv[ARG_MIN_GRAINDUR]);
  *grainAttackPercent = atoi(argv[ARG_GRAIN_ATTACK]);
  *grainDecayPercent = atoi(argv[ARG_GRAIN_DECAY]);
  *grainDensity = 1.0/atof(argv[ARG_GRAIN_DENSITY]);
  *stepSize = (*grainDensity)*outprop->srate;
  printf("*duration: \t %f \t *maxGrainDur \t %f \t *minGrainDur \t %f \t *grainDensity \t %f \t *stepSize \t %ld \t \n", *duration, *maxGrainDur, *minGrainDur, *grainDensity, *stepSize);
  return 0;
}
*/
int setupVariables(GRAIN *grain, GRANSOUND *output, GLOBAL *global)
{
  global->minGrainDur = atof(global->argv[ARG_MIN_GRAINDUR]);
  global->maxGrainDur = atof(global->argv[ARG_MAX_GRAINDUR]);
  global->grainAttackPercent = atoi(global->argv[ARG_GRAIN_ATTACK]);
  global->grainDecayPercent = atoi(global->argv[ARG_GRAIN_DECAY]);
  
  output->duration = atof(global->argv[ARG_DUR]);
  output->grainDensity = 1.0/atof(global->argv[ARG_GRAIN_DENSITY]);

  initialisePSF(grain, output, global);

  output->stepSize = output->grainDensity*output->outprop->srate;

  return 0;
}
/*

int allocateOutputMem(float *duration, int *outputNumFrames, float **output, PSF_PROPS *outprop)
{
  *outputNumFrames = (int)(*duration)*outprop->srate;
  *output = (float*)calloc(1, (*outputNumFrames)*outprop->chans*sizeof(float));
  return 0;
}

int allocateGrainMem(float *grainDur, float *minGrainDur, float *maxGrainDur, int *numFrames, float **grain, PSF_PROPS *inprop)
{
  *grainDur  = *minGrainDur+((float)(*maxGrainDur-*minGrainDur)*rand())/RAND_MAX;		//Calculate grainDur using minGrainDur and maxGrainDur 
  *numFrames = (int)(*grainDur*inprop->srate);							//Calculate numFrames based on props sample rate 
  *grain = (float*)malloc(*numFrames*inprop->chans*sizeof(float));				//Allocate buffer for grain 
  //printf("*TEMPVALUE: \t %d \t *inprop->srate: \t %d \t *grainDur: \t %f \t *minGrainDur \t %f \t *maxGrainDur \t %f \t *numFrames:\t %d \n", (int)(((int)*grainDur)*(props->srate)), props->srate, *grainDur, *minGrainDur, *maxGrainDur, *numFrames);
  return 0;
}

int setupSeek(int *infile, int *numFrames, int *maxSeek, int *seekOffset)
{
  *maxSeek = psf_sndSize(*infile)-*numFrames;							//calculate maximum seek position 
  *seekOffset = ((float)*maxSeek*rand())/RAND_MAX;						//calculate random seek position 
  //printf("*maxSeek: \t %d \t *seekOffset \t %d \t \n", *maxSeek, *seekOffset);
  return 0;
}

int impAttackEnv(int *grainAttackPercent, int *numFrames, float *grain)
{
  long grainAttack = (*grainAttackPercent)*(*numFrames)/100.0;
  float factor = 0.0;
  float increment = 1.0/grainAttack;

  for(int i=0; i<grainAttack; i++)
  {
    grain[i] = factor*grain[i];
    factor+=increment;
  }
  return 0;
}

int impDecayEnv(int *grainDecayPercent, int *numFrames, float *grain)
{
  long grainDecay = (*grainDecayPercent)*(*numFrames)/100.0;
  float *decayStart = grain+((*numFrames)-grainDecay);

  float factor = 1.0;
  float increment = 1.0/grainDecay;

  for(int i=0; i<grainDecay; i++)
  {
    decayStart[i] = factor * decayStart[i];
    factor -= increment;
  }
  return 0;
}

*/
