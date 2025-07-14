#define MINIAUDIO_IMPLEMENTATION
#include<cassert>
#include <stdexcept>
#include"ScypLib/Audio.h"

namespace sl
{
    Audio::Audio()
    {
        if (ma_engine_init(NULL, &soundEngine) != MA_SUCCESS)
        {
            throw std::runtime_error("Failed to initialize audio engine.");
        }
    }

    Audio::~Audio()
    {
        ClearSounds();
        ma_engine_uninit(&soundEngine);
    }

    Sound* Audio::LoadSound(const std::string& filepath)
    {
        if (!sounds.contains(filepath))
        {
            sounds[filepath] = std::make_unique<Sound>(&soundEngine, filepath);
        }
        return sounds[filepath].get();
    }

    void Audio::UnloadSound(Sound* sound)
    {
        assert(sound && "Failed to unload sound. Sound is nullptr");
        for (auto it = sounds.begin(); it != sounds.end(); ++it)
        {
            if (it->second.get() == sound)
            {
                sounds.erase(it);
                break;
            }
        }
    }

    void Audio::ClearSounds()
    {
        sounds.clear();
    }

    void Audio::PlaySound(Sound* sound)
    {
        assert(sound && "Failed to play sound. Sound is nullptr");
        ma_sound_start(&sound->sound);
    }

    void Audio::StopSound(Sound* sound)
    {
        assert(sound && "Failed to stop sound. Sound is nullptr");
        ma_sound_stop(&sound->sound);
    }

    Sound::Sound(ma_engine* engine, const std::string& filepath)
    {
        if (ma_decoder_init_file(filepath.c_str(), nullptr, &decoder) != MA_SUCCESS)
        {
            throw std::runtime_error(("Failed to init decoder: " + filepath).c_str());
        }

        if (ma_sound_init_from_data_source(engine, &decoder, 0, nullptr, &sound) != MA_SUCCESS)
        {
            throw std::runtime_error(("Failed to init sound: " + filepath).c_str());
            ma_decoder_uninit(&decoder);
            return;
        }
    }

    Sound::~Sound()
    {
        ma_sound_uninit(&sound);
        ma_decoder_uninit(&decoder);
    }
}
