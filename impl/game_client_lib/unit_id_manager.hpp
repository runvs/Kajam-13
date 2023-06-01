#ifndef JAMTEMPLATE_UNIT_ID_MANAGER_HPP
#define JAMTEMPLATE_UNIT_ID_MANAGER_HPP

#include <map>

class UnitIdManager {
public:
    int getIdForPlayer(int pid);

private:
    std::map<int, int> m_ids;
};


#endif //JAMTEMPLATE_UNIT_ID_MANAGER_HPP
