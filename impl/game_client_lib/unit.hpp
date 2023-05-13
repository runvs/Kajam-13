
#ifndef JAMTEMPLATE_UNIT_HPP
#define JAMTEMPLATE_UNIT_HPP

#include "counted_object.hpp"
#include "game_object.hpp"
#include "shape.hpp"
#include "vector.hpp"
#include <nlohmann.hpp>
#include <object_properties.hpp>
#include <memory>

class Unit : public jt::GameObject, public jt::CountedObj<Unit> {
public:
    Unit();
    // TODO make this an interface & base class?
    void updateState(ObjectProperties const& props);

    void setPosition(jt::Vector2f const& pos);

    ObjectProperties saveState() const;
    int getUnitID() const;

private:
    std::shared_ptr<jt::Shape> m_shape;
    int m_unitID;

    void doCreate() override;
    void doUpdate(float const elapsed) override;
    void doDraw() const override;
};

#endif // JAMTEMPLATE_UNIT_HPP
