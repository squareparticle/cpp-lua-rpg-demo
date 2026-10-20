Say("You found the storage-room key and put it safely away.", nil, "keyPickup")
AddInventoryItem("storage_door_key")
SetTag("storageLantern_has_doorKey", true)
DestroyObject(CurrentObjectId)
