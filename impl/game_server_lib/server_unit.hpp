
#ifndef JAMTEMPLATE_SERVER_UNIT_HPP
#define JAMTEMPLATE_SERVER_UNIT_HPP

#include <simulation_object_interface.hpp>
#include <unit_info.hpp>
#include <vector.hpp>

class ServerUnit : public SimulationObjectInterface {
public:
    // TODO think about adding box2d
    explicit ServerUnit(UnitInfo const& info);
    void update(float elapsed, WorldInfoInterface& world) override;
    ObjectProperties saveState() const override;
    void updateState(ObjectProperties const& props) override;
    void setUnitID(int unitID);

    void setPosition(jt::Vector2f const& pos);
    jt::Vector2f getPosition() const override;
    int getPlayerID() const override;
    int getUnitID() const override;
    void takeDamage(const DamageInfo& damage) override;
    bool isAlive() const override;

private:
    UnitInfo m_info;

    jt::Vector2f m_pos;
    int m_unitID { 0 };
    int m_playerID { 0 };
    float m_age { 0.0f };
    float m_hp { 100.0f };
    float m_attackTimer { 0.0f };

    mutable std::string m_newAnim { "" };
};

#endif // JAMTEMPLATE_SERVER_UNIT_HPP
