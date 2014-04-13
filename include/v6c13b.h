#ifndef V6C13
#define V6C13

enum{ARG_NAME, ARG_INPUT, ARG_OUTPUT, ARG_DUR, ARG_MIN_GRAINDUR, ARG_MAX_GRAINDUR, ARG_GRAIN_ATTACK, ARG_GRAIN_DECAY, ARG_GRAIN_DENSITY, ARGC};

typedef struct GLOBAL
{
  int spaceLeft;
  char **argv;
  float minGrainDur;
  float maxGrainDur;
  int grainAttackPercent;
  int grainDecayPercent;
} GLOBAL;


typedef struct GRAIN
{
  int inputFile;

  float *minGrainDur;
  float *maxGrainDur;
  float grainDur;
  
  int numFrames;

  int maxSeek;
  int seekOffset;

  float grainX;
  
  float *buffer;
  PSF_PROPS *inprop;

} GRAIN;

typedef struct GRANSOUND
{
  long stepSize;
  long step;

  int outputFile;

  float duration;
  int outputNumFrames;
  
  float grainDensity;
  
  float *buffer;
  PSF_PROPS *outprop;
} GRANSOUND;

/*
int initialisePSF(int *infile, int *outfile, PSF_PROPS *inprop, PSF_PROPS *outprop, char **argv);
int closePSF(int *infile, int *outfile, char **argv);
int setupVariables(float *duration, float *minGrainDur, float *maxGrainDur, int *grainAttackPercent, int *grainDecayPercent, float *grainDensity, long *stepSize, PSF_PROPS *inprop, PSF_PROPS *outprop, char **argv);
int allocateGrainMem(float *grainDur, float *minGrainDur, float *maxGrainDur, int *numFrames, float **grain, PSF_PROPS *inprop);
int allocateOutputMem(float *duration, int *outputNumFrames, float **output, PSF_PROPS *outprop);
int setGrainX(float *grainX);
int setupSeek(int *infile, int *numFrames, int *maxSeek, int *seekOffset);
int impAttackEnv(int *grainAttackPercent, int *numFrames, float *grain);
int impDecayEnv(int *grainDecayPercent, int *numFrames, float *grain);
*/

int initialisePSF(GRAIN *grain, GRANSOUND *output, GLOBAL *global);
int setupVariables(GRAIN *grain, GRANSOUND *output, GLOBAL *global);
int allocateGrainMem(GRAIN *grain, GLOBAL *global);
int allocateOutputMem(GRANSOUND *output);
int setGrainX(GRAIN *grain);
int setupSeek(GRAIN *grain);
int impAttackEnv(GRAIN *grain, GLOBAL *global);
int impDecayEnv(GRAIN *grain, GLOBAL *global);

int cleanUp(GRAIN *grain, GRANSOUND *output);



#endif
