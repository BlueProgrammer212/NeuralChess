#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <unordered_map>

using SFXCache = std::unordered_map<const char*, Mix_Chunk*>;

//Configure the audio settings.
constexpr int FREQUENCY = 22050;
constexpr int CHUNK_SIZE = 4096;

class AudioManager {
 public:
  AudioManager();

  ~AudioManager();

  //Load the sound effects from a path then
  //store it to the cache for optimization.
  const Mix_Chunk* LoadSFX(const char* name, std::string path);

  //Play the audio from the cache.
  int PlayWAV(const char* name);

  //Invoke the destructor.
  void destroy() { delete this; }

 private:
  //Sound effect cache
  SFXCache m_sound_effects;
};