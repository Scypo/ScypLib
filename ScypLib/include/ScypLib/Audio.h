#pragma once
#include<unordered_map>
#include<memory>
#include<string>

namespace sl
{
    class Sound
    {
    public:
        ~Sound() = default;
    private:
        struct InternalSound;
        Sound(std::unique_ptr<InternalSound>&& sound);
    private:
        friend class Audio;
        std::unique_ptr<InternalSound> sound = nullptr;
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
        struct InternalSoundEngine;
        std::unique_ptr<InternalSoundEngine> internalSoundEngine = nullptr;
        std::unordered_map<std::string, std::unique_ptr<Sound>> sounds;
    };
}