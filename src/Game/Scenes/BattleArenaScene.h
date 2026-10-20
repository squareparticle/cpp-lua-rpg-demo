#pragma once

#include "../../Framework/Scenes/BaseScene.h"
#include "../Battle/BattleMove.h"
#include "../Battle/BattleRequest.h"
#include "../Items/ItemDefinition.h"
#include "../UI/BattleMenu.h"

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class BattleEnemy;
class Audio;
class Inventory;
class LuaSystem;
class PlayerBattle;
class PlayerState;
struct BattleActionIntent;

class BattleArenaScene final : public BaseScene
{
public:
    BattleArenaScene(
        ResourceManager& resources,
        Audio& audio,
        LuaSystem& luaSystem,
        Inventory& inventory,
        PlayerState& playerState,
        const std::unordered_map<int, int>& experienceToNextLevelByLevel,
        const std::unordered_map<std::string, ItemDefinition>& itemDefinitions,
        BattleRequest request = {});

    bool AddBattleMove(BattleMove move);
    bool OnGameObjectCreated(
        BaseGameObject& object,
        ObjectLayer& owner) override;
    bool Init() override;
    void Update(float deltaTime, const InputState& input) override;
    void Render(const Graphics& graphics) override;
    std::optional<SceneChangeRequest>
        TakeSceneChangeRequest() override;

private:
    enum class BattlePhase
    {
        PlayerInput,
        PlayerAction,
        EnemyAction,
        Victory,
        Defeat
    };

    struct ActiveAction
    {
        // The selected enemy is the target during PlayerAction and the actor
        // during EnemyAction. This keeps the first one-player battle slice
        // explicit without reintroducing a common combatant inheritance tree.
        const BattleMove* move = nullptr;
        BattleEnemy* enemy = nullptr;
        bool impactApplied = false;
        bool targetAnimationStarted = false;
    };

    bool BeginPlayerAction(
        const std::string& moveId,
        BattleEnemy& target);
    bool BeginEnemyAction(
        BattleEnemy& actor,
        const std::string& moveId,
        const std::string& targetId);
    bool ChooseEnemyAction(
        const BattleEnemy& actor,
        BattleActionIntent& intent) const;
    const BattleMove* FindMove(const std::string& moveId) const;
    const BattleMove* FindFirstUsableAttack(
        const std::vector<std::string>& moveIds,
        int availableMp) const;
    bool ValidateKnownMoves(
        const std::vector<std::string>& moveIds,
        std::string_view ownerId) const;
    void UpdateActiveAction();
    void ApplyActiveActionImpact();
    void FinishActiveAction();
    bool ShowVictoryRewards();
    void OpenPlayerTurn();
    void ProcessMenuFeedback();
    std::vector<BattleEnemy*> GetActiveEnemies() const;

    std::vector<std::string> requestedEnemyIds_;
    Audio& audio_;
    LuaSystem& luaSystem_;
    Inventory& inventory_;
    PlayerState& playerState_;
    const std::unordered_map<int, int>& experienceToNextLevelByLevel_;
    const std::unordered_map<std::string, ItemDefinition>& itemDefinitions_;
    std::string returnSceneId_;
    SceneParameterMap returnParameters_;
    PlayerBattle* player_ = nullptr;
    std::vector<BattleEnemy*> scriptedEnemies_;
    std::vector<BattleEnemy*> encounterEnemies_;
    std::unordered_map<std::string, BattleEnemy*> enemiesById_;
    std::unordered_map<std::string, BattleMove> movesById_;
    std::optional<SceneChangeRequest> pendingSceneChange_;
    BattleMenu battleMenu_;
    BattlePhase phase_ = BattlePhase::PlayerInput;
    ActiveAction activeAction_;
};
