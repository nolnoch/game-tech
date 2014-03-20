/*
 * SoundManager.cpp
 *
 *  Created on: Mar 7, 2014
 *      Author: nolnoch
 */

#include "SoundManager.h"

std::vector<Sound> SoundManager::activeSounds(0);
void channelEnd(int);
// Registers the callback
void channelEnd(int channel) {
  std::cout << " callback! " << std::endl;
  SoundManager manager;
  manager.channelDone(channel);
}

SoundManager::SoundManager():
sounding(true),
initialized(false),
musicPlaying(false),
music(0)
{
}




SoundManager::~SoundManager() {
  mute();


  Mix_FreeMusic(music);
    // Register a callback for when a sound stops playing.


  std::vector<Mix_Chunk *>::iterator it;
  for (it = chunks.begin(); it != chunks.end(); it++) {
    Mix_FreeChunk(*it);
  }
  chunks.clear();
}

void SoundManager::channelDone(int channel) {
  for(int i = 0; i < activeSounds.size(); i++) {
    Sound *s = &(activeSounds[i]);
    if(s->channel == channel) {
      std::cout << " channel " << channel << " array size " << activeSounds.size() <<  std::endl;
    }
  }
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

bool SoundManager::loadMusic(const char *name) {
  if (!initialized) {
    std::cout << "SoundManager: Manager not initialized." << std::endl;
    return false;
  }

  music = Mix_LoadMUS(name);

  return music;
}

int SoundManager::loadSound(const char *name) {
  if (!initialized) {
    std::cout << "SoundManager: Manager not initialized." << std::endl;
    return false;
  }

  int idx;
  Mix_Chunk *chunk;

  chunk = Mix_LoadWAV(name);
  if (!chunk) {
    std::cout << "SDL_mixer: Unable to load chunk sound file." << std::endl;
    idx = -1;
  } else {
    idx = chunks.size();
    chunks.push_back(chunk);
  }

  return idx;
}

int SoundManager::getVolume() {
  return Mix_Volume(-1, -1);
}

void SoundManager::setVolume(double vol) {
  if (!initialized) {
    std::cout << "SoundManager: Manager not initialized." << std::endl;
    return;
  }
  int volume = MIX_MAX_VOLUME;
  if (((int) vol) < volume)
    volume = (int) (vol * MIX_MAX_VOLUME);

  Mix_Volume(-1, volume);
  Mix_Volume(0, volume);
  Mix_Volume(1, volume);
  Mix_VolumeMusic(volume);
}

void SoundManager::lowerVolume()
{
    double volume = getVolume();
    volume -= 8;
    if(volume < 0)
      volume = 0;
    setVolume(volume / MIX_MAX_VOLUME);
}

void SoundManager::raiseVolume()
{
    double volume = getVolume();
    volume += 8;
    if(volume > MIX_MAX_VOLUME)
      volume = MIX_MAX_VOLUME;
    setVolume(volume / MIX_MAX_VOLUME);
}

void SoundManager::playMusic() {
  if (!initialized) {
    std::cout << "SoundManager: Manager not initialized." << std::endl;
    return;
  }

  if (sounding) {
    musicPlaying = true;
    Mix_PlayMusic(music, -1);
  }
}

void SoundManager::playSound(int chunkIdx) {
  if (!initialized) {
    std::cout << "SoundManager: Manager not initialized." << std::endl;
    return;
  }

  if (sounding)
    Mix_PlayChannel(-1, chunks[chunkIdx], 0);
}


void SoundManager::playSound(int chunkIdx, Ogre::Vector3 sound, Ogre::Vector3 camera) {
  if (!initialized) {
    std::cout << "SoundManager: Manager not initialized." << std::endl;
    return;
  }



  if (sounding) {
    Mix_ChannelFinished(channelEnd);
    int channel = Mix_PlayChannel(-1, chunks[chunkIdx], 0);
    Ogre::Real distance = sound.distance(camera);
    if(distance > 1500)
      distance = 1500;
    int dist = distance/1500 * 255; //10000 should be the max range.
    std::cout << "Distance " << distance << " " << dist << " channel " << channel <<  std::endl;

    // put this sound in our list of active sounds.
    Sound s;
    s.soundPosition = sound;
    s.chunk = chunks[chunkIdx];
    s.distance = dist;
    s.channel = channel;
    activeSounds.push_back(s);
    // set the position of this sound to be at this angle and distance.
    Mix_SetPosition(channel, 0, dist);
  }
}




void SoundManager::pauseMusic() {
  if (!initialized || !musicPlaying) {
    std::cout << "SoundManager: No music playing." << std::endl;
    return;
  }

  musicPlaying = false;

  Mix_HaltMusic();
}

void SoundManager::mute() {
  sounding = false;

  if (initialized)
    pauseMusic();
}

void SoundManager::unmute() {
  sounding = true;

  if (initialized)
    playMusic();
}

void SoundManager::toggleSound() {
  sounding ? mute() : unmute();
}

void SoundManager::updateSounds(Ogre::Vector3 camPosition) {
  for(int i = 0; i < activeSounds.size(); i++) {
    Sound *s = &(activeSounds[i]);
    Ogre::Real distance = camPosition.distance(s->soundPosition); //get the distance between sound and camera
    if(distance > 1500)
      distance = 1500;
    s->distance = distance;
    int dist = distance/1500 * 255; //1500 should be the max range.
    Mix_SetPosition(s->channel, 0, dist);
  }
}





/*
0 = directly in front.
90 = directly to the right.
180 = directly behind.
270 = directly to the left.
*/
