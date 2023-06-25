#include "sound_fade_manager.hpp"
#include <algorithm>

namespace jt {

void SoundFadeManager::volumeFade(std::weak_ptr<SoundInterface> sound, float durationInSeconds,
    float startVolume, float endVolume)
{
    m_fadeInfos.emplace_back(SoundFadeInfo { sound, durationInSeconds, startVolume, endVolume });
}

void SoundFadeManager::update(float elapsed)
{
    (void)std::erase_if(
        m_fadeInfos, [](auto const& fadeInfo) { return !fadeInfo.hasValidSound(); });

    for (auto& fade : m_fadeInfos) {
        fade.update(elapsed);
    }

    (void)std::erase_if(m_fadeInfos, [](auto const& fadeInfo) { return !fadeInfo.isAlive(); });
}
std::size_t SoundFadeManager::size() const { return m_fadeInfos.size(); }

} // namespace jt
