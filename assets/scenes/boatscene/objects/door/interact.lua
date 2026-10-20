if not HasInventoryItem("storage_door_key") then
    PlaySound("storageDoorLocked")
    Say("The storage-room door is locked.")
    return
end

Say("The key turns with a heavy click. The storage-room door swings open.", nil, "storageDoorOpen")
RemoveInventoryItem("storage_door_key")
SetTag("storageLantern_door_open", true)
SetGroundBehavior("storageDoor", "warp")
DestroyObject(CurrentObjectId)
