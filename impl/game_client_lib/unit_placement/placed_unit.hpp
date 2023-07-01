#ifndef JAMTEMPLATE_PLACED_UNIT_HPP
#define JAMTEMPLATE_PLACED_UNIT_HPP

#include <animation.hpp>
#include <game_object.hpp>
#include <network_data/unit_client_to_server_data.hpp>
#include <network_data/unit_info.hpp>
#include <tweens/tween_base.hpp>
#include <unit_interface.hpp>
#include <vector2.hpp>
#include <memory>

class PlacedUnit : public jt::GameObject, public UnitInterface {
public:
    explicit PlacedUnit(UnitInfo info);
    UnitClientToServerData saveState() const;

    void setIDs(int uid, int pid);
    void setPosition(jt::Vector2f const& pos);
    jt::Vector2f getPosition() const;
    void setOffset(jt::Vector2f const& offset);
    jt::Vector2f getOffset() const;

    bool isMouseOver() const override;
    UnitInfo const& getInfo() const override;

    std::shared_ptr<jt::Tween> createInitialTween();

    int getLevel() const override;
    int getExp() const override;
    int getExpForLevelUp() const override;

    int getPlayerID() const override;
    void flash() override;
    void setHighlight(bool v) override;

private:
    UnitInfo m_info;
    std::shared_ptr<jt::Animation> m_anim;
    bool m_glowActive { false };
    std::shared_ptr<jt::Sprite> m_glow;
    int m_unitID { 0 };
    int m_playerID { 0 };
    jt::Vector2f m_position;
    jt::Vector2f m_offset;

    void doCreate() override;
    void doUpdate(float const elapsed) override;
    void doDraw() const override;
};

#endif // JAMTEMPLATE_PLACED_UNIT_HPP
