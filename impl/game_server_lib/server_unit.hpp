
#ifndef JAMTEMPLATE_SERVER_UNIT_HPP
#define JAMTEMPLATE_SERVER_UNIT_HPP

#include <simulation_object_interface.hpp>
#include <vector.hpp>

class ServerUnit : public SimulationObjectInterface {
public:
    void update(float elapsed, WorldInfoInterface& world) override;
    ObjectProperties saveState() const override;
    void updateState(ObjectProperties const& props);
    void setUnitID(int unitID);

    void setPosition(jt::Vector2f const& pos);
    jt::Vector2f getPosition() const override;
    int getPlayerID() const override;

private:
    // TODO add playerID
    // TODO support different unit types
    jt::Vector2f m_pos;
    int m_unitID { 0 };
    int m_playerID { 0 };
    float m_age { 0.0f };
    float m_hp { 100.0f };
};

#endif // JAMTEMPLATE_SERVER_UNIT_HPP
