assert(CurrentObjectId == nil)

SetTag("scene_init_ran", true)
SetGroundBehavior("storageDoor", "walkable")
SpawnObject("assets/scenes/boatscene/objects/key/storage_door_key.obj.yaml")
SetGroundParams("storageDoor", {
    destination = "storageRoom",
    entrance = "mainDoor",
})
