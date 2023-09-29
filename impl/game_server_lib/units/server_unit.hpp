
#ifndef JAMTEMPLATE_SERVER_UNIT_HPP
#define JAMTEMPLATE_SERVER_UNIT_HPP

#include <box2dwrapper/box2d_object.hpp>
#include <log/logger_interface.hpp>
#include <network_data/unit_info.hpp>
#include <object_properties.hpp>
#include <simulation_object_interface.hpp>
#include <units/ai/ai_interface.hpp>
#include <upgrade_unit_data.hpp>
#include <vector2.hpp>
#include <memory>
#include <string>
#include <vector>

class ServerUnit : public SimulationObjectInterface {
public:
    explicit ServerUnit(jt::LoggerInterface& logger, const UnitInfo& info,
        std::shared_ptr<jt::Box2DWorldInterface> world);

    void update(float elapsed, WorldInfoInterface& world) override;
    UnitServerToClientData saveState() const override;

    void setRoundStartState(UnitServerRoundStartData* props) override;

    void setWalkingDirection(bool right);

    void applyUpgrades(std::vector<UpgradeUnitData> const& upgrades);

    void levelUnitUp() override;

    void setPosition(jt::Vector2f const& pos);
    jt::Vector2f getPosition() const override;

    void setOffset(jt::Vector2f const& offset);
    jt::Vector2f getOffset() const override;

    int getPlayerID() const override;
    void setUnitID(int unitID);
    int getUnitID() const override;

    void takeDamage(const DamageInfo& damage) override;

    bool isAlive() const override;
    // with upgrades and level
    UnitInfo getUnitInfoFull() const override;

    std::shared_ptr<jt::Box2DObject> getPhysicsObject();

    int getCost() override;

    void gainExperience(int exp) override;
    int getLevel() const override;

    void setAnim(std::string const& newAnimName);

    void setCurrentSlope(float slope);

private:
    jt::LoggerInterface& m_logger;
    std::shared_ptr<jt::Box2DObject> m_physicsObject { nullptr };

    UnitServerRoundStartData* m_roundStartObjectProperties { nullptr };

    UnitInfo m_infoBase;
    std::vector<UpgradeInfo> m_upgrades;

    std::unique_ptr<AiInterface> m_ai { nullptr };

    jt::Vector2f m_pos;
    jt::Vector2f m_offset;

    bool m_walkingRight { true };

    int m_unitID { 0 };
    int m_playerID { 0 };
    float m_age { 0.0f };
    float m_hp { 1.0f };

    int m_level { 1 };
    int m_experience {};

    mutable std::string m_newAnim { "" };
    float m_slope { 0 };

    void clampPositionToScreen();
};

#endif // JAMTEMPLATE_SERVER_UNIT_HPP
