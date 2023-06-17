#ifndef JAMTEMPLATE_MESSAGE_HPP
#define JAMTEMPLATE_MESSAGE_HPP

#include <nlohmann.hpp>
#include <cstdint>

enum class MessageType : std::uint8_t {
    InitialPing,
    PlayerIdResponse,
    AddBot,
    // stay alive ping not needed with TCP
    StayAlivePing,
    AllPlayersConnected,
    RoundReady,
    UnitUpgrade,
    SimulationResult
};

struct Message {
    MessageType type { MessageType::InitialPing };
    int playerId { -1 };
    std::string data { "" };
};

void to_json(nlohmann::json& j, const Message& message);
void from_json(const nlohmann::json& j, Message& message);

#endif // JAMTEMPLATE_MESSAGE_HPP
