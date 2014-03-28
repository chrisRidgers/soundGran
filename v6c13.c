#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <portsf.h>

enum{ARG_NAME, ARG_INPUT, ARG_OUTPUT, ARG_DUR, ARG_MIN_GRAINDUR, ARG_MAX_GRAINDUR, ARG_GRAIN_ATTACK, ARG_GRAIN_DECAY, ARGC};

int main(int argc, char *argv[])
{
  PSF_PROPS props;

  int infile;
  int outfile;
  //To test that PSF opens files successfully
  
  long numFrames;

  float duration;
  //Length of output file in seconds
  
  float grainDur;
  //Length of a grain - Determined using minGrainDur and maxGrainDur
  float minGrainDur;
  float maxGrainDur;
  
  float *grain;
  //Buffer to store individual grain

  if(argc!=ARGC)
  {
    printf("Please use v6c13.out as: v6c13.out INPUT_WAV OUTPUT_WAV OUTPUT_DURATION(seconds) MINGRAINDURATION(seconds) MAXGRAINDURATION(seconds) GRAINATTACKDURATION(percent) GRAINDECAYDURATION(percent)\n");
    return 1;
  }

  duration = atof(argv[ARG_DUR]);
  maxGrainDur = atof(argv[ARG_MAX_GRAINDUR]);
  minGrainDur = atof(argv[ARG_MAX_GRAINDUR]);
 
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

  for(float totalDur = 0.0f; totalDur < duration; totalDur += grainDur)
  {
    //grainDur  = (float)(minGrainDur+(maxGrainDur-minGrainDur))*(float)rand()/RAND_MAX;
    grainDur  = (float)(maxGrainDur)*rand()/RAND_MAX;
    //Calculate grainDur using minGrainDur and maxGrainDur
    numFrames = (long)(grainDur*props.srate);
    //Calculate numFrames based on props sample rate
    grain = (float*)malloc(numFrames*props.chans*sizeof(float));
    //Allocate buffer for grain
    
    long maxSeek = psf_sndSize(infile)-numFrames;
    //calculate maximum seek position
    long seekOffset = maxSeek*rand()/RAND_MAX;
    //calculate random seek position within range defined by maxSeek
    psf_sndSeek(infile, seekOffset, PSF_SEEK_SET);
    //sets seek of input file
    
    psf_sndReadFloatFrames(infile, grain, numFrames);
    //Read grain into grain from input

    int grainAttackPercent = atoi(argv[ARG_GRAIN_ATTACK]);
    int grainDecayPercent = atoi(argv[ARG_GRAIN_DECAY]);
    //Stores perecentage of grain to be used for attack and delay

    long grainAttack = 20;
    long grainDecay = 20;

    float factor = 0.0;
    float increment = 1.0/grainAttack;

    for(int i=0; i<grainAttack; i++)
    {
	    grain[i] = factor*grain[i];
	    factor+=increment;
    }

    float *decayStart = grain+(numFrames-grainDecay-1);

    factor = 1.0;
    increment = 1.0/grainDecay;

    for(int i=0; i<grainDecay; i++)
    {
	    decayStart[i] = factor * decayStart[i];
	    factor -= increment;
    }

    psf_sndWriteFloatFrames(outfile, grain, numFrames);
    //Write grain to output 
    printf("finishedWriting\n");

    free(grain);
    printf("Memory freed\n");
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


