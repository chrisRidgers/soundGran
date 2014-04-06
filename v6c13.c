#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <portsf.h>

enum{ARG_NAME, ARG_INPUT, ARG_OUTPUT, ARG_DUR, ARG_MIN_GRAINDUR, ARG_MAX_GRAINDUR, ARG_GRAIN_ATTACK, ARG_GRAIN_DECAY, ARGC};

int intialisePSF(int *infile, int *outfile, PSF_PROPS *props, char **argv);
int setupVariables(float *duration, float *minGrainDur, float *maxGrainDur, int *grainAttackPercent, int *grainDecayPercent, char **argv);
int allocateGrainMem(float *grainDur, float *minGrainDur, float *maxGrainDur, int *numFrames, float **grain, PSF_PROPS *props);

int main(int argc, char *argv[])
{
  PSF_PROPS props;

  int infile;
  int outfile;//To test that PSF opens files successfully 
  
  float minGrainDur;
  float maxGrainDur;
  float duration; //Length of output file in seconds 
  int grainAttackPercent;
  int grainDecayPercent;//Stores perecentage of grain to be used for attack and delay 

  float grainDur;//Length of a grain - Determined using minGrainDur and maxGrainDur 
  int numFrames;
  float *grain;
  //Buffer to store individual grain

  if(argc!=ARGC)
  {
    printf("Please use v6c13.out as: v6c13.out INPUT_WAV OUTPUT_WAV OUTPUT_DURATION(seconds) MINGRAINDURATION(seconds) MAXGRAINDURATION(seconds) GRAINATTACKDURATION(percent) GRAINDECAYDURATION(percent)\n");
    return 1;
  }
  setupVariables(&duration, &minGrainDur, &maxGrainDur, &grainAttackPercent, &grainDecayPercent, argv);

  /*
  duration = atof(argv[ARG_DUR]);
  maxGrainDur = atof(argv[ARG_MAX_GRAINDUR]);
  minGrainDur = atof(argv[ARG_MIN_GRAINDUR]);
  */

                                         /*
  //Initialisation of psf library
  if(psf_init())
  {
    printf("Error: unable to open portsf\n");
    return 1;
  }

  infile = psf_sndOpen(argv[ARG_INPUT], &props, 0);
  if(infile<0)
  {
    printf("Error, unable to read %s\n", argv[ARG_INPUT]);
    return 1;
  }

  outfile = psf_sndCreate(argv[ARG_OUTPUT], &props, 0, 0, PSF_CREATE_RDWR);
  if(outfile<0)
  {
    printf("Error, unable to create %s\n", argv[ARG_OUTPUT]);
    return 1;
  }
  //End psf library initialisation
  */

  intialisePSF(&infile, &outfile, &props, argv);

  for(float totalDur = 0.0f; totalDur < duration; totalDur += grainDur)
  {
    /*
       grainDur  = minGrainDur+((float)(maxGrainDur-minGrainDur)*rand())/RAND_MAX;
    //grainDur  = (float)(maxGrainDur)*rand()/RAND_MAX;
    //Calculate grainDur using minGrainDur and maxGrainDur
    numFrames = grainDur*props.srate;
    //Calculate numFrames based on props sample rate
    grain = (float*)malloc(numFrames*props.chans*sizeof(float));
    //Allocate buffer for grain
    */

    allocateGrainMem(&grainDur, &minGrainDur, &maxGrainDur, &numFrames, &grain, &props);

    int maxSeek = psf_sndSize(infile)-numFrames;
    //calculate maximum seek position
    int seekOffset = ((float)maxSeek*rand())/RAND_MAX;
    //calculate random seek position within range defined by maxSeek
    psf_sndSeek(infile, seekOffset, PSF_SEEK_SET);
    //sets seek of input file

    //printf("Seek offset %d\t NumFrames %d \t FileSize %d\t grainDur %f \n", seekOffset, numFrames,psf_sndSize(infile), grainDur);

    psf_sndReadFloatFrames(infile, grain, numFrames);
    //Read grain into grain from input

    long grainAttack = grainAttackPercent*numFrames/100.0;
    long grainDecay = grainDecayPercent*numFrames/100.0;

    float factor = 0.0;
    float increment = 1.0/grainAttack;

    for(int i=0; i<grainAttack; i++)
    {
      grain[i] = factor*grain[i];
      factor+=increment;
    }

    float *decayStart = grain+(numFrames-grainDecay);

    factor = 1.0;
    increment = 1.0/grainDecay;

    for(int i=0; i<grainDecay; i++)
    {
      decayStart[i] = factor * decayStart[i];
      factor -= increment;
    }

    if(psf_sndWriteFloatFrames(outfile, grain, numFrames) != numFrames)
    {
      printf("Warning: error writing %s\n", argv[ARG_OUTPUT]);
      return 1;
    }
    //Write grain to output 
    //printf("finishedWriting\n");

    free(grain);
    //printf("Memory freed\n");
  }
  printf("loop ended\n");

  /*
     printf("Writing %s ... \n", argv[ARG_OUTPUT]);
     if(psf_sndWriteFloatFrames(outfile, outBuffer, numFrames) != numFrames)
     {
     printf("Warning: error writing %s\n", argv[ARG_OUTPUT]);
     return 1;
     }
     */

  if(infile >= 0)
  {
    if(psf_sndClose(infile))
    {
      printf("Warning error closing %s\n", argv[ARG_INPUT]);
    }
  }
  if(outfile >= 0)
  {
    if(psf_sndClose(outfile))
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
  //End psf library initialisation
  return 0;
}

int setupVariables(float *duration, float *minGrainDur, float *maxGrainDur, int *grainAttackPercent, int *grainDecayPercent, char **argv)
{
  *duration = atof(argv[ARG_DUR]);
  *maxGrainDur = atof(argv[ARG_MAX_GRAINDUR]);
  *minGrainDur = atof(argv[ARG_MIN_GRAINDUR]);
  *grainAttackPercent = atoi(argv[ARG_GRAIN_ATTACK]);
  *grainDecayPercent = atoi(argv[ARG_GRAIN_DECAY]);

  printf("*duration: \t %f \t *maxGrainDur \t %f \t *minGrainDur \t %f \t \n", *duration, *maxGrainDur, *minGrainDur);
  return 0;
}

int allocateGrainMem(float *grainDur, float *minGrainDur, float *maxGrainDur, int *numFrames, float **grain, PSF_PROPS *props)
{
  *grainDur  = *minGrainDur+((float)(*maxGrainDur-*minGrainDur)*rand())/RAND_MAX;
  //Calculate grainDur using minGrainDur and maxGrainDur
  *numFrames = (int)(*grainDur*props->srate);
  //printf("*TEMPVALUE: \t %d \t *props->srate: \t %d \t *grainDur: \t %f \t *minGrainDur \t %f \t *maxGrainDur \t %f \t *numFrames:\t %d \n", (int)(((int)*grainDur)*(props->srate)), props->srate, *grainDur, *minGrainDur, *maxGrainDur, *numFrames);
  //Calculate numFrames based on props sample rate
  *grain = (float*)malloc(*numFrames*props->chans*sizeof(float));
  //Allocate buffer for grain
  return 0;
}


