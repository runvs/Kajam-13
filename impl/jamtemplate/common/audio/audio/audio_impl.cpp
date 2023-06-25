#include "audio_impl.hpp"
#include <audio/sound/intro_looping_sound_with_effect.hpp>
#include <audio/sound/sound.hpp>
#include <audio/sound/sound_group.hpp>
#include <audio/sound/sound_with_effect.hpp>
#include <random/random.hpp>
#include <algorithm>

jt::AudioImpl::AudioImpl(std::unique_ptr<SoundFadeManagerInterface> soundFadeManager)
    : m_fades { std::move(soundFadeManager) }
{
    if (!m_fades) {
        m_fades = std::make_unique<SoundFadeManager>();
    }
}

jt::AudioImpl::~AudioImpl() { m_temporarySounds.clear(); }

void jt::AudioImpl::update(float elapsed)
{
    cleanUpUnusedSounds();

    for (auto const& snd : m_temporarySounds) {
        auto sound = snd.lock();
        if (sound) {
            sound->update();
        }
    }
    m_fades->update(elapsed);
    for (auto& snd : m_permanentSounds) {
        snd.second->update();
    }
}

void jt::AudioImpl::cleanUpUnusedSounds()
{
    (void)std::erase_if(m_temporarySounds, [](auto const& wptr) { return wptr.expired(); });
}

std::shared_ptr<jt::SoundInterface> jt::AudioImpl::addTemporarySound(std::string const& fileName)
{
    auto const sound = std::make_shared<jt::Sound>(fileName);
    sound->setVolumeProvider(m_volumeGroups);
    m_temporarySounds.push_back(sound);
    return sound;
}

oalpp::SoundContextInterface& jt::AudioImpl::getContext() { return m_context; }

std::shared_ptr<jt::SoundInterface> jt::AudioImpl::getPermanentSound(std::string const& identifier)
{
    if (m_permanentSounds.count(identifier) == 0) {
        return nullptr;
    }
    return m_permanentSounds[identifier];
}

void jt::AudioImpl::removePermanentSound(std::string const& identifier)
{
    m_permanentSounds.erase(identifier);
}

std::shared_ptr<jt::SoundInterface> jt::AudioImpl::soundPool(
    std::string const& baseIdentifier, std::string const& fileName, std::size_t count)
{
    auto const randomNumber = jt::Random::getInt(0, static_cast<int>(count));
    auto const soundName = baseIdentifier + "####" + std::to_string(randomNumber);

    auto snd = getPermanentSound(soundName);
    if (snd == nullptr) {
        snd = addPermanentSound(soundName, fileName);
    }
    return snd;
}

std::shared_ptr<jt::SoundInterface> jt::AudioImpl::addPermanentSound(
    std::string const& identifier, std::string const& fileName)
{
    auto const sound = std::make_shared<jt::Sound>(fileName);
    sound->setVolumeProvider(m_volumeGroups);
    m_permanentSounds[identifier] = sound;
    return sound;
}

std::shared_ptr<jt::SoundInterface> jt::AudioImpl::addPermanentSound(std::string const& identifier,
    std::string const& fileName, oalpp::effects::MonoEffectInterface& effect)
{
    auto const sound = std::make_shared<jt::SoundWithEffect>(fileName, effect);
    sound->setVolumeProvider(m_volumeGroups);
    m_permanentSounds[identifier] = sound;
    return sound;
}

std::shared_ptr<jt::SoundInterface> jt::AudioImpl::addPermanentSound(std::string const& identifier,
    std::string const& introFileName, std::string const& loopingFileName,
    oalpp::effects::MonoEffectInterface& effect)
{
    auto const sound
        = std::make_shared<jt::IntroLoopingSoundWithEffect>(introFileName, loopingFileName, effect);
    sound->setVolumeProvider(m_volumeGroups);
    m_permanentSounds[identifier] = sound;
    return sound;
}

std::shared_ptr<jt::SoundInterface> jt::AudioImpl::addTemporarySoundGroup(
    std::vector<std::shared_ptr<jt::SoundInterface>> const& sounds)
{
    auto const group = std::make_shared<jt::SoundGroup>(sounds);
    group->setVolumeProvider(m_volumeGroups);
    m_temporarySounds.push_back(group);
    return group;
}

jt::SoundFadeManagerInterface& jt::AudioImpl::fades() { return *m_fades; }
jt::GroupVolumeSetterInterface& jt::AudioImpl::groups() { return m_volumeGroups; }
