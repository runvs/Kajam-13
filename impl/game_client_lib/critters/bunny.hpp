#ifndef JAMTEMPLATE_BUNNY_HPP
#define JAMTEMPLATE_BUNNY_HPP

#include <critters/critter.hpp>

class Bunny : public Critter {
public:
    Bunny()
        : Critter { "assets/critters/bunny.json", "walk" }
    {
    }
};

#endif // JAMTEMPLATE_BUNNY_HPP
