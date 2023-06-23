#ifndef JAMTEMPLATE_BIRD_HPP
#define JAMTEMPLATE_BIRD_HPP

#include <critters/critter.hpp>

class Bird : public Critter {
public:
    Bird()
        : Critter { "assets/critters/bird.json", "fly" }
    {
    }
};

#endif // JAMTEMPLATE_BIRD_HPP
