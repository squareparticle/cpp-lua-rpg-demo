if CurrentSceneId == "boatScene"
    and CurrentGroundRegionId == "lanternRelease"
    and not GetTag("storageLantern_fairies_released") then
    playerInput = false
    Say("Fairies released", nil, "lanternRelease")
    SetTag("storageLantern_fairies_released", true)
    SetTag("weather=fairies", true)
    SpawnObject("assets/scenes/boatscene/objects/water/water.obj.yaml")
    FadeOutObject("lavaObj", 2.0)
    DestroyObject("lavaObj")
    FadeOutWeather(0.5)
    playerInput = true
else
    Say("Nothing happened...")
end
