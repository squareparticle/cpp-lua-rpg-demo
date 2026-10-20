#pragma once

#include <memory>

class BaseGameObject;
namespace YAML
{
class Node;
}

// These functions are the YAML-facing construction boundary for each concrete
// game-object type. The objects themselves remain unaware of yaml-cpp.
namespace GameObjectLoad
{
std::unique_ptr<BaseGameObject> SpriteObject(const YAML::Node& node);
std::unique_ptr<BaseGameObject> ScrollingSurface(const YAML::Node& node);
std::unique_ptr<BaseGameObject> Player(const YAML::Node& node);
std::unique_ptr<BaseGameObject> BattleEnemy(const YAML::Node& node);
std::unique_ptr<BaseGameObject> PlayerBattle(const YAML::Node& node);
std::unique_ptr<BaseGameObject> InteractableObject(const YAML::Node& node);
}
