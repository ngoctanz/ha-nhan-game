#include "game/LevelManager.hpp"
#include "game/BambooVillageLevel.hpp"

namespace game
{
bool LevelManager::Load(LevelId id)
{
    switch (id)
    {
        case LevelId::BambooVillage:
            current_ = std::make_unique<BambooVillageLevel>();
            break;
    }
    return current_ && current_->Load();
}

void LevelManager::Update(float deltaTime, const GameInput &input)
{
    if (current_) current_->Update(deltaTime, input);
}

void LevelManager::Draw(const Ui &ui) const
{
    if (current_) current_->Draw(ui);
}
} // namespace game
