
#ifndef JAMTEMPLATE_UNIT_HPP
#define JAMTEMPLATE_UNIT_HPP

#include "game_object.hpp"
#include "shape.hpp"
#include "vector.hpp"
#include <nlohmann.hpp>
#include <object_properties.hpp>
#include <memory>

class Unit : public jt::GameObject {

    std::shared_ptr<jt::Shape> m_shape;

    void doCreate() override;
    void doUpdate(float const elapsed) override;
    void doDraw() const override;

public:
    // TODO make this an interface & base class?
    void updateState(ObjectProperties const& props);
    jt::Vector2f getPosition() const;
    void setPosition(jt::Vector2f const& pos);
};

#endif // JAMTEMPLATE_UNIT_HPP
