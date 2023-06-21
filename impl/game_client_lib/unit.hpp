
#ifndef JAMTEMPLATE_UNIT_HPP
#define JAMTEMPLATE_UNIT_HPP

#include <animation.hpp>
#include <bar.hpp>
#include <counted_object.hpp>
#include <game_object.hpp>
#include <network_data/unit_server_to_client_data.hpp>
#include <nlohmann.hpp>
#include <object_properties.hpp>
#include <shape.hpp>
#include <text.hpp>
#include <unit_info.hpp>
#include <vector.hpp>
#include <memory>

class Unit : public jt::GameObject {
public:
    explicit Unit(UnitInfo const& info);

    void updateState(UnitServerToClientData const& props);

    void setPosition(jt::Vector2f const& pos);
    jt::Vector2f getPosition() const;

    void setOffset(jt::Vector2f const& offset);
    jt::Vector2f getOffset() const;

    int getUnitID() const;
    int getPlayerID() const;
    void setIDs(int uid, int pid);

    int getLevel() const;

    bool isUnitAlive() const;

    bool isMouseOver() const;

    UnitInfo const& getInfo() const;

    void resetForNewRound();

private:
    UnitInfo m_info;

    jt::Vector2f m_position;
    jt::Vector2f m_offset;

    std::shared_ptr<jt::Animation> m_anim;
    std::string m_newAnimName { "" };

    std::shared_ptr<jt::Text> m_levelText;
    float m_animTimeUntilBackToIdle = -1.0f;

    int m_unitID;
    int m_playerID { 0 };
    float m_hpMax { 100.0f };
    float m_hp { 100.0f };
    int m_level { 1 };
    std::vector<std::string> m_boughtUpgrades;
    std::shared_ptr<jt::Bar> m_hpBar;

    void doCreate() override;

    void doUpdate(float const elapsed) override;

    void doDraw() const override;
    void playAnimation();
};

#endif // JAMTEMPLATE_UNIT_HPP
