#include "BattleArenaScene.h"

#include "../../Framework/Resources/ResourceManager.h"
#include "../../Platform/Audio/Audio.h"
#include "../../Platform/DebugLog.h"
#include "../Battle/BattleRules.h"
#include "../State/Inventory.h"
#include "../State/PlayerState.h"
#include "../Objects/BattleEnemy.h"
#include "../Objects/PlayerBattle.h"
#include "../Scripting/LuaSystem.h"

#include <unordered_set>
#include <limits>
#include <utility>

namespace
{
template<typename BattleActor>
BattleActorSnapshot MakeBattleSnapshot(const BattleActor& actor)
{
    const BattleStats& stats = actor.GetStats();
    return {
        actor.GetId(),
        stats.name,
        stats.level,
        stats.hp,
        stats.maxHp,
        stats.mp,
        stats.maxMp,
        stats.attack,
        stats.defense,
        actor.GetMoveIds()};
}
}

BattleArenaScene::BattleArenaScene(
    ResourceManager& resources,
    Audio& audio,
    LuaSystem& luaSystem,
    Inventory& inventory,
    PlayerState& playerState,
    const std::unordered_map<int, int>& experienceToNextLevelByLevel,
    const std::unordered_map<std::string, ItemDefinition>& itemDefinitions,
    BattleRequest request)
    : BaseScene(
          resources,
          "assets/scenes/battlearena/map.yaml"),
      requestedEnemyIds_(std::move(request.enemyIds)),
      audio_(audio),
      luaSystem_(luaSystem),
      inventory_(inventory),
      playerState_(playerState),
      experienceToNextLevelByLevel_(experienceToNextLevelByLevel),
      itemDefinitions_(itemDefinitions),
      returnSceneId_(std::move(request.returnSceneId)),
      returnParameters_(std::move(request.returnParameters))
{
}

bool BattleArenaScene::AddBattleMove(BattleMove move)
{
    if (move.id.empty() || movesById_.contains(move.id))
    {
        return false;
    }
    movesById_.emplace(move.id, std::move(move));
    return true;
}

bool BattleArenaScene::OnGameObjectCreated(
    BaseGameObject& object,
    ObjectLayer& owner)
{
    static_cast<void>(owner);

    if (auto* player = dynamic_cast<PlayerBattle*>(&object);
        player != nullptr)
    {
        if (player_ != nullptr)
        {
            WriteDebugMessage("Battle arena has more than one playerBattle.");
            return false;
        }
        player_ = player;
        return player_->BindPlayerState(playerState_);
    }

    auto* enemy = dynamic_cast<BattleEnemy*>(&object);
    if (enemy == nullptr)
    {
        return true;
    }
    if (!enemiesById_.emplace(enemy->GetId(), enemy).second)
    {
        WriteDebugMessage("Battle enemy IDs must be unique: " + enemy->GetId());
        return false;
    }

    scriptedEnemies_.push_back(enemy);
    if (!requestedEnemyIds_.empty())
    {
        // A Lua encounter has final say over the map's direct-start defaults.
        enemy->SetActive(false);
    }
    return true;
}

bool BattleArenaScene::Init()
{
    if (player_ == nullptr || !BaseScene::Init())
    {
        return false;
    }

    std::vector<BattleEnemy*> activeEnemies;
    if (requestedEnemyIds_.empty())
    {
        for (BattleEnemy* enemy : scriptedEnemies_)
        {
            if (enemy->IsActive())
            {
                activeEnemies.push_back(enemy);
            }
        }
    }
    else
    {
        std::unordered_set<std::string> activatedIds;
        for (const std::string& id : requestedEnemyIds_)
        {
            const auto enemy = enemiesById_.find(id);
            if (enemy == enemiesById_.end()
                || !activatedIds.emplace(id).second)
            {
                WriteDebugMessage("Battle requested an unknown or duplicate enemy: " + id);
                return false;
            }
            enemy->second->SetActive(true);
            activeEnemies.push_back(enemy->second);
        }
    }

    if (activeEnemies.empty())
    {
        WriteDebugMessage("Battle arena requires at least one active enemy.");
        return false;
    }

    if (!playerState_.IsInitialized()
        || player_->GetStats().level != playerState_.GetLevel()
        || !experienceToNextLevelByLevel_.contains(
            playerState_.GetLevel()))
    {
        WriteDebugMessage(
            "Battle player does not match the game progression table.");
        return false;
    }

    encounterEnemies_ = activeEnemies;
    for (const BattleEnemy* enemy : encounterEnemies_)
    {
        for (const std::string& itemId : enemy->GetItemDropIds())
        {
            if (!itemDefinitions_.contains(itemId))
            {
                WriteDebugMessage(
                    "Battle enemy drops an unloaded item: " + itemId);
                return false;
            }
        }
    }

    if (!ValidateKnownMoves(player_->GetMoveIds(), player_->GetId())
        || FindFirstUsableAttack(
            player_->GetMoveIds(),
            player_->GetStats().mp) == nullptr)
    {
        WriteDebugMessage(
            "Battle player requires an affordable opponent-damage move.");
        return false;
    }
    for (const BattleEnemy* enemy : activeEnemies)
    {
        if (!ValidateKnownMoves(enemy->GetMoveIds(), enemy->GetId()))
        {
            WriteDebugMessage(
                "Battle enemy has invalid authored moves: "
                + enemy->GetId());
            return false;
        }
    }

    const SoundAsset* battleStart = resources_.GetSound("battleStart");
    if (battleStart == nullptr || !audio_.PlaySound(*battleStart))
    {
        WriteDebugMessage("Could not play the battle start sound.");
    }
    OpenPlayerTurn();
    return true;
}

void BattleArenaScene::Update(float deltaTime, const InputState& input)
{
    BaseScene::Update(deltaTime, input);
    if (phase_ == BattlePhase::Victory)
    {
        battleMenu_.Update(input);
        ProcessMenuFeedback();
        if (battleMenu_.ConsumeVictoryConfirmed()
            && !returnSceneId_.empty())
        {
            pendingSceneChange_ = SceneChangeRequest{
                "battleVictory",
                returnSceneId_,
                returnParameters_};
        }
        return;
    }

    if (phase_ == BattlePhase::PlayerInput)
    {
        battleMenu_.Update(input);
        ProcessMenuFeedback();
        if (battleMenu_.GetState() == BattleMenuState::TargetChosen)
        {
            const auto target = enemiesById_.find(
                battleMenu_.GetSelectedTargetId());
            const BattleMove* move = FindFirstUsableAttack(
                player_->GetMoveIds(),
                player_->GetStats().mp);
            if (target == enemiesById_.end()
                || !target->second->IsActive()
                || move == nullptr
                || !BeginPlayerAction(move->id, *target->second))
            {
                WriteDebugMessage("Could not begin the selected player action.");
                OpenPlayerTurn();
            }
        }
        return;
    }

    if (phase_ == BattlePhase::PlayerAction
        || phase_ == BattlePhase::EnemyAction)
    {
        UpdateActiveAction();
    }
}

void BattleArenaScene::Render(const Graphics& graphics)
{
    BaseScene::Render(graphics);
    battleMenu_.Render(graphics, resources_);
}

std::optional<SceneChangeRequest>
BattleArenaScene::TakeSceneChangeRequest()
{
    std::optional<SceneChangeRequest> request =
        std::move(pendingSceneChange_);
    pendingSceneChange_.reset();
    return request;
}

bool BattleArenaScene::BeginPlayerAction(
    const std::string& moveId,
    BattleEnemy& target)
{
    const BattleMove* move = FindMove(moveId);
    if (move == nullptr
        || move->effect != BattleMoveEffect::Damage
        || move->target != BattleMoveTarget::Opponent
        || !player_->KnowsMove(moveId)
        || player_->GetStats().mp < move->mpCost
        || !player_->PlayBattleAnimation(move->actorAnimation)
        || !player_->SpendMp(move->mpCost))
    {
        return false;
    }

    activeAction_ = ActiveAction{move, &target, false, false};
    phase_ = BattlePhase::PlayerAction;
    battleMenu_.SetBusy();
    return true;
}

bool BattleArenaScene::BeginEnemyAction(
    BattleEnemy& actor,
    const std::string& moveId,
    const std::string& targetId)
{
    const BattleMove* move = FindMove(moveId);
    if (move == nullptr
        || move->effect != BattleMoveEffect::Damage
        || move->target != BattleMoveTarget::Opponent
        || targetId != player_->GetId()
        || !actor.KnowsMove(moveId)
        || actor.GetStats().mp < move->mpCost
        || !actor.PlayBattleAnimation(move->actorAnimation)
        || !actor.SpendMp(move->mpCost))
    {
        return false;
    }

    activeAction_ = ActiveAction{move, &actor, false, false};
    phase_ = BattlePhase::EnemyAction;
    return true;
}

bool BattleArenaScene::ChooseEnemyAction(
    const BattleEnemy& actor,
    BattleActionIntent& intent) const
{
    BattleDecisionContext context;
    context.actor = MakeBattleSnapshot(actor);
    context.player = MakeBattleSnapshot(*player_);
    for (const BattleEnemy* enemy : GetActiveEnemies())
    {
        context.enemies.push_back(MakeBattleSnapshot(*enemy));
    }

    // Lua chooses meaning (move and target) from copied state. The returned
    // strings are only a proposal; BeginEnemyAction applies C++ legality,
    // resource, animation, and targeting rules before anything can happen.
    return luaSystem_.ChooseBattleAction(
        actor.GetBattleScript(),
        context,
        intent);
}

const BattleMove* BattleArenaScene::FindMove(
    const std::string& moveId) const
{
    const auto move = movesById_.find(moveId);
    return move != movesById_.end() ? &move->second : nullptr;
}

const BattleMove* BattleArenaScene::FindFirstUsableAttack(
    const std::vector<std::string>& moveIds,
    int availableMp) const
{
    // Attack is the only enabled command in this vertical slice. It resolves
    // through the combatant's authored move order instead of giving the scene
    // knowledge of a particular move such as "strike". A later menu or Lua
    // decision can pass a specific move ID into the same Begin*Action path.
    for (const std::string& moveId : moveIds)
    {
        const BattleMove* move = FindMove(moveId);
        if (move != nullptr
            && move->effect == BattleMoveEffect::Damage
            && move->target == BattleMoveTarget::Opponent
            && move->mpCost <= availableMp)
        {
            return move;
        }
    }
    return nullptr;
}

bool BattleArenaScene::ValidateKnownMoves(
    const std::vector<std::string>& moveIds,
    std::string_view ownerId) const
{
    for (const std::string& moveId : moveIds)
    {
        if (FindMove(moveId) == nullptr)
        {
            WriteDebugMessage(
                "Battle object " + std::string(ownerId)
                + " knows an unloaded move: " + moveId);
            return false;
        }
    }
    return true;
}

void BattleArenaScene::UpdateActiveAction()
{
    if (activeAction_.move == nullptr || activeAction_.enemy == nullptr)
    {
        OpenPlayerTurn();
        return;
    }

    const bool playerIsActing = phase_ == BattlePhase::PlayerAction;
    const std::size_t actorFrame = playerIsActing
        ? player_->GetAnimationFrameIndex()
        : activeAction_.enemy->GetAnimationFrameIndex();
    const bool actorFinished = playerIsActing
        ? player_->IsAnimationFinished()
        : activeAction_.enemy->IsAnimationFinished();

    // Crossing the authored impact frame applies mechanics exactly once. The
    // action remains busy afterward so both one-shot animations can finish.
    if (!activeAction_.impactApplied
        && (actorFrame >= activeAction_.move->impactFrame || actorFinished))
    {
        ApplyActiveActionImpact();
    }

    const bool targetFinished = playerIsActing
        ? !activeAction_.enemy->IsActive()
            || !activeAction_.targetAnimationStarted
            || activeAction_.enemy->IsAnimationFinished()
        : !activeAction_.targetAnimationStarted
            || player_->IsAnimationFinished();
    if (activeAction_.impactApplied && actorFinished && targetFinished)
    {
        FinishActiveAction();
    }
}

void BattleArenaScene::ApplyActiveActionImpact()
{
    activeAction_.impactApplied = true;
    if (!activeAction_.move->impactSoundId.empty())
    {
        const SoundAsset* impactSound =
            resources_.GetSound(activeAction_.move->impactSoundId);
        if (impactSound == nullptr || !audio_.PlaySound(*impactSound))
        {
            WriteDebugMessage(
                "Could not play battle move impact sound: "
                + activeAction_.move->impactSoundId);
        }
    }
    if (phase_ == BattlePhase::PlayerAction)
    {
        activeAction_.targetAnimationStarted =
            activeAction_.enemy->PlayBattleAnimation(
                activeAction_.move->targetAnimation);
        if (!activeAction_.targetAnimationStarted)
        {
            WriteDebugMessage(
                "Enemy sprite is missing battle reaction animation: "
                + activeAction_.move->targetAnimation);
        }
        activeAction_.enemy->ApplyDamage(CalculateBattleDamage(
            player_->GetStats(),
            activeAction_.enemy->GetStats(),
            activeAction_.move->power));
        return;
    }

    activeAction_.targetAnimationStarted = player_->PlayBattleAnimation(
        activeAction_.move->targetAnimation);
    if (!activeAction_.targetAnimationStarted)
    {
        WriteDebugMessage(
            "Player battle sprite is missing reaction animation: "
            + activeAction_.move->targetAnimation);
    }
    player_->ApplyDamage(CalculateBattleDamage(
        activeAction_.enemy->GetStats(),
        player_->GetStats(),
        activeAction_.move->power));
}

void BattleArenaScene::FinishActiveAction()
{
    const BattlePhase completedPhase = phase_;
    BattleEnemy* enemy = activeAction_.enemy;
    player_->ReturnToIdle();
    if (enemy->IsActive())
    {
        enemy->ReturnToIdle();
    }
    activeAction_ = {};

    if (player_->GetStats().hp == 0)
    {
        phase_ = BattlePhase::Defeat;
        battleMenu_.Configure(player_->GetStats(), {});
        battleMenu_.ShowDefeat();
        return;
    }

    if (GetActiveEnemies().empty())
    {
        phase_ = BattlePhase::Victory;
        battleMenu_.Configure(player_->GetStats(), {});
        if (!ShowVictoryRewards())
        {
            WriteDebugMessage("Could not apply battle victory rewards.");
        }
        return;
    }

    if (completedPhase == BattlePhase::PlayerAction && enemy->IsActive())
    {
        BattleActionIntent intent;
        if (!ChooseEnemyAction(*enemy, intent)
            || !BeginEnemyAction(
                *enemy,
                intent.moveId,
                intent.targetId))
        {
            WriteDebugMessage("Could not begin the enemy response action.");
            OpenPlayerTurn();
        }
        return;
    }

    OpenPlayerTurn();
}

bool BattleArenaScene::ShowVictoryRewards()
{
    int experienceGained = 0;
    std::vector<std::string> itemIds;
    std::vector<std::string> itemNames;
    for (const BattleEnemy* enemy : encounterEnemies_)
    {
        if (experienceGained
            > std::numeric_limits<int>::max()
                - enemy->GetExperienceReward())
        {
            return false;
        }
        experienceGained += enemy->GetExperienceReward();
        for (const std::string& itemId : enemy->GetItemDropIds())
        {
            const auto item = itemDefinitions_.find(itemId);
            if (item == itemDefinitions_.end())
            {
                return false;
            }
            itemIds.push_back(itemId);
            itemNames.push_back(item->second.name);
        }
    }

    if (!playerState_.AddExperience(experienceGained))
    {
        return false;
    }
    for (std::string& itemId : itemIds)
    {
        // Inventory is intentionally unique-item storage in this slice. A
        // duplicate reward is already owned, but still appears in the battle
        // summary because the encounter authored that drop.
        inventory_.AddItem(std::move(itemId));
    }
    battleMenu_.ShowVictory(
        experienceGained,
        playerState_.GetExperienceUntilNextLevel(
            experienceToNextLevelByLevel_.at(
                playerState_.GetLevel())),
        std::move(itemNames));
    const SoundAsset* victorySound = resources_.GetSound("battleVictory");
    if (victorySound == nullptr || !audio_.PlaySound(*victorySound))
    {
        WriteDebugMessage("Could not play the battle victory sound.");
    }
    return true;
}

void BattleArenaScene::OpenPlayerTurn()
{
    std::vector<BattleMenuTarget> targets;
    for (const BattleEnemy* enemy : GetActiveEnemies())
    {
        targets.push_back({enemy->GetId(), enemy->GetStats().name});
    }
    battleMenu_.Configure(player_->GetStats(), std::move(targets));
    phase_ = BattlePhase::PlayerInput;
}

void BattleArenaScene::ProcessMenuFeedback()
{
    const auto play = [this](const std::string& assetId)
    {
        const SoundAsset* sound = resources_.GetSound(assetId);
        if (sound == nullptr || !audio_.PlaySound(*sound))
        {
            WriteDebugMessage("Could not play sound asset: " + assetId);
        }
    };

    if (battleMenu_.ConsumeNavigationFeedback())
    {
        play("menuMove");
    }
    if (battleMenu_.ConsumeSelectionFeedback())
    {
        play("menuSelect");
    }
}

std::vector<BattleEnemy*> BattleArenaScene::GetActiveEnemies() const
{
    std::vector<BattleEnemy*> enemies;
    for (BattleEnemy* enemy : scriptedEnemies_)
    {
        if (enemy->IsActive())
        {
            enemies.push_back(enemy);
        }
    }
    return enemies;
}
