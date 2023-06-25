#ifndef JAMTEMPLATE_AI_INTERFACE_HPP
#define JAMTEMPLATE_AI_INTERFACE_HPP

class ServerUnit;
class WorldInfoInterface;

class AiInterface {
public:
    virtual ~AiInterface() = default;
    virtual void update(float elapsed, ServerUnit* unit, WorldInfoInterface& world) = 0;
};

#endif // JAMTEMPLATE_AI_INTERFACE_HPP
