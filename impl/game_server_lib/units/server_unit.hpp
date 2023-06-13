
#ifndef JAMTEMPLATE_SERVER_UNIT_HPP
#define JAMTEMPLATE_SERVER_UNIT_HPP

#include "object_properties.hpp"
#include "simulation_object_interface.hpp"
#include "unit_info.hpp"
#include "units/ai/ai_interface.hpp"
#include "vector.hpp"
#include <box2dwrapper/box2d_object.hpp>
#include <log/logger_interface.hpp>
#include <memory>

class ServerUnit : public SimulationObjectInterface {
public:
    explicit ServerUnit(jt::LoggerInterface& logger, const UnitInfo& info,
        std::shared_ptr<jt::Box2DWorldInterface> world);

    void update(float elapsed, WorldInfoInterface& world) override;

    ObjectProperties saveState() const override;

    void updateState(ObjectProperties* props) override;

    void levelUnitUp() override;

    void setUnitID(int unitID);

    void setPosition(jt::Vector2f const& pos);

    jt::Vector2f getPosition() const override;

    void setOffset(jt::Vector2f const& offset);

    jt::Vector2f getOffset() const override;

    int getPlayerID() const override;

    int getUnitID() const override;

    void takeDamage(const DamageInfo& damage) override;

    bool isAlive() const override;

    // base values
    UnitInfo const& getUnitInfoBase() const;

    // with upgrades
    UnitInfo getUnitInfoFull() const;

    std::shared_ptr<jt::Box2DObject> getPhysicsObject();

    int getCost() override;

    void gainExperience(int exp) override;
    int getLevel() const override;

private:
    jt::LoggerInterface& m_logger;
    std::shared_ptr<jt::Box2DObject> m_physicsObject { nullptr };

    ObjectProperties* m_roundStartObjectProperties { nullptr };

    // basic values
    UnitInfo m_infoBase;
    // leveled Values
    UnitInfo m_infoLevel;

    // from upgrades
    UnitInfo m_infoUpgrades;
    std::unique_ptr<AiInterface> m_ai { nullptr };

    jt::Vector2f m_pos;
    jt::Vector2f m_offset;

    bool m_walkingRight { true };
    int m_level { 1 };
    int m_unitID { 0 };
    int m_playerID { 0 };
    float m_age { 0.0f };
    float m_hp { 1.0f };
    int m_experience {};

    mutable std::string m_newAnim { "" };
};

#endif // JAMTEMPLATE_SERVER_UNIT_HPP
