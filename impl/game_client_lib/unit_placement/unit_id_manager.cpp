
#include "unit_id_manager.hpp"

int UnitIdManager::getIdForPlayer(int pid) {
    auto const uid = m_ids[pid];
    m_ids[pid]++;
    return uid;
}
