
#ifndef JAMTEMPLATE_UNIT_HPP
#define JAMTEMPLATE_UNIT_HPP

#include <bar.hpp>
#include <counted_object.hpp>
#include <game_object.hpp>
#include <nlohmann.hpp>
#include <object_properties.hpp>
#include <shape.hpp>
#include <vector.hpp>
#include <memory>

class Unit : public jt::GameObject, public jt::CountedObj<Unit> {
public:
    // needed for unit id counting
    // TODO remove counting in constructor and create a UnitPlacementManager
    // TODO unitIDs should/could be issued by the server.
    // TODO Or this is a combination of playerid+unitid
    Unit();
    // TODO make this an interface & base class? Or even better a UnitInfo class
    void updateState(ObjectProperties const& props);
    ObjectProperties saveState() const;

    void setPosition(jt::Vector2f const& pos);
    int getUnitID() const;
    void setPlayerID(int pid);

private:
    std::shared_ptr<jt::Shape> m_shape;
    int m_unitID;
    int m_playerID { 0 };
    float m_hpMax { 100.0f };
    float m_hp { 100.0f };
    std::shared_ptr<jt::Bar> m_hpBar;

    void doCreate() override;
    void doUpdate(float const elapsed) override;
    void doDraw() const override;
};

#endif // JAMTEMPLATE_UNIT_HPP
