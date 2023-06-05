#ifndef JAMTEMPLATE_PLACED_UNIT_HPP
#define JAMTEMPLATE_PLACED_UNIT_HPP

#include <game_object.hpp>
#include <object_properties.hpp>
#include <shape.hpp>
#include <unit_info.hpp>
#include <vector.hpp>

class PlacedUnit : public jt::GameObject {
public:
    explicit PlacedUnit(UnitInfo info);
    ObjectProperties saveState() const;

    void setIDs(int uid, int pid);
    void setPosition(jt::Vector2f const& pos);

private:
    UnitInfo m_info;
    std::shared_ptr<jt::Shape> m_shape;
    int m_unitID { 0 };
    int m_playerID { 0 };

    void doCreate() override;
    void doUpdate(float const elapsed) override;
    void doDraw() const override;
};
#endif // JAMTEMPLATE_PLACED_UNIT_HPP
