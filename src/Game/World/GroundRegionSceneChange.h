#pragma once

#include "../../Framework/Scenes/SceneChangeRequest.h"

#include <optional>

struct GroundRegion;

// Translates the game-specific meaning of a warp ground region into the
// framework's generic scene-transition request.
std::optional<SceneChangeRequest> BuildSceneChangeRequest(
    const GroundRegion& region);
