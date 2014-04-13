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

  setupVariables(
      &grain, 
      &output, 
      &global);

  allocateOutputMem(&output);

  while(global.spaceLeft)
  {
    allocateGrainMem(&grain, &global);

    setupSeek(&grain);

    psf_sndSeek(
	grain.inputFile, 
	grain.seekOffset, 
	PSF_SEEK_SET);						//sets seek of input file 

    psf_sndReadFloatFrames(
	grain.inputFile, 
	grain.buffer, 
	grain.numFrames);						//Read grain into grain from input 

    impAttackEnv(
	&grain, 
	&global);

    impDecayEnv(&grain, &global);

    setGrainX(&grain);
    //printf("Seek offset %d \t NumFrames %d \t FileSize %d\t grainDur %f \n", seekOffset, numFrames,psf_sndSize(infile), grainDur);

    if(output.NumFrames - output.step > grain.numFrames)
    {
      global.spaceLeft 	= 1;

      float *grainStart = output.buffer + output.step * output.outprop.chans;
      for(int i = 0; i < grain.numFrames; i++)
      {
	//printf("NumFrames: \t %d \t stepSize \t %ld \t numFrames: \t %d \t outputNumFrames - stepSize: \t %ld \t \n", outputNumFrames, stepSize, numFrames, outputNumFrames - stepSize);
	grainStart[i] 	= (sqrt(2.0) / 2) * (cos(grain.grainX) + sin(grain.grainX)) * grain.buffer[i] * 0.5;
	grainStart[i+1] = (sqrt(2.0) / 2) * (cos(grain.grainX) - sin(grain.grainX)) * grain.buffer[i] * 0.5;
	//printf("frame: %d \t numFrames: %d \t NumFrames: %d \t \n",i, numFrames, outputNumFrames);
      }

      output.step 	+= output.stepSize;
    } else
    {
      global.spaceLeft 	= 0;
    }
    //printf("Memory freed\n");
  }

  if(psf_sndWriteFloatFrames(
	output.outputFile, 
	output.buffer, 
	output.NumFrames) != output.NumFrames)
  {
    printf("Warning: error writing %s\n", global.argv[ARG_OUTPUT]);
    return 1;
  }
  //printf("finishedWriting\n");

  cleanUp(
      &grain, 
      &output, 
      &global);

  return 0;
}

int setGrainX(GRAIN *grain)
{
  grain->grainX = (-1.0f) + (float) rand() / ((float) (RAND_MAX/(1.0-(-1.0))));

  return 0;
}

int closePSF(
    GRAIN *grain, 
    GRANSOUND *output, 
    GLOBAL *global)
{
  if(grain->inputFile >= 0)
  {
    if(psf_sndClose(grain->inputFile))
    {
      printf("Warning error closing %s\n", global->argv[ARG_INPUT]);
    }
  }
  if(output->outputFile >= 0)
  {
    if(psf_sndClose(output->outputFile))
    {
      printf("Warning error closing %s\n", global->argv[ARG_OUTPUT]);
    }
    printf("Closed fine \n");
  }

  psf_finish();

  return 0;
}

int initialisePSF(
    GRAIN *grain, 
    GRANSOUND *output, 
    GLOBAL *global)
{
  if(psf_init())
  {
    printf("Error: unable to open portsf\n");
    return 1;
  }

  grain->inputFile = psf_sndOpen(global->argv[ARG_INPUT], &grain->inprop, 0);
  if(grain->inputFile < 0)
  {
    printf("Error, unable to read %s\n", global->argv[ARG_INPUT]);
    return 1;
  }

  output->outprop 	= grain->inprop;
  output->outprop.chans = 2;
  output->outputFile 	= psf_sndCreate(
      global->argv[ARG_OUTPUT], 
      &output->outprop, 
      0, 
      0, 
      PSF_CREATE_RDWR);
  if(output->outputFile < 0)
  {
    printf("Error, unable to create %s\n", global->argv[ARG_OUTPUT]);
    return 1;
  }

  return 0;
}

int setupVariables(
    GRAIN *grain, 
    GRANSOUND *output, 
    GLOBAL *global)
{
  global->minGrainDur 		= atof(global->argv[ARG_MIN_GRAINDUR]);
  global->maxGrainDur 		= atof(global->argv[ARG_MAX_GRAINDUR]);
  global->grainAttackPercent 	= atoi(global->argv[ARG_GRAIN_ATTACK]);
  global->grainDecayPercent 	= atoi(global->argv[ARG_GRAIN_DECAY]);

  output->duration 		= atof(global->argv[ARG_DUR]);
  output->grainDensity 		= 1.0 / atof(global->argv[ARG_GRAIN_DENSITY]);

  initialisePSF(
      grain, 
      output, 
      global);

  output->stepSize 		= output->grainDensity * output->outprop.srate;

  return 0;
}

int allocateOutputMem(GRANSOUND *output)
{
  output->NumFrames 	= (int)output->duration * output->outprop.srate;
  output->buffer 	= (float*) calloc(1, output->NumFrames * output->outprop.chans * sizeof(float));

  return 0;
}

int allocateGrainMem(GRAIN *grain, GLOBAL *global)
{
  grain->grainDur 	= global->minGrainDur + ((float) (global->maxGrainDur - global->minGrainDur) * rand()) / RAND_MAX;
  grain->numFrames 	= (int) (grain->grainDur * grain->inprop.srate);
  grain->buffer 	= (float*) malloc(grain->numFrames * grain->inprop.chans * sizeof(float));

  return 0;
}

int setupSeek(GRAIN *grain)
{
  grain->maxSeek 	= psf_sndSize(grain->inputFile) - grain->numFrames;
  grain->seekOffset 	= ((float) grain->maxSeek * rand()) / RAND_MAX;

  return 0;
}

int impAttackEnv(GRAIN *grain, GLOBAL *global)
{
  long grainAttack 	= global->grainAttackPercent * grain->numFrames / 100.0;
  float factor 		= 0.0;
  float increment 	= 1.0 / grainAttack;

  for(int i = 0; i < grainAttack; i++)
  {
    grain->buffer[i] 	= factor * grain->buffer[i];
    factor 		+= increment;
  }

  return 0;
}

int impDecayEnv(GRAIN* grain, GLOBAL *global)
{
  long grainDecay 	= global->grainDecayPercent * grain->numFrames / 100.0;
  float *decayStart 	= grain->buffer + (grain->numFrames - grainDecay);
  float factor 		= 1.0;
  float increment 	= 1.0 / grainDecay;

  for(int i = 0; i < grainDecay; i++)
  {
    decayStart[i] 	= factor * decayStart[i];
    factor 		-= increment;
  }

  return 0;
}

int cleanUp(GRAIN *grain, GRANSOUND *output, GLOBAL *global)
{
  free(output->buffer);
  free(grain->buffer);

  closePSF(
      grain, 
      output, 
      global);

  return 0;
}
