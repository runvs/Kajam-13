#ifndef JAMTEMPLATE_PLACED_UNIT_HPP
#define JAMTEMPLATE_PLACED_UNIT_HPP

#include "state_menu.hpp"
#include <animation.hpp>
#include <game_object.hpp>
#include <object_properties.hpp>
#include <shape.hpp>
#include <unit_info.hpp>
#include <vector.hpp>
#include <memory>

class PlacedUnit : public jt::GameObject {
public:
    explicit PlacedUnit(UnitInfo info);
    ObjectProperties saveState() const;

    void setIDs(int uid, int pid);
    void setPosition(jt::Vector2f const& pos);
    jt::Vector2f getPosition() const;
    void setOffset(jt::Vector2f const& offset);
    jt::Vector2f getOffset() const;

private:
    UnitInfo m_info;
    //    std::shared_ptr<jt::Shape> m_shape;
    std::shared_ptr<jt::Animation> m_anim;
    int m_unitID { 0 };
    int m_playerID { 0 };
    jt::Vector2f m_position;
    jt::Vector2f m_offset;

    void doCreate() override;
    void doUpdate(float const elapsed) override;
    void doDraw() const override;
};
#endif // JAMTEMPLATE_PLACED_UNIT_HPP
