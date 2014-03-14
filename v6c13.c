#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <portsf.h>

enum{ARG_NAME, ARG_INPUT, ARG_OUTPUT, ARG_DUR, ARG_MIN_GRAINDUR, ARG_MAX_GRAINDUR, ARGC};

int main(int argc, char *argv[])
{
  PSF_PROPS props;
  int infile;
  int outfile;
  float *buffer;
  long num_frames;

  float *grains;
  float duration;
  float grain_duration;
  float min_grain_duration;
  float max_grain_duration;

  if(argc!=ARGC)
  {
    printf("Please use wav2aif as: wav2aif INPUT_WAV OUTPUT_AIF DURATION GRAINDURATION\n");
    return 1;
  }

  duration = atof(argv[ARG_DUR]);
  min_grain_duration = atof(argv[ARG_MIN_GRAINDUR])/1000;
  max_grain_duration = atof(argv[ARG_MAX_GRAINDUR])/1000;
 
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
    printf("Unable to create %s\n", argv[ARG_OUTPUT]);
    return 1;
  }

  //End psf library initialisation
    num_frames = (long)(max_grain_duration*props.srate);
    buffer = (float*)malloc(num_frames*props.chans*sizeof(float));
    if(buffer==NULL)
    {
      printf("Error unable to allocate buffer\n");
      if(psf_sndClose(infile))
      {
	printf("Warning: error closing %s\n", argv[ARG_INPUT]);
      }
      if(psf_sndClose(outfile))
      {
	printf("Warning: error closing %s\n", argv[ARG_OUTPUT]);
      }
      return 1;
    }
    printf("Reading %s to buffer \n", argv[ARG_INPUT]);

  for(float totalDur = 0.0f; totalDur < duration; totalDur += grain_duration)
  {
    grain_duration  = (float)min_grain_duration+(max_grain_duration-min_grain_duration)*rand()/RAND_MAX;
    num_frames = (long)(grain_duration*props.srate);

    printf("Grain duration:\t %f \t Num Frames: \t %ld \t Total Dur: \t %f \t Duration: \t %f \n", grain_duration, num_frames, totalDur, duration);

    if(psf_sndReadFloatFrames(infile, buffer, num_frames) !=num_frames)
    {
      printf("Warning: error reading %s\n", argv[ARG_OUTPUT]);
      return 1;
    }
    //finished buffer allocation

    //add attack and decay to buffer

    printf("Writing %s ... \n", argv[ARG_OUTPUT]);
    if(psf_sndWriteFloatFrames(outfile, buffer, num_frames) != num_frames)
    {
      printf("Warning: error writing %s\n", argv[ARG_OUTPUT]);
      return 1;
    }

  }

  free(buffer);

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
  printf("Completed format conversion\n");

  return 0;
}


