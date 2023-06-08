#include "server_unit.hpp"
#include "ai/ai_archer.hpp"
#include "ai/ai_swordman.hpp"
#include "damage_info.hpp"
#include "json_keys.hpp"
#include "map/terrain.hpp"
#include "math_helper.hpp"
#include "object_properties.hpp"
#include "unit_info.hpp"
#include "world_info_interface.hpp"
#include <Box2D/Dynamics/b2Body.h>
#include <cmath>
#include <memory>

#include <iostream>

ServerUnit::ServerUnit(const UnitInfo& info, std::shared_ptr<jt::Box2DWorldInterface> world)
{
    m_info = info;
    // TODO use stored AI information instead of type
    if (m_info.ai.type == AiInfo::SWORDMAN) {
        m_ai = std::make_unique<AiSwordman>();
    } else if (m_info.ai.type == AiInfo::ARCHER) {
        m_ai = std::make_unique<AiArcher>();
    } else {
        // TODO pass in and use logger
        std::cerr << "Warning: create a unit with unknown ai type\n";
        m_ai = std::make_unique<AiSwordman>();
    }

    m_hp = m_info.hitpoints;

    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    m_physicsObject = std::make_shared<jt::Box2DObject>(world, &bodyDef);

    b2FixtureDef fixtureDef;
    fixtureDef.friction = 0.0f;
    fixtureDef.density = 1.0f;
    b2CircleShape circleCollider {};
    circleCollider.m_radius = terrainChunkSizeInPixel / 2.0f - 2.0f;
    // Not sure if we need to take offset into account
    //    circleCollider.m_p.Set(5.0f, 5.0f);
    fixtureDef.shape = &circleCollider;
    m_physicsObject->getB2Body()->CreateFixture(&fixtureDef);
}

ObjectProperties ServerUnit::saveState() const
{
    ObjectProperties props;
    props.ints[jk::unitID] = m_unitID;
    props.ints[jk::playerID] = m_playerID;
    props.floats[jk::positionX] = m_pos.x;
    props.floats[jk::positionY] = m_pos.y;
    props.floats[jk::hpCurrent] = m_hp;
    props.floats[jk::hpMax] = m_info.hitpoints;
    props.strings[jk::unitType] = m_info.type;
    if (!m_newAnim.empty()) {
        props.strings[jk::unitAnim] = m_newAnim;
        m_newAnim = "";
    }
    return props;
}

void ServerUnit::updateState(ObjectProperties const& props)
{
    m_unitID = props.ints.at(jk::unitID);
    m_playerID = props.ints.at(jk::playerID);
    m_pos = jt::Vector2f { props.floats.at(jk::positionX), props.floats.at(jk::positionY) };
    m_physicsObject->setPosition(m_pos);
}

void ServerUnit::update(float elapsed, WorldInfoInterface& world)
{
    m_age += elapsed;
    if (!isAlive()) {
        return;
    }
    m_ai->update(elapsed, *this, world);
    m_pos = m_physicsObject->getPosition();
}

void ServerUnit::setPosition(jt::Vector2f const& pos)
{
    m_pos = pos;
    m_physicsObject->setPosition(pos);
}

jt::Vector2f ServerUnit::getPosition() const { return m_pos; }
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
