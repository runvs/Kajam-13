
#ifndef JAMTEMPLATE_UNIT_HPP
#define JAMTEMPLATE_UNIT_HPP

#include <animation.hpp>
#include <bar.hpp>
#include <counted_object.hpp>
#include <game_object.hpp>
#include <nlohmann.hpp>
#include <object_properties.hpp>
#include <shape.hpp>
#include <unit_info.hpp>
#include <vector.hpp>
#include <memory>

class Unit : public jt::GameObject {
public:
    explicit Unit(UnitInfo const& info);
    void updateState(ObjectProperties const& props);

    ObjectProperties saveState() const;
    void setPosition(jt::Vector2f const& pos);
    int getUnitID() const;
    void setIDs(int uid, int pid);
    int getPlayerID() const;

private:
    UnitInfo m_info;

    std::shared_ptr<jt::Animation> m_anim;
    float m_animTimeUntilBackToIdle = -1.0f;

    int m_unitID;
    int m_playerID { 0 };
    float m_hpMax { 100.0f };
    float m_hp { 100.0f };
    std::shared_ptr<jt::Bar> m_hpBar;

    void doCreate() override;

    void doUpdate(float const elapsed) override;

    void doDraw() const override;
};

#endif // JAMTEMPLATE_UNIT_HPP
