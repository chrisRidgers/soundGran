#ifndef V6C13
#define V6C13

enum{ARG_NAME, ARG_INPUT, ARG_OUTPUT, ARG_DUR, ARG_MIN_GRAINDUR, ARG_MAX_GRAINDUR, ARG_GRAIN_ATTACK, ARG_GRAIN_DECAY, ARG_GRAIN_DENSITY, ARGC};

enum{T_DEFAULT, T_INTERACTIVE};

typedef struct GLOBAL
{
  char **argv;
  int argc;
  int spaceLeft;
  int help;
  int interactive;
  int verbose;
  struct option long_options[4];
  int option_index;

  char inputFile[21];
  char outputFile[21];
  float minGrainDur;
  float maxGrainDur;
  int grainAttackPercent;
  int grainDecayPercent;
} GLOBAL;

typedef struct PANPOS
{
  float grainX;
  float left;
  float right;
} PANPOS;

typedef struct GRAIN
{
  int inputFile;

  float *minGrainDur;
  float *maxGrainDur;
  float grainDur;
  
  int numFrames;

  int maxSeek;
  int seekOffset;

  PANPOS panInfo;
  
  float *buffer;
  int bufTest;
  PSF_PROPS inprop;

} GRAIN;

typedef struct GRANSOUND
{
  long stepSize;
  long step;

  int outputFile;

  float duration;
  int NumFrames;
  
  float grainDensity;
  
  float *buffer;
  int bufTest;
  PSF_PROPS outprop;
} GRANSOUND;

typedef struct INITPSF
{
  int type;

  GRAIN *grain;
  GRANSOUND *output;
  GLOBAL *global;
  int *optind;

  char *inputFile;
  char *outputFile;
} INITPSF;

int setOverloadPSF(INITPSF *initStruct, GRAIN *grain, GRANSOUND *output, GLOBAL *global, int *optind);
int closePSF(GRAIN *grain, GRANSOUND *output, GLOBAL *global);
int setupVariables(GRAIN *grain, GRANSOUND *output, GLOBAL *global, INITPSF *initStruct);
int allocateGrainMem(GRAIN *grain, GLOBAL *global);
int allocateOutputMem(GRANSOUND *output);
int setGrainX(GRAIN *grain);
int setupSeek(GRAIN *grain);
int impAttackEnv(GRAIN *grain, GLOBAL *global);
int impDecayEnv(GRAIN *grain, GLOBAL *global);

int cleanUp(GRAIN *grain, GRANSOUND *output, GLOBAL *global);

int initialisePSF(INITPSF *initStruct);



#endif
