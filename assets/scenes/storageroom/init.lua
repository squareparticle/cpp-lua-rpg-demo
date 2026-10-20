if GetTag("storageBattle_triggered") then
    SetGroundBehavior("storageBattle", "walkable")
end

if GetTag("storageLantern_has_lantern") then
    DestroyObject("guidingLanternObj")
end
