#ifndef JAMTEMPLATE_BIRD_HPP
#define JAMTEMPLATE_BIRD_HPP

#include <critters/critter.hpp>

class Bird : public Critter {
public:
    Bird()
        : Critter { "assets/critters/bird.aseprite", "fly" }
    {
    }
};

#endif // JAMTEMPLATE_BIRD_HPP
