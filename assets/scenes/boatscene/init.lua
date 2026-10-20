local doorOpen = GetTag("storageLantern_door_open")
local hasLantern = GetTag("storageLantern_has_lantern")

if doorOpen then
    SetGroundBehavior("storageDoor", "warp")
    DestroyObject("storageDoorObj")
end

-- Mission progress skips this on boat returns and save loads. WorldScene
-- starts the queued cutscene before the first gameplay frame.
if not GetTag("mission_storageLantern")
    and not hasLantern
    and not GetTag("storageLantern_flames_active") then
    StartCutscene("assets/scenes/boatscene/opening.lua")
end

if GetTag("mission_storageLantern")
    and not doorOpen
    and not GetTag("storageLantern_has_doorKey") then
    SpawnObject("assets/scenes/boatscene/objects/key/storage_door_key.obj.yaml")
end

if hasLantern
    and not GetTag("storageLantern_fairies_released") then
    PlaySound("boatFire")
    DestroyObject("waterObj")
    SpawnObject("assets/scenes/boatscene/objects/water/lava.obj.yaml")
end

if GetTag("storageLantern_flames_active") then
    local flame = "assets/scenes/boatscene/objects/flames/flame.obj.yaml"
    SpawnObject(flame, { id = "flame1Obj", x = 570, y = 30 })
    SpawnObject(flame, { id = "flame2Obj", x = 775, y = 310 })
    SpawnObject(flame, { id = "flame3Obj", x = 500, y = 625 })
    SpawnObject(flame, { id = "flame4Obj", x = 745, y = 690 })
end
