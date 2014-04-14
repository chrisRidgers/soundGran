#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <portsf.h>
#include <getopt.h>
#include "v6c13b.h"

int main(int argc, char *argv[])
{
  GLOBAL 	global = {
    argv, 
    argc,
    1, 
    0, 
    0, 
    0,
    { {"interactive", no_argument, &global.interactive, 1},
      {"help", no_argument, &global.help, 1},
      {"verbose", no_argument, &global.verbose, 1},
      {0, 0, 0, 0}},
    0}; 	
  GRAIN 	grain; 			//Struct to hold grain related variables
  GRANSOUND 	output = {0,0}; 	//Struct to hold variables affecting the output buffer
  INITPSF	initStruct = {T_DEFAULT};
  /*
     if(argc!=ARGC)
     {
     printf("Please use v6c13.out as: v6c13.out INPUT_WAV OUTPUT_WAV OUTPUT_DURATION(seconds) MINGRAINDURATION(seconds) MAXGRAINDURATION(seconds) GRAINATTACKDURATION(percent) GRAINDECAYDURATION(percent) GRAINDENSITY(grains per second)\n");

     return 1;
     }
     */

  setupVariables(
      &grain, 
      &output, 
      &global,
      &initStruct);

  allocateOutputMem(&output); 		//Setting up output buffer

  while(global.spaceLeft)
  {
    allocateGrainMem(&grain, &global); 	//Setting up grain buffer
    setupSeek(&grain);                  //Sets random seek for input file

    psf_sndSeek(  			//Sets seek for input file
	grain.inputFile, 
	grain.seekOffset, 
	PSF_SEEK_SET);						

    psf_sndReadFloatFrames( 		//Reads grain into buffer
	grain.inputFile, 
	grain.buffer, 
	grain.numFrames);					   

    impAttackEnv(&grain, &global); 	//Applies an attack envelope to reduce clipping
    impDecayEnv(&grain, &global);	//Applies a decay envelope to reduce clipping
    setGrainX(&grain);			//Randomly determines a sound source position (stereo)

    if(output.NumFrames - output.step > grain.numFrames)
    {
      global.spaceLeft 	= 1;		//Just in case variable somehow becomes unset

      float *grainStart = output.buffer + output.step * output.outprop.chans;
      //Pointer to first frame of grain, adjusts by stepsize each loop
      for(int i = 0; i < grain.numFrames; i++)
      {
	grainStart[i] 	= grain.panInfo.left * grain.buffer[i];
	grainStart[i + 1] = grain.panInfo.right * grain.buffer[i];
      }

      output.step 	+= output.stepSize;
      //Increases step by stepsize for next grain
    } else
    {
      global.spaceLeft 	= 0;		//Prevents loop from running once finished
    }

    free(grain.buffer);
    grain.bufTest 	= 0;
  }

  if(psf_sndWriteFloatFrames(		//Writes output buffer to file
	output.outputFile, 
	output.buffer, 
	output.NumFrames) != output.NumFrames)
  {
    printf("Warning: error writing %s\n", global.argv[ARG_OUTPUT]);
    return 1;
  }

cleanup:
  cleanUp(				//Frees up memory buffers before programme exit
      &grain, 
      &output, 
      &global);

  return 0;
}

int setupVariables(
    GRAIN *grain, 
    GRANSOUND *output, 
    GLOBAL *global,
    INITPSF *initStruct)
{
  int c;
  while(( c = getopt_long(
	  global->argc,
	  global->argv,
	  "ihv",
	  global->long_options,
	  &global->option_index)) != -1)
  {
    switch(c)
    {
      case 0:
	if(global->long_options[global->option_index].flag != 0)
	  break;
	printf("options %s", global->long_options[global->option_index].name);
	if(optarg)
	  printf (" with arg %s", optarg);
	printf("\n");

      case 'i':
	global->interactive = 1;
	break;

      case 'h':
	global->help = 1;

      case 'v':
	global->verbose = 1;

      case '?':
	break;

      default:
	abort();

    }
  }

  if(global->help)
  {
    printf("\n v6c13.c sound granulator works as follows:\n \n \tPlease supply options in order of: \n \t \t INPUT(wav/aif file) \n \t \t OUTPUT(wav/aif file) \n \t \t DURATION(seconds) \n \t \t MINIMUMGRAINLENGTH(seconds) \n \t \t MAXIMUMGRAINLENGTH(seconds) \n \t \t ATTACKLENGTH(percentage of grain duration) \n \t \t ATTACKDECAY(percentage of grain duration) \n \t \t GRAINDENSITY(number of grains per secondn)\n \n \t \t Example: \n \t \t \t (./)v6c13.out input.wav output.wav 20 0.2 0.4 10 10 40 \n \n \t Also, the following options may be useful: \n \t \t -h, --help \t \t Display's this dialogue \n \t \t -v, --verbose \t \t Run's the programme while updating the user via the command line. \n \t \t -i, --interactive \t Force the programme to ignore all other arguments (bar help) and request input from the user to produce an output \n \n");
    exit(0);
  }
  if(global->interactive)
  {
    initStruct->type = T_INTERACTIVE;
    printf("Running interactively: \n \n");

    printf("Please enter a valid input file (less than 20 characters) e.g sample.wav OR ./path/to/sample.wav: ");
    scanf("%s", global->inputFile);

    printf("\n Please enter a valid output file (less than 20 characters) e.g output.wav: ");
    scanf("%s", global->outputFile);

    printf("\n Please enter a valid output duration e.g 20: ");
    scanf("%f", &output->duration);

    printf("\n Please enter a valid minimum grain duration e.g 0.2: ");
    scanf("%f", &global->minGrainDur);

    printf("\n Please enter a valid maximum grain duration e.g 0.4: ");
    scanf("%f", &global->maxGrainDur);

    printf("\n Please enter a valid percentage value for the grain attack envelope e.g 10: ");
    scanf("%d", &global->grainAttackPercent);

    printf("\n Please enter a valid percentage value for the graind decay envelope e.g 10: ");
    scanf("%d", &global->grainDecayPercent);

    printf("\n Please enter a valid value for the number of grains per second e.g 25: ");
    float grainDensity;
    scanf("%f", &grainDensity);
    output->grainDensity = 1.0 / grainDensity;

    printf("\n");

  }

  setOverloadPSF(initStruct, grain, output, global, &optind);
  initialisePSF(initStruct);


  /*initialisePSF(
    grain, 
    output, 
    global,
    &optind);
    */

  output->duration 		= atof(global->argv[ARG_DUR + optind - 1]);
  //printf("%f \n", output->duration);
  global->minGrainDur 		= atof(global->argv[ARG_MIN_GRAINDUR + optind - 1]);
  //printf("%f \n", global->minGrainDur);
  global->maxGrainDur 		= atof(global->argv[ARG_MAX_GRAINDUR + optind - 1]);
  //printf("%f \n", global->maxGrainDur);
  global->grainAttackPercent 	= atoi(global->argv[ARG_GRAIN_ATTACK + optind - 1]);
  //printf("%d \n", global->grainAttackPercent);
  global->grainDecayPercent 	= atoi(global->argv[ARG_GRAIN_DECAY + optind - 1]);
  //printf("%d \n", global->grainDecayPercent);

  output->grainDensity 		= 1.0 / atof(global->argv[ARG_GRAIN_DENSITY + optind - 1]);
  //printf("%f \n", output->grainDensity);



  output->stepSize 		= output->grainDensity * output->outprop.srate;
  output->bufTest 		= 0;
  grain->bufTest		= 0;


  return 0;
}

int initialisePSF2(
    GRAIN *grain, 
    GRANSOUND *output, 
    GLOBAL *global,
    int *optind)
{
  if(psf_init())
  {
    printf("Error: unable to open portsf\n");
    return 1;
  }

  grain->inputFile = psf_sndOpen(global->argv[ARG_INPUT + (*optind) - 1], &grain->inprop, 0);
  if(grain->inputFile < 0)
  {
    printf("Error, unable to read %s\n", global->argv[ARG_INPUT + (*optind) - 1]);
    return 1;
  }

  output->outprop 	= grain->inprop;
  output->outprop.chans = 2;
  output->outputFile 	= psf_sndCreate(
      global->argv[ARG_OUTPUT + (*optind) - 1], 
      &output->outprop, 
      0, 
      0, 
      PSF_CREATE_RDWR);
  if(output->outputFile < 0)
  {
    printf("Error, unable to create %s\n", global->argv[ARG_OUTPUT + (*optind) - 1]);
    return 1;
  }

  return 0;
}

int allocateOutputMem(GRANSOUND *output)
{
  output->NumFrames 	= (int)output->duration * output->outprop.srate;
  output->buffer 	= (float*) calloc(1, output->NumFrames * output->outprop.chans * sizeof(float));
  output->bufTest	= 1;

  return 0;
}

int allocateGrainMem(GRAIN *grain, GLOBAL *global)
{
  grain->grainDur 	= global->minGrainDur + ((float) (global->maxGrainDur - global->minGrainDur) * rand()) / RAND_MAX;
  grain->numFrames 	= (int) (grain->grainDur * grain->inprop.srate);
  grain->buffer 	= (float*) malloc(grain->numFrames * grain->inprop.chans * sizeof(float));
  grain->bufTest	= 1;

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

int setGrainX(GRAIN *grain)
{
  grain->panInfo.grainX = (-1.0f) + (float) rand() / ((float) (RAND_MAX/(1.0-(-1.0))));
  //grain->panInfo.left = (sqrt(2.0) / 2) * (cos(grain->panInfo.grainX) - sin(grain->panInfo.grainX)) * 0.5;
  //grain->panInfo.right = (sqrt(2.0) / 2) * (cos(grain->panInfo.grainX) + sin(grain->panInfo.grainX)) * 0.5;

  float piovr2 = M_PI * 0.5;
  float root2ovr2 = sqrt(2.0) * 0.5;
  float thispos = grain->panInfo.grainX * piovr2;
  float angle = thispos * 0.5;

  grain->panInfo.left = root2ovr2 * (cos(angle) - sin(angle));
  grain->panInfo.right = root2ovr2 * (cos(angle) + sin(angle));

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
      printf("Warning error closing %s\n", global->argv[ARG_INPUT + optind - 1]);
    }
  }
  if(output->outputFile >= 0)
  {
    if(psf_sndClose(output->outputFile))
    {
      printf("Warning error closing %s\n", global->argv[ARG_OUTPUT + optind - 1]);
    }
    printf("Closed fine \n");
  }

  psf_finish();

  return 0;
}

int cleanUp(GRAIN *grain, GRANSOUND *output, GLOBAL *global)
{
  if(output->bufTest)free(output->buffer), output->bufTest 	= 0;
  if(grain->bufTest)free(grain->buffer), grain->bufTest 	= 0;

  closePSF(
      grain, 
      output, 
      global);

  return 0;
}

int setOverloadPSF(INITPSF *initStruct, GRAIN *grain, GRANSOUND *output, GLOBAL *global, int *optind)
{
  switch(initStruct->type)
  {
    case T_DEFAULT:
      initStruct->grain = grain;
      initStruct->output = output;
      initStruct->global = global;
      initStruct->optind = optind;
      break;
    case T_INTERACTIVE:
      initStruct->grain = grain;
      initStruct->output = output;
      initStruct->global = global;
      initStruct->inputFile = global->inputFile;
      initStruct->outputFile = global->outputFile;
      break;
  }

  return 0;
}

int initialisePSF(INITPSF *initStruct)
{ 
  if(psf_init())
  {
    printf("Error: unable to open portsf\n");
    return 1;
  }

  switch(initStruct->type)
  {
    case T_DEFAULT:

      initStruct->grain->inputFile = psf_sndOpen(initStruct->global->argv[ARG_INPUT + *(initStruct->optind) - 1], &initStruct->grain->inprop, 0);
      if(initStruct->grain->inputFile < 0)
      {
	printf("Error, unable to read %s\n", initStruct->global->argv[ARG_INPUT + *(initStruct->optind) - 1]);
	return 1;
      }

      initStruct->output->outprop 	= initStruct->grain->inprop;
      initStruct->output->outprop.chans = 2;
      initStruct->output->outputFile 	= psf_sndCreate(
	  initStruct->global->argv[ARG_OUTPUT + *(initStruct->optind) - 1], 
	  &initStruct->output->outprop, 
	  0, 
	  0, 
	  PSF_CREATE_RDWR);

      if(initStruct->output->outputFile < 0)
      {
	printf("Error, unable to create %s\n", initStruct->global->argv[ARG_OUTPUT + *(initStruct->optind) - 1]);
	return 1;
      }

      break;

    case T_INTERACTIVE:

      initStruct->grain->inputFile = psf_sndOpen(initStruct->inputFile, &initStruct->grain->inprop, 0); //??????
      if(initStruct->grain->inputFile < 0)
      {
	printf("Error, unable to read %s\n", initStruct->inputFile);
	return 1;
      }

      initStruct->output->outprop 	= initStruct->grain->inprop;
      initStruct->output->outprop.chans = 2;
      initStruct->output->outputFile 	= psf_sndCreate(
	  initStruct->outputFile, 
	  &initStruct->output->outprop, 
	  0, 
	  0, 
	  PSF_CREATE_RDWR);
      if(initStruct->output->outputFile < 0)
      {
	printf("Error, unable to create %s\n", initStruct->outputFile);
	return 1;
      }

      break;
  }

  return 0;
}

