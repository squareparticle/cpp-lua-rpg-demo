-- Scene-owned opening: crew duties and player interaction are independent.
playerInput = false
SetObjectPosition("crew1", "crew1StairsTop")
PlayAnimation("crew1", "walkDown")
MoveObjectTo("crew1", "crew1Watch", 150)
PlayAnimation("crew1", "idleLeft")
dofile("assets/scenes/boatscene/characters/crew1/dialog_start_mission.lua")
playerInput = true
