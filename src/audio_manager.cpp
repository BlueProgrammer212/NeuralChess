#include "audio_manager.hpp"

AudioManager::AudioManager() {
  m_sound_effects = SFXCache();
}

AudioManager::~AudioManager() {
  for (std::pair<const char*, Mix_Chunk*> audio : m_sound_effects) {
    Mix_FreeChunk(audio.second);
  }

  Mix_CloseAudio();
};

//Load the sound effects and save it in the cache.
const Mix_Chunk* AudioManager::LoadSFX(const char* name, std::string path) {
  //m_sound_effects[name] = Mix_LoadWAV(path.c_str());

  //If the WAV file does not exist. Terminate the program.
  if (m_sound_effects.at(name) == NULL) {
    std::cout << "Failed to load audio, " << path << "\n";
    SDL_Quit();
  }

  return m_sound_effects.at(name);
}

//Play the sound effect from the cache.
int AudioManager::PlayWAV(const char* src) {
  return Mix_PlayChannel(-1, Mix_LoadWAV(src), 0);
}