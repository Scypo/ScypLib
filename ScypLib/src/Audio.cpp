#define MINIAUDIO_IMPLEMENTATION
#include<cassert>
#include <stdexcept>
#include"ScypLib/Audio.h"
#include<miniaudio/miniaudio.h>
#undef PlaySound

namespace sl
{
    struct Sound::InternalSound
    {
        ~InternalSound()
        {
            ma_sound_uninit(&sound);
            ma_decoder_uninit(&decoder);
        }
        ma_sound sound;
        ma_decoder decoder;
    };
    struct Audio::InternalSoundEngine
    {
        ma_engine soundEngine;
    };
    Audio::Audio()
    {
        internalSoundEngine = std::make_unique<InternalSoundEngine>();
        if (ma_engine_init(NULL, &internalSoundEngine->soundEngine) != MA_SUCCESS)
        {
            throw std::runtime_error("Failed to initialize audio engine.");
        }
    }

    Audio::~Audio()
    {
        ClearSounds();
        ma_engine_uninit(&internalSoundEngine->soundEngine);
    }

    Sound* Audio::LoadSound(const std::string& filepath)
    {
        if (!sounds.contains(filepath))
        {
            std::unique_ptr<Sound::InternalSound> sound = std::make_unique<Sound::InternalSound>();
            if (ma_decoder_init_file(filepath.c_str(), nullptr, &sound->decoder) != MA_SUCCESS)
            {
                throw std::runtime_error(("Failed to init decoder: " + filepath).c_str());
            }
            if (ma_sound_init_from_data_source(&internalSoundEngine->soundEngine, &sound->decoder, 0, nullptr, &sound->sound) != MA_SUCCESS)
            {
                ma_decoder_uninit(&sound->decoder);
                throw std::runtime_error(("Failed to init sound: " + filepath).c_str());
            }
            sounds[filepath] = std::unique_ptr<Sound>(new Sound(std::move(sound)));
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
        ma_sound_start(&sound->sound->sound);
    }

    void Audio::StopSound(Sound* sound)
    {
        assert(sound && "Failed to stop sound. Sound is nullptr");
        ma_sound_stop(&sound->sound->sound);
    }

    Sound::Sound(std::unique_ptr<InternalSound>&& sound)
        : sound(std::move(sound)) {}
}
