/*
 * SoundManager.cpp
 *
 *  Created on: Mar 7, 2014
 *      Author: nolnoch
 */

#include "SoundManager.h"

SoundManager::SoundManager():
sounding(false),
initialized(false),
music(0)
{
}

SoundManager::~SoundManager() {
  delete chunks;
}

bool SoundManager::initSoundManager() {
  // Initialize Audio [based on http://www.kekkai.org/roger/sdl/mixer/]
    /* We're going to be requesting certain things from our audio
             device, so we set them up beforehand */
    int audio_rate = 22050;
    Uint16 audio_format = AUDIO_S16; /* 16-bit stereo */
    int audio_channels = 2;
    int audio_buffers = 4096;

    /* This is where we open up our audio device.  Mix_OpenAudio takes
             as its parameters the audio format we'd /like/ to have. */
    if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers))
      std::cout << "Unable to open audio!\n" << std::endl;
    else
      initialized = true;

    return initialized;
}



