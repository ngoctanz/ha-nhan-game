#include "game/SoundEffects.hpp"
#include "game/AssetLocator.hpp"

#include <string>

namespace game
{
namespace
{
Sound LoadMp3Effect(const char *relativePath, float volume)
{
    Sound sound = {};
    const std::string path = ResolveAssetPath(relativePath);
    int dataSize = 0;
    unsigned char *data = LoadFileData(path.c_str(), &dataSize);
    if (data == nullptr || dataSize <= 0) return sound;

    Wave wave = LoadWaveFromMemory(".mp3", data, dataSize);
    if (IsWaveValid(wave))
    {
        sound = LoadSoundFromWave(wave);
        UnloadWave(wave);
        if (IsSoundValid(sound)) SetSoundVolume(sound, volume);
    }
    UnloadFileData(data);
    return sound;
}

Sound LoadMp3EffectCropped(const char *relativePath, float volume, float maxSeconds)
{
    Sound sound = {};
    const std::string path = ResolveAssetPath(relativePath);
    int dataSize = 0;
    unsigned char *data = LoadFileData(path.c_str(), &dataSize);
    if (data == nullptr || dataSize <= 0) return sound;

    Wave wave = LoadWaveFromMemory(".mp3", data, dataSize);
    if (IsWaveValid(wave))
    {
        // Crop to maxSeconds to prevent the meme from playing forever.
        const unsigned int maxSamples =
            static_cast<unsigned int>(maxSeconds * static_cast<float>(wave.sampleRate));
        if (wave.frameCount > maxSamples)
            WaveCrop(&wave, 0, static_cast<int>(maxSamples));

        sound = LoadSoundFromWave(wave);
        UnloadWave(wave);
        if (IsSoundValid(sound)) SetSoundVolume(sound, volume);
    }
    UnloadFileData(data);
    return sound;
}

void UnloadEffect(Sound &sound)
{
    if (IsSoundValid(sound)) UnloadSound(sound);
    sound = {};
}

void RestartEffect(Sound sound)
{
    if (!IsSoundValid(sound)) return;
    if (IsSoundPlaying(sound)) StopSound(sound);
    PlaySound(sound);
}
} // namespace

MemeSound ParseMemeSound(std::string_view name)
{
    if (name == "huh") return MemeSound::Huh;
    if (name == "doit") return MemeSound::DoIt;
    if (name == "cao_ni_ma") return MemeSound::CaoNiMa;
    return MemeSound::None;
}

SoundEffects::~SoundEffects()
{
    UnloadEffect(huh_);
    UnloadEffect(doIt_);
    UnloadEffect(caoNiMa_);
}

bool SoundEffects::Load()
{
    if (!IsAudioDeviceReady()) return false;
    huh_ = LoadMp3Effect("assets/sounds/huh_37bAoRo.mp3", 0.76F);
    doIt_ = LoadMp3Effect("assets/sounds/boi-doit-do-it.mp3", 0.72F);
    caoNiMa_ = LoadMp3EffectCropped("assets/sounds/cao-ni-ma-meme.mp3", 0.68F, 3.0F);
    return IsSoundValid(huh_) && IsSoundValid(doIt_) && IsSoundValid(caoNiMa_);
}

void SoundEffects::Play(MemeSound sound)
{
    switch (sound)
    {
        case MemeSound::Huh: RestartEffect(huh_); break;
        case MemeSound::DoIt: RestartEffect(doIt_); break;
        case MemeSound::CaoNiMa: RestartEffect(caoNiMa_); break;
        case MemeSound::None: break;
    }
}
} // namespace game
