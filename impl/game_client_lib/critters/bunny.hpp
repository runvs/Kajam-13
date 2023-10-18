#ifndef JAMTEMPLATE_BUNNY_HPP
#define JAMTEMPLATE_BUNNY_HPP

#include <critters/critter.hpp>

class Bunny : public Critter {
public:
    Bunny()
        : Critter { "assets/critters/bunny.aseprite", "walk" }
    {
    }
};

#endif // JAMTEMPLATE_BUNNY_HPP
