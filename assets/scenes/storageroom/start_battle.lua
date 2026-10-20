if not GetTag("storageBattle_triggered") then
    SetTag("storageBattle_triggered", true)
    SetGroundBehavior("storageBattle", "walkable")
    StartBattle({ "goblinSword", "goblinClub" })
end
