#ifndef JAMTEMPLATE_DEER_HPP
#define JAMTEMPLATE_DEER_HPP

#include <critters/critter.hpp>

class Deer : public Critter {
public:
    Deer()
        : Critter { "assets/critters/deer.json", "walk" }
    {
    }
};

#endif // JAMTEMPLATE_DEER_HPP
