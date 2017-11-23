#include "sound.h"
#include "io.h"
#include "stb_vorbis.c"
#include <stdlib.h>
#include <stdio.h>

ex_sound_t *ex_sound;

void ex_sound_init()
{
  ex_sound = malloc(sizeof(ex_sound_t));

  // init device
  ex_sound->device = alcOpenDevice(NULL);
  if (!ex_sound->device) {
    printf("Failed opening OpenAL device\n");
    goto cleanup;
  }
  
  // init and set context
  ex_sound->context = alcCreateContext(ex_sound->device, NULL);
  if (!alcMakeContextCurrent(ex_sound->context)) {
    printf("Failed creating OpenAL context current\n");
    goto cleanup;
  }

  // setup listener properties
  ALfloat pos[] = {0.0, 0.0, 1.0};
  ALfloat vel[] = {0.0, 0.0, 0.0};
  ALfloat ori[] = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0};
  alListenerfv(AL_POSITION, pos);
  alListenerfv(AL_VELOCITY, vel);
  alListenerfv(AL_ORIENTATION, ori);
  alListenerf(AL_GAIN, 1.0);

  return;

cleanup:
  free(ex_sound);
  ex_sound = NULL;
}

ex_source_t* ex_sound_load_source(const char *path, ex_sound_e format, int loop)
{
  printf("Loading audio file %s\n", path);
  int channels, rate;
  short *data = NULL;
  int32_t len = 0;

  // decode ogg data
  if (format == EX_SOUND_OGG) {
    printf("Decoding ogg format\n");
    len = stb_vorbis_decode_filename(path, &channels, &rate, &data);
    
    // loading failed
    if (len <= 0) {
      printf("Failed decoding ogg file %s\n", path);
      return NULL;
    }

  }

  // init the al source
  ex_source_t *s = malloc(sizeof(ex_source_t));
  alGenSources(1, &s->id);

  // set default source values
  alSourcef(s->id, AL_PITCH, 1);
  alSourcef(s->id, AL_GAIN, 1);
  alSource3f(s->id, AL_POSITION, 0, 0, 0);
  alSource3f(s->id, AL_VELOCITY, 0, 0, 0);
  alSourcei(s->id, AL_LOOPING, loop);

  // buffer
  uint32_t length = len * channels * (sizeof(int16_t) / sizeof(uint8_t));
  alGenBuffers(1, &s->buffer);
  alBufferData(s->buffer, AL_FORMAT_STEREO16, data, length, rate);

  // bind buffer to source
  alSourcei(s->id, AL_BUFFER, s->buffer);

  free(data);

  return s;
}

void ex_sound_destroy(ex_source_t *s)
{
  alDeleteSources(1, &s->id);
  alDeleteBuffers(1, &s->buffer);
  free(s);
  s = NULL;
}

void ex_sound_exit()
{
  alcMakeContextCurrent(NULL);
  alcDestroyContext(ex_sound->context);
  alcCloseDevice(ex_sound->device);
  free(ex_sound);
  ex_sound = NULL;
}