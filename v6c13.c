#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <portsf.h>

enum{ARG_NAME, ARG_INPUT, ARG_OUTPUT, ARG_DUR, ARG_MIN_GRAINDUR, ARG_MAX_GRAINDUR, ARG_GRAIN_ATTACK, ARG_GRAIN_DECAY, ARG_GRAIN_DENSITY, ARGC};

int intialisePSF(int *infile, int *outfile, PSF_PROPS *props, char **argv);
int closePSF(int *infile, int *outfile, char **argv);
int setupVariables(float *duration, float *minGrainDur, float *maxGrainDur, int *grainAttackPercent, int *grainDecayPercent, float *grainDensity, long *stepSize, PSF_PROPS *props, char **argv);
int allocateGrainMem(float *grainDur, float *minGrainDur, float *maxGrainDur, int *numFrames, float **grain, PSF_PROPS *props);
int allocateOutputMem(float *duration, int *outputNumFrames, float **output, PSF_PROPS *props);
int setupSeek(int *infile, int *numFrames, int *maxSeek, int *seekOffset);
int impAttackEnv(int *grainAttackPercent, int *numFrames, float *grain);
int impDecayEnv(int *grainDecayPercent, int *numFrames, float *grain);

int main(int argc, char *argv[])
{
  PSF_PROPS props;

  int spaceLeft=1;

  int infile;
  int outfile;			//To test that PSF opens files successfully 

  float duration; 		//Length of output file in seconds 
  int outputNumFrames;
  float *output;

  float minGrainDur;
  float maxGrainDur;
  float grainDur;		//Length of a grain - Determined using minGrainDur and maxGrainDur 
  int numFrames;
  float grainDensity;
  long stepSize=0;
  long step=0;
  float *grain;			//Buffer to store individual grain 

  int grainAttackPercent;
  int grainDecayPercent;	//Stores perecentage of grain to be used for attack and delay 

  int maxSeek;
  int seekOffset;


  if(argc!=ARGC)
  {
    printf("Please use v6c13.out as: v6c13.out INPUT_WAV OUTPUT_WAV OUTPUT_DURATION(seconds) MINGRAINDURATION(seconds) MAXGRAINDURATION(seconds) GRAINATTACKDURATION(percent) GRAINDECAYDURATION(percent)\n");
    return 1;
  }

  intialisePSF(&infile, &outfile, &props, argv);
  setupVariables(&duration, &minGrainDur, &maxGrainDur, &grainAttackPercent, &grainDecayPercent, &grainDensity, &stepSize, &props, argv);
  allocateOutputMem(&duration, &outputNumFrames, &output, &props);

  for(float totalDur = 0.0f; totalDur < duration; totalDur += grainDur)
  {
    allocateGrainMem(&grainDur, &minGrainDur, &maxGrainDur, &numFrames, &grain, &props);
    setupSeek(&infile, &numFrames, &maxSeek, &seekOffset);
    psf_sndSeek(infile, seekOffset, PSF_SEEK_SET);						//sets seek of input file 
    psf_sndReadFloatFrames(infile, grain, numFrames);						//Read grain into grain from input 
    impAttackEnv(&grainAttackPercent, &numFrames, grain);
    impDecayEnv(&grainDecayPercent, &numFrames, grain);
    //printf("Seek offset %d\t NumFrames %d \t FileSize %d\t grainDur %f \n", seekOffset, numFrames,psf_sndSize(infile), grainDur);
    
    float *grainStart = output + step;
    for(int i = 0; i < numFrames && grainDur < duration - totalDur; i++)
    {
      if(outputNumFrames - stepSize > numFrames)
      {
      printf("outputNumFrames: \t %d \t stepSize \t %ld \t numFrames: \t %d \t outputNumFrames - stepSize: \t %ld \t \n", outputNumFrames, stepSize, numFrames, outputNumFrames - stepSize);
      grainStart[i] = grain[i];
      //printf("frame: %d \t numFrames: %d \t outputNumFrames: %d \t \n",i, numFrames, outputNumFrames);
      }
    }
    step += stepSize;
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

  return 0;
}

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

int intialisePSF(int *infile, int *outfile, PSF_PROPS *props, char **argv)
{
  if(psf_init())
  {
    printf("Error: unable to open portsf\n");
    return 1;
  }

  *infile = psf_sndOpen(argv[ARG_INPUT], props, 0);
  if(*infile<0)
  {
    printf("Error, unable to read %s\n", argv[ARG_INPUT]);
    return 1;
  }

  *outfile = psf_sndCreate(argv[ARG_OUTPUT], props, 0, 0, PSF_CREATE_RDWR);
  if(*outfile<0)
  {
    printf("Error, unable to create %s\n", argv[ARG_OUTPUT]);
    return 1;
  }
  return 0;
}

int setupVariables(float *duration, float *minGrainDur, float *maxGrainDur, int *grainAttackPercent, int *grainDecayPercent, float *grainDensity, long *stepSize, PSF_PROPS *props, char **argv)
{
  *duration = atof(argv[ARG_DUR]);
  *maxGrainDur = atof(argv[ARG_MAX_GRAINDUR]);
  *minGrainDur = atof(argv[ARG_MIN_GRAINDUR]);
  *grainAttackPercent = atoi(argv[ARG_GRAIN_ATTACK]);
  *grainDecayPercent = atoi(argv[ARG_GRAIN_DECAY]);
  *grainDensity = 1.0/atof(argv[ARG_GRAIN_DENSITY]);
  *stepSize = (*grainDensity)*props->srate;
  printf("*duration: \t %f \t *maxGrainDur \t %f \t *minGrainDur \t %f \t *grainDensity \t %f \t *stepSize \t %ld \t \n", *duration, *maxGrainDur, *minGrainDur, *grainDensity, *stepSize);
  return 0;
}

int allocateOutputMem(float *duration, int *outputNumFrames, float **output, PSF_PROPS *props)
{
  *outputNumFrames = (int)(*duration)*props->srate;
  *output = (float*)calloc(1, (*outputNumFrames)*props->chans*sizeof(float));
  return 0;
}

int allocateGrainMem(float *grainDur, float *minGrainDur, float *maxGrainDur, int *numFrames, float **grain, PSF_PROPS *props)
{
  *grainDur  = *minGrainDur+((float)(*maxGrainDur-*minGrainDur)*rand())/RAND_MAX;		//Calculate grainDur using minGrainDur and maxGrainDur 
  *numFrames = (int)(*grainDur*props->srate);							//Calculate numFrames based on props sample rate 
  *grain = (float*)malloc(*numFrames*props->chans*sizeof(float));				//Allocate buffer for grain 
  //printf("*TEMPVALUE: \t %d \t *props->srate: \t %d \t *grainDur: \t %f \t *minGrainDur \t %f \t *maxGrainDur \t %f \t *numFrames:\t %d \n", (int)(((int)*grainDur)*(props->srate)), props->srate, *grainDur, *minGrainDur, *maxGrainDur, *numFrames);
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
