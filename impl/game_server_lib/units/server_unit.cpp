#include "server_unit.hpp"
#include "ai/ai_close_combat.hpp"
#include "ai/ai_ranged_combat.hpp"
#include "damage_info.hpp"
#include "json_keys.hpp"
#include "map/terrain.hpp"
#include "math_helper.hpp"
#include "object_properties.hpp"
#include "unit_info.hpp"
#include "world_info_interface.hpp"
#include <game_properties.hpp>
#include <map/terrain.hpp>
#include <Box2D/Dynamics/b2Body.h>
#include <cmath>
#include <memory>
#include <string>

ServerUnit::ServerUnit(jt::LoggerInterface& logger, UnitInfo const& info,
    std::shared_ptr<jt::Box2DWorldInterface> world)
    : m_logger { logger }
{
    m_infoBase = info;
    m_infoLevel = info;

    if (m_infoBase.ai.type == AiInfo::CLOSE_COMBAT) {
        m_ai = std::make_unique<AiCloseCombat>();
    } else if (m_infoBase.ai.type == AiInfo::RANGED_COMBAT) {
        m_ai = std::make_unique<AiRangedCombat>();
    } else {
        m_logger.error("Create a unit with unknown ai type: " + std::to_string(m_infoBase.ai.type),
            { "ServerUnit", "Ai" });
        m_ai = std::make_unique<AiCloseCombat>();
    }

    m_hp = m_infoBase.hitpointsMax;

    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    m_physicsObject = std::make_shared<jt::Box2DObject>(world, &bodyDef);

    b2FixtureDef fixtureDef;
    fixtureDef.friction = 0.0f;
    fixtureDef.density = 1.0f;
    b2CircleShape circleCollider {};
    circleCollider.m_radius = m_infoBase.colliderRadius;
    fixtureDef.shape = &circleCollider;
    m_physicsObject->getB2Body()->CreateFixture(&fixtureDef);
}

ObjectProperties ServerUnit::saveState() const
{
    m_logger.verbose("save State", { "ServerUnit" });
    ObjectProperties props;
    props.ints[jk::unitID] = m_unitID;
    props.ints[jk::playerID] = m_playerID;
    props.strings[jk::unitType] = m_infoBase.type;

    //    m_logger.info("save exp with value: " + std::to_string(m_experience));
    props.ints[jk::experience] = m_experience;
    props.ints[jk::experienceForLevelUp] = m_infoBase.experienceRequiredForLevelUp;
    props.ints[jk::level] = m_level;

    props.floats[jk::positionX] = m_pos.x;
    props.floats[jk::positionY] = m_pos.y;

    props.floats[jk::offsetX] = m_offset.x;
    props.floats[jk::offsetY] = m_offset.y;
    props.floats[jk::hpCurrent] = m_hp;
    props.floats[jk::hpMax] = m_infoLevel.hitpointsMax;

    props.bools[jk::unitWalkingRight] = m_walkingRight;

    std::string str;
    for (auto const& upg : m_upgrades) {
        str += upg.name + ",";
    }
    if (!str.empty()) {
        str.pop_back();
    }
    props.strings[jk::upgrades] = str;

    if (!m_newAnim.empty()) {
        props.strings[jk::unitAnim] = m_newAnim;
        m_newAnim = "";
    }
    return props;
}

void ServerUnit::updateState(ObjectProperties* props)
{
    m_roundStartObjectProperties = props;

    m_logger.debug("update State", { "ServerUnit" });
    m_unitID = props->ints.at(jk::unitID);
    m_playerID = props->ints.at(jk::playerID);
    if (props->ints.count(jk::level) == 1) {
        m_level = props->ints.at(jk::level);
    } else {
        m_level = 1;
    }
    m_pos = jt::Vector2f { props->floats.at(jk::positionX), props->floats.at(jk::positionY) };
    m_physicsObject->setPosition(m_pos);
    m_offset = jt::Vector2f { props->floats.at(jk::offsetX), props->floats.at(jk::offsetY) };
    if (props->ints.count(jk::experience) == 0) {
        m_experience = m_infoBase.experienceRequiredForLevelUp * m_level;
    } else {
        m_experience = props->ints.at(jk::experience);
        m_logger.info("load exp with value: " + std::to_string(m_experience));
    }
}

void ServerUnit::levelUnitUp()
{
    m_logger.debug("Level up", { "ServerUnit" });
    if (m_experience > 0) {
        m_logger.warning("Not enough experience to upgrade unit", { "ServerUnit" });
        return;
    }
    m_logger.info("upgrade unit to level: " + std::to_string(m_level), { "ServerUnit" });

    m_level++;
    m_roundStartObjectProperties->ints[jk::level] = m_level;
    m_infoLevel.hitpointsMax = m_infoBase.hitpointsMax * m_level;
    m_infoLevel.damage = m_infoBase.damage * m_level;
    m_experience = m_infoBase.experienceRequiredForLevelUp * m_level;
}

void ServerUnit::update(float elapsed, WorldInfoInterface& world)
{
    m_logger.verbose("update " + std::to_string(elapsed), { "ServerUnit", "GameSimulation" });
    m_age += elapsed;
    if (!isAlive()) {
        return;
    }
    m_ai->update(elapsed, *this, world);

    if (m_walkingRight && m_physicsObject->getVelocity().x < 0) {
        m_walkingRight = false;
    } else if (!m_walkingRight && m_physicsObject->getVelocity().x > 0) {
        m_walkingRight = true;
    }
    m_pos = m_physicsObject->getPosition();
    if (m_pos.x <= 0) {
        m_pos.x = 0;
    } else if (m_pos.x >= GP::GetScreenSize().x - terrainChunkSizeInPixel - 4.0f) {
        m_pos.x = GP::GetScreenSize().x - terrainChunkSizeInPixel - 4.0f;
    }
    if (m_pos.y <= 0) {
        m_pos.y = 0;
    } else if (m_pos.y >= GP::GetScreenSize().y - terrainChunkSizeInPixel - 4.0f) {
        m_pos.y = GP::GetScreenSize().y - terrainChunkSizeInPixel - 4.0f;
    }
    m_physicsObject->setPosition(m_pos);
}

void ServerUnit::setPosition(jt::Vector2f const& pos)
{
    m_pos = pos;
    m_physicsObject->setPosition(pos);
}

jt::Vector2f ServerUnit::getPosition() const { return m_pos; }

void ServerUnit::setOffset(jt::Vector2f const& offset) { m_offset = offset; }

jt::Vector2f ServerUnit::getOffset() const { return m_offset; }

void ServerUnit::setUnitID(int unitID) { m_unitID = unitID; }

int ServerUnit::getPlayerID() const { return m_playerID; }

int ServerUnit::getUnitID() const { return m_unitID; }

void ServerUnit::takeDamage(const DamageInfo& damage)
{
    // TODO take armor into account
    m_hp -= damage.damage;
    if (m_hp > 0) {
        m_newAnim = "damage";
    } else {
        m_newAnim = "death";
    }
}

bool ServerUnit::isAlive() const { return m_hp > 0; }

UnitInfo const& ServerUnit::getUnitInfoBase() const { return m_infoBase; }

UnitInfo ServerUnit::getUnitInfoFull() const
{
    UnitInfo info;

    info.hitpointsMax = m_infoLevel.hitpointsMax;
    info.damage = m_infoLevel.damage;
    info.unlockCost = m_infoLevel.unlockCost;
    info.cost = m_infoLevel.cost;
    info.type = m_infoBase.type;
    info.ai = m_infoBase.ai;
    info.attackTimerMax = m_infoLevel.attackTimerMax;
    info.animations = m_infoBase.animations;
    info.colliderRadius = m_infoBase.colliderRadius;
    info.movementSpeed = m_infoLevel.movementSpeed;
    info.experienceGainWhenKilled = m_infoBase.experienceGainWhenKilled * std::sqrt(m_level);

    for (auto const& upg : m_upgrades) {
        applyUpgrade(info, upg);
    }
    return info;
}

std::shared_ptr<jt::Box2DObject> ServerUnit::getPhysicsObject() { return m_physicsObject; }

int ServerUnit::getCost() { return m_infoBase.cost; }
void ServerUnit::gainExperience(int exp)
{
    m_experience -= exp;

    if (m_experience <= 0) {
        // TODO only level up if player pays for it
        levelUnitUp();
    }
    m_logger.info("gain experience. New exp: " + std::to_string(m_experience));
    m_roundStartObjectProperties->ints[jk::experience] = m_experience;
}
int ServerUnit::getLevel() const { return m_level; }

void ServerUnit::applyUpgrades(const std::vector<UpgradeUnitData>& upgrades)
{
    for (auto const& upg : upgrades) {
        if (upg.playerID != getPlayerID()) {
            continue;
        }
        if (upg.unityType != m_infoBase.type) {
            continue;
        }

        m_upgrades.push_back(upg.upgrade);
    }
}
