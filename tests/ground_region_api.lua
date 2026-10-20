SetGroundBehavior("storageDoor", "walkable")
SetGroundParams("storageDoor", {
    destination = "storageRoom",
    entrance = "mainDoor",
    transitionSeconds = 0.25,
    fade = true,
})
SetGroundBehavior("storageDoor", "warp")

assert(not pcall(function()
    SetGroundBehavior("", "collision")
end))
assert(not pcall(function()
    SetGroundBehavior("storageDoor", "unknown")
end))
assert(not pcall(function()
    SetGroundParams("", {})
end))
assert(not pcall(function()
    SetGroundParams("storageDoor", "not a table")
end))
assert(not pcall(function()
    SetGroundParams("storageDoor", { "numeric key" })
end))
assert(not pcall(function()
    SetGroundParams("storageDoor", { nested = {} })
end))
assert(not pcall(function()
    SetGroundParams("storageDoor", { callback = function() end })
end))
assert(not pcall(function()
    SetGroundParams("storageDoor", { amount = math.huge })
end))
