#ifndef JAMTEMPLATE_INTERNAL_STATE_INTERFACE_HPP
#define JAMTEMPLATE_INTERNAL_STATE_INTERFACE_HPP

class StateGame;

class InternalStateInterface {
public:
    virtual ~InternalStateInterface() = default;
    virtual void update(StateGame& state, float elapsed) = 0;
    virtual void draw(StateGame& state) = 0;
};

#endif // JAMTEMPLATE_INTERNAL_STATE_INTERFACE_HPP
