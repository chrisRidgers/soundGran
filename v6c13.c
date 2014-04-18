#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <math.h>
#include <portsf.h>
#include <getopt.h>
#include "v6c13.h"

int main(int argc, char *argv[])
{
  GLOBAL global 	= {							//Struct to hold global variables, keeps everything together
    argv, 
    argc,
    0,
    1, 
    0, 
    0, 
    0,
    0,
    0,
    {{"interactive", no_argument, &global.interactive, 1},
      {"help", no_argument, &global.help, 1},
      {"verbose", no_argument, &global.verbose, 1},
      {0, 0, 0, 0}},
    0}; 	

  GRAIN grain;									//Struct to hold grain related variables
  OUTPUT output 	= {0,0};						//Struct to hold variables affecting the output buffer
  INITPSF initStruct 	= {T_DEFAULT};

  if(setupVariables(
	&grain, 
	&output, 
	&global,
	&initStruct) == -1) goto cleanup;

  if(allocateOutputMem(&output, &global) == -1) goto cleanup;			//Setting up output buffer


  while(global.spaceLeft)
  {
    if(allocateGrainMem(&grain, &global) == -1) goto cleanup;			//Setting up grain buffer
    if(setupSeek(&grain, &global) == -1) goto cleanup;				//Sets random seek for input file

    psf_sndSeek(								//Sets seek for input file
	grain.inputFile, 
	grain.seekOffset, 
	PSF_SEEK_SET);						

    psf_sndReadFloatFrames(							//Reads grain into buffer
	grain.inputFile, 
	grain.buffer, 
	grain.numFrames);					   

    if(impAttackEnv(&grain, &global) == -1) goto cleanup;			//Applies an attack envelope to reduce clipping
    if(impDecayEnv(&grain, &global) == -1) goto cleanup;		        //Applies a decay envelope to reduce clipping
    if(setGrainX(&grain, &global) == -1 ) goto cleanup;				//Randomly determines a sound source position (stereo)

    if(output.NumFrames - output.step > grain.numFrames)
    {
      global.spaceLeft 	= 1;							//Just in case variable somehow becomes unset

      float *grainStart =  output.buffer + output.step * output.outprop.chans;             
      //Pointer to first frame of grain, adjusts by stepsize each loop 

      for(int i = 0; i < grain.numFrames; i++)
      {
	grainStart[2 * i] 	+= grain.panInfo.left * grain.buffer[i] * 0.5;
	grainStart[2 * i + 1] 	+= grain.panInfo.right * grain.buffer[i] * 0.5;
      }

      output.step += output.stepSize;						//Increases step by stepsize for next grain

    } else
    {
      global.spaceLeft 	= 0;							//Prevents loop from running once finished
    }

    if(grain.bufTest == -1)
    {
      free(grain.buffer);
      grain.bufTest 	= 0;
    }
  }

  if(global.verbose == 1){
    printf("\n -------------------------------- \n Writing output file... \n");
  }

  if(psf_sndWriteFloatFrames(							//Writes output buffer to file
	output.outputFile, 
	output.buffer, 
	output.NumFrames) != output.NumFrames)
  {
    printf("Warning: error writing %s\n", global.argv[ARG_OUTPUT]);
    return (-1);
  }

  if(global.interactive == 1)
  {
    printf(" Output File written to %s \n", initStruct.outputFile);
  }else
  {
    printf(" Output file written to %s \n", 
	global.argv[ARG_OUTPUT + optind - 1]);
  }

cleanup:
  cleanUp(									//Frees up memory buffers before programme exit
      &grain, 
      &output, 
      &global,
      &initStruct);

  return (0);
}

int setupVariables(								//Function to retrieve user input and initialise variables
    GRAIN *grain, 
    OUTPUT *output, 
    GLOBAL *global,
    INITPSF *initStruct)
{
  int c;
  while(( c = getopt_long(							//Uses getopt to parse user input 
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
    printf("\
	\n v6c13.c sound granulator works as follows:\
	\n \
	\n \tPlease supply options in order of: \
	\n \t \t INPUT(wav/aif file) \
	\n \t \t OUTPUT(wav/aif file) \
	\n \t \t DURATION(seconds) \
	\n \t \t MINIMUMGRAINLENGTH(seconds) \
	\n \t \t MAXIMUMGRAINLENGTH(seconds) \
	\n \t \t ATTACKLENGTH(percentage of grain duration) \
	\n \t \t ATTACKDECAY(percentage of grain duration) \
	\n \t \t GRAINDENSITY(number of grains per secondn)\
	\n \
	\n \t \t Example: \
	\n \t \t \t (./)v6c13.out input.wav output.wav 20 0.2 0.4 10 10 40 \
	\n \
	\n \t Also, the following options may be useful: \
	\n \t \t -h, --help \t \t Display's this dialogue \
	\n \t \t -v, --verbose \t \t Run's the programme while updating \
	the user via the command line. \
	\n \t \t -i, --interactive \t Force the programme to ignore \
	all other arguments (bar help) \
	and request input from the user to produce an output \
	\n \n");
    exit(0);
  }
  if(global->interactive)
  {
    printf("Running interactively: \n \n");					//Sequentially requests user input, validates, and passes to variables

    initStruct->type 		= T_INTERACTIVE;
    global->userInput 		= (char*) malloc(_POSIX_NAME_MAX * 
	sizeof(char));								//Allocates memory for user input using limits defined in limits.h
    global->userInputTest 	= 1;						//For every buffer there is an int: 0 for buffer not allocated

    int inputValid 		= 0;
    global->pattern 		= (char*) malloc(31 * sizeof(char));
    global->patternTest 	= 1;
    strncpy(global->pattern, 
	"^[[:alnum:]]{1,64}(.wav$|.aif$)", 
	31 * sizeof(char));							//Regular Expressions used validate user input
    while(inputValid == 0)
    {
      printf("Please enter a valid input file (less than 20 characters)\
	  e.g sample.wav OR ./path/to/sample.wav: ");
      switch(validate(global, output))
      {
	case 0:
	  sscanf(global->userInput, "%s", global->inputFile);			//Scanning input safe from buffer overflow through use of fgets and sscanf
	  inputValid = 1;
	  break;
	case 1:
	  break;
	default:
	  return (-1);
      }
    }

    inputValid = 0;
    while(inputValid == 0)
    {
      printf("\n Please enter a valid output file (less than 20 characters)\
	  e.g output.wav: ");
      switch(validate(global, output))
      {
	case 0:
	  sscanf(global->userInput, "%s", global->outputFile);
	  inputValid = 1;
	  break;

	case 1:
	  break;

	default:
	  return (-1);
      }
    }

    if(global->patternTest == 1)
    {
      free(global->pattern);
      global->patternTest = 0;
    } else
    {
      return (-1);
    }

    inputValid 		= 0;
    global->pattern 	= (char*) malloc(46 * sizeof(char));
    global->patternTest = 1;
    strncpy(global->pattern, 
	"^[[:digit:]]{1,}($|([.][[:digit:]]{1,}){0,1}$)", 
	46 * sizeof(char));
    while(inputValid == 0)
    {
      printf("\n Please enter a valid output duration e.g 20: ");
      switch(validate(global, output))
      {
	case 0:
	  sscanf(global->userInput, "%f", &output->duration);
	  inputValid = 1;
	  break;

	case 1:
	  break;

	default:
	  return (-1);
      }
    }

    inputValid = 0;
    while(inputValid == 0)
    {
      printf("\n Please enter a valid minimum grain duration e.g 0.2: ");
      switch(validate(global, output))
      {
	case 0:
	  sscanf(global->userInput, "%f", &global->minGrainDur);
	  inputValid = 1;
	  break;

	case 1:
	  break;

	default:
	  return (-1);
      }
    }

    inputValid = 0;
    while(inputValid == 0)
    {
      printf("\n Please enter a valid maximum grain duration e.g 0.4: ");
      switch(validate(global, output))
      {
	case 0:
	  sscanf(global->userInput, "%f", &global->maxGrainDur);
	  inputValid = 1;
	  break;

	case 1:
	  break;

	default:
	  return (-1);
      }
    }
    if(global->patternTest == 1)
    {
      free(global->pattern);
      global->patternTest = 0;
    }else
    {
      return (-1);
    }

    inputValid 		= 0;
    global->pattern 	= (char*) malloc(17 * sizeof(char));
    global->patternTest = 1;
    strncpy(global->pattern, "^[[:digit:]]{1,}$", 17 * sizeof(char));		//Different regex for different inputs
    while(inputValid == 0)
    {
      printf("\n Please enter a valid percentage value \
	  for the grain attack envelope e.g 10: ");
      switch(validate(global, output))
      {
	case 0:
	  sscanf(global->userInput, "%d", &global->grainAttackPercent);
	  inputValid = 1;
	  break;

	case 1:
	  break;

	default:
	  return (-1);
      }
    }

    inputValid = 0;
    while(inputValid == 0)
    {
      printf("\n Please enter a valid percentage value \
	  for the graind decay envelope e.g 10: ");
      switch(validate(global, output))
      {
	case 0:
	  sscanf(global->userInput, "%d", &global->grainDecayPercent);
	  inputValid = 1;
	  break;

	case 1:
	  break;

	default:
	  return (-1);
      }
    }

    inputValid = 0;
    while(inputValid == 0)
    {
      printf("\n Please enter a valid value \
	  for the number of grains per second e.g 25: ");
      int grainDensity;
      switch(validate(global, output))
      {
	case 0:
	  sscanf(global->userInput, "%d", &grainDensity);
	  output->grainDensity = 1.0 / grainDensity;
	  inputValid = 1;
	  break;

	case 1:
	  break;

	default:
	  return (-1);
      }
    }
    if(global->patternTest == 1)
    {
      free(global->pattern);
      global->patternTest 	= 0;
    }else
    {
      return (-1);
    }
    if(global->userInputTest == 1)
    {
      free(global->userInput);
      global->userInputTest 	= 0;
    }else
    {
      return (-1);
    }

    output->bufTest 	= 0;
    grain->bufTest 	= 0;

    printf("\n");

  } else {									//When run none interactively, variables are pulled from argv
    output->duration 		= atof(global->argv[ARG_DUR + optind - 1]);	//getopt's optind used to indicate starting value for *next* argument
    if(output->duration == 0.0)							//value offset by variables enum value and - 1 to locate correct argument
    {
      printf("Check your arguments\n"); 
      return (-1);
    }

    global->minGrainDur 	= atof(global->argv[ARG_MIN_GRAINDUR + 
	optind - 1]);
    if(global->minGrainDur == 0.0)
    {
      printf("Check your arguments\n");
      return (-1);
    }

    global->maxGrainDur 	= atof(global->argv[ARG_MAX_GRAINDUR + 
	optind - 1]);
    if(global->maxGrainDur == 0.0) 
    {
      printf("Check your arguments\n"); 
      return (-1);
    }

    global->grainAttackPercent 	= atoi(global->argv[ARG_GRAIN_ATTACK + 
	optind - 1]);
    if(global->grainAttackPercent == 0) 
    {
      printf("Check your arguments\n"); 
      return (-1);
    }

    global->grainDecayPercent 	= atoi(global->argv[ARG_GRAIN_DECAY + 
	optind - 1]);
    if(global->grainDecayPercent == 0) 
    {
      printf("Check your arguments\n"); 
      return (-1);
    }

    output->grainDensity 	= 1.0 / atof(global->argv[ARG_GRAIN_DENSITY + 
	optind - 1]);
    output->bufTest 		= 0;
    grain->bufTest		= 0;

  }

  if(global->verbose == 1)
  {
    printf("\n Running verbosely. \n \n Captain, incoming message... \n");
    sleep(2);
  }

  setOverloadPSF(								//Using a struct and set overload function to implement
      initStruct,								//overloading for psf_init().  Dependant on interactive mode
      grain, 
      output, 
      global, 
      &optind);

  if(initialisePSF(initStruct) == -1 ) return (-1);				//Init Struct itself holds pointers to global, and output structs
  output->stepSize 		= output->grainDensity * output->outprop.srate;											    

  if(global->verbose == 1)							//The verbose flag just makes the programme talk a lot
  {
    printf("\
	\n Output: \
	\n duration: \t \t \t %f \
	\n \t minGrainDur: \t \t %f \
	\n \t maxGrainDur: \t \t %f \
	\n \t grainAttackPercent: \t %d \
	\n \t grainDecayPercent: \t %d \
	\n \t grainDensity: \t \t %f \
	\n \t stepSize: \t \t %ld \
	\n",
	output->duration,
	global->minGrainDur,
	global->maxGrainDur,
	global->grainAttackPercent,
	global->grainDecayPercent,
	output->grainDensity,
	output->stepSize);
    sleep(1);
  }

  return (0);
}

int validate(GLOBAL *global, OUTPUT *output)					//Checks user input for endline character and null terminates it
{
  if(fgets(global->userInput, _POSIX_NAME_MAX * sizeof(char), stdin) != NULL)
  {
    char *endline = strchr(global->userInput,'\n');
    if(endline != NULL) *endline = '\0';
  }

  if(match(global, output) == 0)
  {
    return(0);
  }else
  {
    printf("Input not valid, try again\n");
    return(1);									//Even though failed, does not return -1 due to needing programme to continue
  }
}

int match(GLOBAL *global, OUTPUT *output)					//Computs reexec object, and checks if user input matches
{
  int status;

  if(regcomp(
	&global->re, 
	global->pattern,
	REG_EXTENDED|REG_NOSUB) != 0)
  {
    return(-1);
  }

  status = regexec(&global->re, global->userInput, 0, NULL, 0);
  regfree(&global->re);
  if(status != 0)
  {
    return(-1);
  }

  return(0);
}

int setOverloadPSF(								//function defines INITPSF struct variables based on flags
    INITPSF *initStruct, 
    GRAIN *grain, 
    OUTPUT *output, 
    GLOBAL *global, 
    int *optind)
{
  switch(initStruct->type)
  {
    case T_DEFAULT:
      initStruct->grain 	= grain;
      initStruct->output 	= output;
      initStruct->global 	= global;
      initStruct->optind 	= optind;
      break;
    case T_INTERACTIVE:
      initStruct->grain 	= grain;
      initStruct->output 	= output;
      initStruct->global 	= global;
      initStruct->inputFile 	= global->inputFile;
      initStruct->outputFile 	= global->outputFile;
      break;
  }

  return (0);
}

int initialisePSF(INITPSF *initStruct)						//accepts and INITPSF argument and reacts to it accordingly
{ 
  if(psf_init())
  {
    printf("Error: unable to open portsf\n");
    return (-1);
  }

  switch(initStruct->type)
  {
    case T_DEFAULT:

      initStruct->grain->inputFile = psf_sndOpen(
	  initStruct->global->argv[ARG_INPUT + *(initStruct->optind) - 1], 
	  &initStruct->grain->inprop, 0);
      if(initStruct->grain->inputFile < 0)
      {
	printf("Error, unable to read %s (check your file names) \n", 
	    initStruct->global->argv[ARG_INPUT + *(initStruct->optind) - 1]);
	return (-1);
      }

      initStruct->output->outprop 		= initStruct->grain->inprop;
      initStruct->output->outprop.chans 	= 2;
      initStruct->output->outprop.format 	= PSF_WAVE_EX;
      initStruct->output->outputFile 		= psf_sndCreate(
	  initStruct->global->argv[ARG_OUTPUT + *(initStruct->optind) - 1], 
	  &initStruct->output->outprop, 
	  1, 
	  0, 
	  PSF_CREATE_RDWR);

      if(initStruct->output->outputFile < 0)
      {
	printf("Error, unable to create %s (check your file names) \n", 
	    initStruct->global->argv[ARG_OUTPUT + *(initStruct->optind) - 1]);
	return (-1);
      }

      if(initStruct->global->verbose == 1)
      {
	printf("\
	    \n inprop: \t \t \t \t \t outprop: \
	    \n \t inprop.name: \t \t %s \t \t outprop.name: \t \t %s \t \
	    \n \t inprop.chans: \t \t %d \t \t \t outprop.chans: \t %d \t \
	    \n \t inprop.srate: \t \t %d \t \t \t outprop.srate: \t %d \t \
	    \n",
	    initStruct->global->argv[ARG_INPUT+ *(initStruct->optind) - 1],
	    initStruct->global->argv[ARG_OUTPUT + *(initStruct->optind) - 1],
	    initStruct->grain->inprop.chans,
	    initStruct->output->outprop.chans,
	    initStruct->grain->inprop.srate,
	    initStruct->output->outprop.srate);
	sleep(1);
      }

      initStruct->global->psfInitialized = 1;

      break;

    case T_INTERACTIVE:

      initStruct->grain->inputFile = psf_sndOpen(initStruct->inputFile, 
	  &initStruct->grain->inprop, 0); 
      if(initStruct->grain->inputFile < 0)
      {
	printf("Error, unable to read %s\n", initStruct->inputFile);
	return (-1);
      }

      initStruct->output->outprop 		= initStruct->grain->inprop;
      initStruct->output->outprop.chans 	= 2;
      initStruct->output->outprop.format 	= PSF_WAVE_EX;
      initStruct->output->outputFile 		= psf_sndCreate(
	  initStruct->outputFile, 
	  &initStruct->output->outprop, 
	  1, 
	  0, 
	  PSF_CREATE_RDWR);
      if(initStruct->output->outputFile < 0)
      {
	printf("Error, unable to create %s\n", initStruct->outputFile);
	return (-1);
      }

      if(initStruct->global->verbose == 1)
      {
	printf("\
	    \n inprop: \t \t \t \t \t outprop: \
	    \n \t inprop.name: \t \t %s \t \t outprop.name: \t \t %s \t \
	    \n \t inprop.chans: \t \t %d \t \t \t outprop.chans: \t %d \t \
	    \n \t inprop.srate: \t \t %d \t \t \t outprop.srate: \t %d \t \
	    \n",
	    initStruct->inputFile,
	    initStruct->outputFile,
	    initStruct->grain->inprop.chans,
	    initStruct->output->outprop.chans,
	    initStruct->grain->inprop.srate,
	    initStruct->output->outprop.srate);
	sleep(1);
      }

      initStruct->global->psfInitialized = 1;

      break;
  }

  return (0);
}

int allocateOutputMem(OUTPUT *output, GLOBAL *global)
{
  output->NumFrames 	= (int) output->duration * output->outprop.srate;
  output->buffer 	= (float*) calloc(1, 
      output->NumFrames * output->outprop.chans * sizeof(float));
  output->bufTest	= 1;

  if(global->verbose == 1)
  {
    printf("\t outputNumFrames \t %d \n \t outputBufferSize \t %ld \n",
	output->NumFrames,
	output->NumFrames * output->outprop.chans * sizeof(float));
    sleep(1);
  }

  return (0);
}

int allocateGrainMem(GRAIN *grain, GLOBAL *global)
{
  grain->grainDur 	= global->minGrainDur + 
    ((float) (global->maxGrainDur - global->minGrainDur) * rand()) / RAND_MAX;
  grain->numFrames 	= (int) (grain->grainDur * grain->inprop.srate);
  grain->buffer 	= (float*) malloc(grain->numFrames * 
      grain->inprop.chans * sizeof(float));
  grain->bufTest	= 1;

  if(global->verbose == 1)
  {
    printf("\
	\n Grain: \
	\n \t Grain Duration: \t %f \
	\n \t Grain numFrames: \t %d \
	\n \t Grain Buffer Size: \t %ld \
	\n",
	grain->grainDur,
	grain->numFrames,
	grain->numFrames * grain -> inprop.chans * sizeof(float));
  }

  return (0);
}

int setGrainX(GRAIN *grain, GLOBAL *global)					//Constant power pan function, returns left / right
{										//values to a pan info struct within a grain struct
  grain->panInfo.grainX = (-1.0f) + 
    (float) rand() / ((float) (RAND_MAX/(1.0-(-1.0))));
  grain->panInfo.left 	= (sqrt(2.0) / 2) * 
    (cos(grain->panInfo.grainX) - sin(grain->panInfo.grainX)) * 0.5;
  grain->panInfo.right 	= (sqrt(2.0) / 2) * 
    (cos(grain->panInfo.grainX) + sin(grain->panInfo.grainX)) * 0.5;

  if(global->verbose == 1)
  {
    printf("\t Grain X: \t \t %f \n \t \
	Grain factor left: \t %f \n \t Grain factor right: \t %f \n",
	grain->panInfo.grainX,
	grain->panInfo.left,
	grain->panInfo.right);
  }

  return (0);
}

int setupSeek(GRAIN *grain, GLOBAL *global)
{
  grain->maxSeek 	= psf_sndSize(grain->inputFile) - grain->numFrames;
  grain->seekOffset 	= ((float) grain->maxSeek * rand()) / RAND_MAX;

  if(global->verbose == 1)
  {
    printf("\t Grain maxSeek size: \t %d \n \t Grain seek offset: \t %d \n",
	grain->maxSeek,
	grain->seekOffset);
  }

  return (0);
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

  if(global->verbose == 1)
  {
    printf("\t Grain Attack Frames: \t %ld \n",
	grainAttack);
  }

  return (0);
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

  if(global->verbose == 1)
  {
    printf("\t Grain Decay Frames: \t %ld \n", grainDecay);
  }
  return (0);
}

int closePSF(INITPSF *initStruct)						//This function is also overloaded using an INITPSF struct
{
  switch(initStruct->type)
  {
    case T_DEFAULT:

      if(initStruct->grain->inputFile >= 0)
      {
	if(initStruct->global->verbose == 1) 
	{printf("\n Closing input file... \n");
	}
	if(psf_sndClose(initStruct->grain->inputFile))
	{
	  printf("Warning error closing %s\n", 
	      initStruct->global->argv[ARG_INPUT + optind - 1]);
	}
      }
      if(initStruct->output->outputFile >= 0)
      {
	if(initStruct->global->verbose == 1) 
	{
	  printf(" Closing output file... \n");
	}
	if(psf_sndClose(initStruct->output->outputFile))
	{
	  printf("Warning error closing %s\n", 
	      initStruct->global->argv[ARG_OUTPUT + optind - 1]);
	}
      }

      break;

    case T_INTERACTIVE:

      if(initStruct->grain->inputFile >= 0)
      {
	if(initStruct->global->verbose == 1) 
	{
	  printf("\n Closing input file... \n");
	}
	if(psf_sndClose(initStruct->grain->inputFile))
	{
	  printf("Warning error closing %s\n", initStruct->inputFile);
	}
      }
      if(initStruct->output->outputFile >= 0)
      {
	if(initStruct->global->verbose == 1) 
	{
	  printf(" Closing output file... \n");
	}
	if(psf_sndClose(initStruct->output->outputFile))
	{
	  printf("Warning error closing %s\n", initStruct->outputFile);
	}
      }

      break;
  }

  return (0);
}

int cleanUp(									//Checks to see which buffers are allocated and deallocates them
    GRAIN *grain, 								//checks needed to prevent deallocation errors if programme fails
    OUTPUT *output, 
    GLOBAL *global,
    INITPSF *initStruct)
{
  if(output->bufTest) 
  {
    free(output->buffer), output->bufTest 		= 0;
  }
  if(grain->bufTest) 
  {
    free(grain->buffer), grain->bufTest 		= 0;
  }
  if(global->patternTest) 
  {
    free(global->pattern), global->patternTest 		= 0;
  }
  if(global->userInputTest) 
  {
    free(global->userInput), global->userInputTest 	= 0;
  }

  if(global->psfInitialized == 1) closePSF(initStruct);

  return (0);
}
