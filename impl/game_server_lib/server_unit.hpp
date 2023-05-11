
#ifndef JAMTEMPLATE_SERVER_UNIT_HPP
#define JAMTEMPLATE_SERVER_UNIT_HPP

#include <simulation_object_interface.hpp>
#include <vector.hpp>

class ServerUnit : public SimulationObjectInterface {
public:
    void update(float elapsed) override;
    ObjectProperties saveState() const override;

    void setPosition(jt::Vector2f const& pos);
    jt::Vector2f getPosition() const;

private:
    // TODO add playerID
    // TODO support different unit types
    jt::Vector2f m_pos;
    float m_age { 0.0f };
};

#endif // JAMTEMPLATE_SERVER_UNIT_HPP
