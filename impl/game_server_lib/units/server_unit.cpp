#include "server_unit.hpp"
#include <game_properties.hpp>
#include <map/terrain.hpp>
#include <math_helper.hpp>
#include <network_data/damage_info.hpp>
#include <network_data/unit_info.hpp>
#include <network_data/unit_server_to_client_data.hpp>
#include <units/ai/ai_cannon.hpp>
#include <units/ai/ai_close_combat.hpp>
#include <units/ai/ai_ranged_combat.hpp>
#include <world_info_interface.hpp>
#include <Box2D/Dynamics/b2Body.h>
#include <cmath>
#include <memory>
#include <string>

ServerUnit::ServerUnit(jt::LoggerInterface& logger, UnitInfo const& info,
    std::shared_ptr<jt::Box2DWorldInterface> world)
    : m_logger { logger }
{
    m_infoBase = info;

    if (m_infoBase.ai.type == AiInfo::CLOSE_COMBAT) {
        m_ai = std::make_unique<AiCloseCombat>();
    } else if (m_infoBase.ai.type == AiInfo::RANGED_COMBAT) {
        m_ai = std::make_unique<AiRangedCombat>();
    } else if (m_infoBase.ai.type == AiInfo::CANNON) {
        m_ai = std::make_unique<AiCannon>();
    } else {
        m_logger.error("Create a unit with unknown ai type: " + std::to_string(m_infoBase.ai.type),
            { "ServerUnit", "Ai" });
        m_ai = std::make_unique<AiCloseCombat>();
    }

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

UnitServerToClientData ServerUnit::saveState() const
{
    m_logger.verbose("save State", { "ServerUnit" });
    auto const info = getUnitInfoWithLevelAndUpgrades(m_infoBase, m_level, m_upgrades);
    UnitServerToClientData data;
    data.unitID = m_unitID;
    data.playerID = m_playerID;
    data.unitType = info.type;

    data.experience = info.experienceRequiredForLevelUp - m_experience;
    data.level = m_level;

    data.positionX = m_pos.x;
    data.positionY = m_pos.y;

    data.offsetX = m_offset.x;
    data.offsetY = m_offset.y;
    data.hpCurrent = m_hp;
    data.hpMax = info.hitpointsMax;

    data.unitWalkingRight = m_walkingRight;

    if (!m_newAnim.empty()) {
        data.unitAnim = m_newAnim;
        m_newAnim = "";
    }
    return data;
}

void ServerUnit::setRoundStartState(UnitServerRoundStartData* props)
{
    m_roundStartObjectProperties = props;

    m_logger.debug("update State", { "ServerUnit" });
    m_unitID = props->unitClientToServerData.unitID;
    m_playerID = props->unitClientToServerData.playerID;

    m_level = props->level;

    m_pos = jt::Vector2f { props->unitClientToServerData.positionX,
        props->unitClientToServerData.positionY };
    m_physicsObject->setPosition(m_pos);
    m_offset = jt::Vector2f { props->unitClientToServerData.offsetX,
        props->unitClientToServerData.offsetY };
    if (props->experience == 0) {
        m_experience = static_cast<int>(m_infoBase.experienceRequiredForLevelUp * sqrt(m_level));
    } else {
        m_experience = props->experience;
    }

    auto const info = getUnitInfoWithLevelAndUpgrades(m_infoBase, m_level, m_upgrades);
    m_hp = info.hitpointsMax;
    m_logger.info("Load Unit with Level: " + std::to_string(m_level) + "( "
            + std::to_string(m_experience) + " / "
            + std::to_string(info.experienceRequiredForLevelUp) + " )",
        { "ServerUnit" });
}

void ServerUnit::levelUnitUp()
{
    m_logger.debug("Level up", { "ServerUnit" });
    if (m_experience > 0) {
        m_logger.warning("Not enough experience to upgrade unit", { "ServerUnit" });
        return;
    }

    m_level++;
    m_logger.info("upgrade unit to level: " + std::to_string(m_level), { "ServerUnit" });
    m_roundStartObjectProperties->level = m_level;
    m_experience = m_infoBase.experienceRequiredForLevelUp * m_level;
}

void ServerUnit::update(float elapsed, WorldInfoInterface& world)
{
    m_logger.verbose("update " + std::to_string(elapsed), { "ServerUnit", "GameSimulation" });
    m_age += elapsed;
    if (!isAlive()) {
        return;
    }
    m_ai->update(elapsed, this, world);

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
    auto const info = getUnitInfoFull();
    float damageValue = damage.damage;
    if (std::count(info.armor.types.begin(), info.armor.types.end(), "25%RangedDamage") != 0) {
        if (std::count(damage.damageTypes.begin(), damage.damageTypes.end(), "ranged") != 0) {
            damageValue *= 0.25f;
        }
    }
    if (std::count(info.armor.types.begin(), info.armor.types.end(), "NoSplashDamage") != 0) {
        if (std::count(damage.damageTypes.begin(), damage.damageTypes.end(), "splash") != 0) {
            damageValue = 0.0f;
        }
    }
    std::string armorTypes = "";
    for (auto const& av : info.armor.types) {
        armorTypes += av + ", ";
    }
    std::string damageTypes = "";
    for (auto const& dt : damage.damageTypes) {
        damageTypes += dt + ", ";
    }
    m_logger.debug("unit [" + armorTypes + "] take damage [" + damageTypes + "] ("
        + std::to_string(damage.damage) + ":" + std::to_string(damageValue) + ")");
    m_logger.debug("initialHp: " + std::to_string(m_hp)
        + ", after Damage Hp: " + std::to_string(m_hp - damageValue));
    m_hp -= damageValue;

    if (m_hp > 0) {
        setAnim("damage");

    } else {
        setAnim("death");
    }
}

bool ServerUnit::isAlive() const { return m_hp > 0; }

UnitInfo ServerUnit::getUnitInfoFull() const
{
    return getUnitInfoWithLevelAndUpgrades(m_infoBase, m_level, m_upgrades);
}

std::shared_ptr<jt::Box2DObject> ServerUnit::getPhysicsObject() { return m_physicsObject; }

int ServerUnit::getCost() { return m_infoBase.cost; }
void ServerUnit::gainExperience(int exp)
{
    m_experience -= exp;
    if (m_experience <= 0) {
        levelUnitUp();
    }
    m_logger.info("gain experience. New exp: " + std::to_string(m_experience));
    m_roundStartObjectProperties->experience = m_experience;
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
void ServerUnit::setAnim(const std::string& newAnimName) { m_newAnim = newAnimName; }
