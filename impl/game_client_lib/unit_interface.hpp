#ifndef JAMTEMPLATE_UNIT_INTERFACE_HPP
#define JAMTEMPLATE_UNIT_INTERFACE_HPP

#include <network_data/unit_info.hpp>

class UnitInterface {
public:
    ~UnitInterface() = default;
    virtual bool isMouseOver() const = 0;
    virtual UnitInfo const& getInfo() const = 0;
    virtual void flash() = 0;
    virtual void setHighlight(bool v) = 0;
    virtual int getLevel() const = 0;
    virtual int getPlayerID() const = 0;
};

#endif // JAMTEMPLATE_UNIT_INTERFACE_HPP
