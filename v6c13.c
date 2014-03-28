#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <portsf.h>

enum{ARG_NAME, ARG_INPUT, ARG_OUTPUT, ARG_DUR, ARG_MAX_GRAINDUR, ARGC};

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
  float minGrainDur = 0.2f;
  float maxGrainDur;
  
  float *grain;
  //Buffer to store individual grain

  if(argc!=ARGC)
  {
    printf("Please use v6c13.out as: v6c13.out INPUT_WAV OUTPUT_WAV OUTPUT_DURATION(seconds) MAXGRAINDURATION(seconds)\n");
    return 1;
  }

  duration = atof(argv[ARG_DUR]);
  maxGrainDur = atof(argv[ARG_MAX_GRAINDUR]);
 
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
    //grainDur  = (float)(minGrainDur+(maxGrainDur-minGrainDur))*rand()/RAND_MAX;
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
    psf_sndSeek(infile, seekOffset, PSF_SEEK_CUR);
    //sets seek of input file
    
    psf_sndReadFloatFrames(infile, grain, numFrames);
    //Read grain into grain from input

    psf_sndWriteFloatFrames(outfile, grain, numFrames);
    //Write grain to output 

    free(grain);
  }

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


