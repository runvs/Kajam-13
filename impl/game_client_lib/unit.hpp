
#ifndef JAMTEMPLATE_UNIT_HPP
#define JAMTEMPLATE_UNIT_HPP

#include "audio/sound/sound_group.hpp"
#include "audio/sound/sound_interface.hpp"
#include <animation.hpp>
#include <bar.hpp>
#include <counted_object.hpp>
#include <game_object.hpp>
#include <network_data/unit_info.hpp>
#include <network_data/unit_server_to_client_data.hpp>
#include <nlohmann.hpp>
#include <object_properties.hpp>
#include <shape.hpp>
#include <text.hpp>
#include <unit_interface.hpp>
#include <vector2.hpp>
#include <memory>
#include <string>
#include <vector>

class Unit : public jt::GameObject, public UnitInterface {
public:
    explicit Unit(UnitInfo const& info);

    void updateState(UnitServerToClientData const& data);

    void setPosition(jt::Vector2f const& pos);
    jt::Vector2f getPosition() const;

    void setOffset(jt::Vector2f const& offset);
    jt::Vector2f getOffset() const;

    int getUnitID() const;
    int getPlayerID() const override;
    void setIDs(int uid, int pid);

    int getLevel() const override;
    int getExp() const override;
    int getExpForLevelUp() const override;
    bool isUnitAlive() const;

    bool isMouseOver() const override;

    UnitInfo const& getInfo() const override;

    void resetForNewRound();
    void flash() override;

    void setHighlight(bool v) override;

private:
    UnitInfo m_info;

    jt::Vector2f m_position;
    jt::Vector2f m_offset;

    std::shared_ptr<jt::Animation> m_anim;
    std::string m_newAnimName { "" };
    bool m_glowActive { false };
    std::shared_ptr<jt::Sprite> m_glow;

    std::shared_ptr<jt::Text> m_levelText;
    std::shared_ptr<jt::SoundInterface> m_sfxGrpCloseCombat { nullptr };
    std::shared_ptr<jt::SoundInterface> m_sfxCloseCombat1 { nullptr };
    std::shared_ptr<jt::SoundInterface> m_sfxCloseCombat2 { nullptr };
    std::shared_ptr<jt::SoundInterface> m_sfxCloseCombat3 { nullptr };

    std::shared_ptr<jt::SoundInterface> m_sfxArcher { nullptr };
    std::shared_ptr<jt::SoundInterface> m_sfxCrossbow { nullptr };

    std::vector<std::pair<float, std::shared_ptr<jt::SoundInterface>>> m_soundsToPlay;
    float m_animTimeUntilBackToIdle = -1.0f;

    int m_unitID;
    int m_playerID { 0 };
    float m_hpMax { 1.0f };
    float m_hp { 1.0f };
    int m_level { 1 };
    int m_exp { 0 };
    std::vector<std::string> m_boughtUpgrades;
    std::shared_ptr<jt::Bar> m_hpBar;
    float m_slope { 0.0f };

    void doCreate() override;
    void doUpdate(float const elapsed) override;
    void doDraw() const override;

    void playAnimation();
};

#endif // JAMTEMPLATE_UNIT_HPP
