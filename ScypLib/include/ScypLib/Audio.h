#pragma once
#include<unordered_map>
#include<memory>
#include<string>

#include<miniaudio/miniaudio.h>
#undef PlaySound

namespace sl
{
    class Sound
    {
    public:
        Sound(ma_engine* engine, const std::string& filepath);
        ~Sound();
    private:
        friend class Audio;

        ma_sound sound;
        ma_decoder decoder;
    };

    class Audio
    {
    public:
        Audio();
        ~Audio();
        Sound* LoadSound(const std::string& filepath);
        void UnloadSound(Sound* sound);
        void PlaySound(Sound* sound);
        void StopSound(Sound* sound);
        void ClearSounds();
    private:
        ma_engine soundEngine;
        std::unordered_map<std::string, std::unique_ptr<Sound>> sounds;
    };
}