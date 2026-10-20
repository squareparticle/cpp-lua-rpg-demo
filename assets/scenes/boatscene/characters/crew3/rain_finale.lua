-- Route geometry lives in map.yaml. Lua chooses every direction/animation;
-- MoveObjectTo only follows the named straight segment and waits for arrival.
playerInput = false
FollowObject(CurrentObjectId, 54, 74)

local route = {
    { point = "mageAisle", walk = "walkUp", idle = "idleUp" },
    { point = "mageAcrossDeck", walk = "walkUp", idle = "idleUp" },
    { point = "mageStairsBase", walk = "walkRight", idle = "idleRight" },
    { point = "mageStairsTop", walk = "walkUp", idle = "idleUp" },
    { point = "mageChestAisle", walk = "walkLeft", idle = "idleLeft" },
    { point = "mageChest", walk = "walkUp", idle = "idleUp" },
}

for _, leg in ipairs(route) do
    PlayAnimation(CurrentObjectId, leg.walk)
    MoveObjectTo(CurrentObjectId, leg.point, 150)
    PlayAnimation(CurrentObjectId, leg.idle)
end

PlaySound("chestUnlock")
Say("Got it!")
PlayAnimation(CurrentObjectId, "idleDown")
Say("Here we go!", nil, "rainMagic")
SetTag("weather=rain", true)
Wait(1.0)

if GetTag("storageLantern_flames_active") then
    local fires = {
        { object = "flame1Obj", focus = "fire1Focus" },
        { object = "flame2Obj", focus = "fire2Focus" },
        { object = "flame3Obj", focus = "fire3Focus" },
        { object = "flame4Obj", focus = "fire4Focus" },
    }
    for _, fire in ipairs(fires) do
        MoveCameraTo(fire.focus, 1.0)
        Wait(1.0)
        PlaySound("boatFire")
        FadeOutObject(fire.object, 0.35)
        DestroyObject(fire.object)
    end
end

SetTag("storageLantern_flames_active", false)
SetTag("shipFinale_complete", true)
Wait(1.0)
StopMusic()
FadeOutScene(1.5)
ChangeScene("ending")
