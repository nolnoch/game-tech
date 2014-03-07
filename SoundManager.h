/*
 * SoundManager.h
 *
 *  Created on: Mar 7, 2014
 *      Author: nolnoch
 */

#ifndef SOUNDMANAGER_H_
#define SOUNDMANAGER_H_


#include <vector>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>


class SoundManager {
public:
  SoundManager();
  virtual ~SoundManager();

  bool initSoundManager();
  bool loadMusic(char name[]);
  bool loadSound(char name[]);
  void setVolume(int vol);
  void playMusic();
  void playSound(int chunk);
  void pauseMusic();
  void pauseSound();
  void mute();

private:
  bool sounding, initialized;
  Mix_Music *music;
  std::vector<Mix_Chunk *> chunks;
};

#endif /* SOUNDMANAGER_H_ */
