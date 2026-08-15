#pragma once

#include "game/NeighborWoman.hpp"
#include "game/Elder.hpp"
#include "game/Player.hpp"
#include "game/SystemDog.hpp"

namespace game
{
inline void FaceConversationPartners(Player &player, SystemDog &system)
{
    player.FaceToward(system.Position().x);
    system.FaceToward(player.Position().x);
}

inline void FaceConversationPartners(Player &player, NeighborWoman &neighbor)
{
    player.FaceToward(neighbor.Position().x);
    neighbor.FaceToward(player.Position().x);
}

inline void FaceConversationPartners(Player &player, Elder &elder, Vector2 elderPosition)
{
    player.FaceToward(elderPosition.x);
    elder.FaceToward(elderPosition.x, player.Position().x);
}
} // namespace game
