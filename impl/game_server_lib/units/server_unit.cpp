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

ServerUnit::ServerUnit(jt::LoggerInterface& logger, const UnitInfo& info,
    std::shared_ptr<jt::Box2DWorldInterface> world)
    : m_logger { logger }
{
    m_info = info;
    if (m_info.ai.type == AiInfo::CLOSE_COMBAT) {
        m_ai = std::make_unique<AiCloseCombat>();
    } else if (m_info.ai.type == AiInfo::RANGED_COMBAT) {
        m_ai = std::make_unique<AiRangedCombat>();
    } else {
        m_logger.error("Create a unit with unknown ai type: " + std::to_string(m_info.ai.type),
            { "ServerUnit", "Ai" });
        m_ai = std::make_unique<AiCloseCombat>();
    }

    m_hp = m_info.hitpoints;

    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    m_physicsObject = std::make_shared<jt::Box2DObject>(world, &bodyDef);

    b2FixtureDef fixtureDef;
    fixtureDef.friction = 0.0f;
    fixtureDef.density = 1.0f;
    b2CircleShape circleCollider {};
    // TODO move to GP
    circleCollider.m_radius = terrainChunkSizeInPixel / 2.0f - 2.0f;
    fixtureDef.shape = &circleCollider;
    m_physicsObject->getB2Body()->CreateFixture(&fixtureDef);
}

ObjectProperties ServerUnit::saveState() const
{
    m_logger.verbose("save State", { "ServerUnit" });
    ObjectProperties props;
    props.ints[jk::unitID] = m_unitID;
    props.ints[jk::playerID] = m_playerID;
    props.floats[jk::positionX] = m_pos.x;
    props.floats[jk::positionY] = m_pos.y;
    props.floats[jk::offsetX] = m_offset.x;
    props.floats[jk::offsetY] = m_offset.y;
    props.floats[jk::hpCurrent] = m_hp;
    props.floats[jk::hpMax] = m_info.hitpoints;
    props.strings[jk::unitType] = m_info.type;
    props.bools[jk::unitWalkingRight] = m_walkingRight;
    if (!m_newAnim.empty()) {
        props.strings[jk::unitAnim] = m_newAnim;
        m_newAnim = "";
    }
    return props;
}

void ServerUnit::updateState(ObjectProperties const& props)
{
    m_logger.debug("update State", { "ServerUnit" });
    m_unitID = props.ints.at(jk::unitID);
    m_playerID = props.ints.at(jk::playerID);
    m_pos = jt::Vector2f { props.floats.at(jk::positionX), props.floats.at(jk::positionY) };
    m_physicsObject->setPosition(m_pos);
    m_offset = jt::Vector2f { props.floats.at(jk::offsetX), props.floats.at(jk::offsetY) };
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

UnitInfo const& ServerUnit::getInfo() const { return m_info; }
std::shared_ptr<jt::Box2DObject> ServerUnit::getPhysicsObject() { return m_physicsObject; }
int ServerUnit::getCost() { return m_info.cost; }
