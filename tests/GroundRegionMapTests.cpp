#include "../src/Game/World/GroundRegionMap.h"

#include <iostream>
#include <string>

namespace
{
bool Expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << message << '\n';
        return false;
    }
    return true;
}

template<typename Value>
const Value* FindParameter(
    const GroundRegion& region,
    const std::string& name)
{
    const auto parameter = region.parameters.find(name);
    return parameter != region.parameters.end()
        ? std::get_if<Value>(&parameter->second)
        : nullptr;
}
}

int main()
{
    GroundParameterMap doorParameters{
        {"destination", std::string("storageRoom")},
        {"entrance", std::string("mainDoor")},
        {"transitionSeconds", 0.25},
        {"fade", true}};

    GroundRegionMap regions;
    if (!Expect(regions.AddRegion(
            "storageDoor",
            0xFF0000,
            GroundBehavior::Collision,
            doorParameters),
            "Could not add the storage-door region")
        || !Expect(regions.BlocksMovement(0xFF0000),
            "Collision region did not block movement")
        || !Expect(!regions.BlocksMovement(0x00FF00),
            "Unknown color unexpectedly blocked movement")
        || !Expect(!regions.AddRegion(
                "storageDoor",
                0x00FF00,
                GroundBehavior::Walkable),
            "Duplicate region ID was accepted")
        || !Expect(!regions.AddRegion(
                "otherDoor",
                0xFF0000,
                GroundBehavior::Walkable),
            "Duplicate region color was accepted")
        || !Expect(!regions.AddRegion(
                "invalidWarp",
                0x00FF00,
                GroundBehavior::Warp),
            "Warp behavior was accepted without a destination parameter")
        || !Expect(!regions.AddRegion(
                "invalidTrigger",
                0x00FF00,
                GroundBehavior::Trigger),
            "Trigger behavior was accepted without a hook")
        || !Expect(regions.AddRegion(
                "storageBattle",
                0x00FF00,
                GroundBehavior::Trigger,
                {},
                "assets/scenes/storageroom/start_battle.lua"),
            "Could not add a trigger region"))
    {
        return 1;
    }

    const GroundRegion* byId = regions.FindById("storageDoor");
    const GroundRegion* byColor = regions.FindByColor(0xFF0000);
    const GroundRegion* trigger = regions.FindById("storageBattle");
    if (!Expect(byId != nullptr && byColor == byId,
            "Region lookup did not preserve identity")
        || !Expect(trigger != nullptr && !trigger->hook.empty(),
            "Trigger region did not retain its Lua hook")
        || !Expect(FindParameter<std::string>(*byId, "destination") != nullptr
                && *FindParameter<std::string>(*byId, "destination")
                    == "storageRoom",
            "Region did not retain its string parameter")
        || !Expect(FindParameter<double>(*byId, "transitionSeconds") != nullptr
                && *FindParameter<double>(*byId, "transitionSeconds") == 0.25,
            "Region did not retain its numeric parameter")
        || !Expect(FindParameter<bool>(*byId, "fade") != nullptr
                && *FindParameter<bool>(*byId, "fade"),
            "Region did not retain its Boolean parameter")
        || !Expect(regions.SetBehavior(
                "storageDoor",
                GroundBehavior::Warp),
            "Could not change the region to warp")
        || !Expect(!regions.BlocksMovement(0xFF0000),
            "Warp region still blocked movement"))
    {
        return 1;
    }

    byId = regions.FindById("storageDoor");
    if (!Expect(byId != nullptr
                && FindParameter<std::string>(*byId, "entrance") != nullptr,
            "Changing behavior erased the region parameters")
        || !Expect(!regions.SetParameters(
                "storageDoor",
                GroundParameterMap{{"amount", 10.0}}),
            "Warp accepted parameters without a string destination"))
    {
        return 1;
    }

    byId = regions.FindById("storageDoor");
    if (!Expect(byId != nullptr
                && FindParameter<std::string>(*byId, "destination") != nullptr,
            "Rejected parameter replacement altered the region")
        || !Expect(regions.SetBehavior(
                "storageDoor",
                GroundBehavior::Walkable),
            "Could not change the region to walkable")
        || !Expect(regions.SetParameters(
                "storageDoor",
                GroundParameterMap{
                    {"amount", 10.0},
                    {"damageType", std::string("fire")}}),
            "Walkable region rejected passive parameters")
        || !Expect(!regions.SetBehavior(
                "storageDoor",
                GroundBehavior::Warp),
            "Warp accepted a parameter map without destination")
        || !Expect(!regions.SetParameters(
                "missing",
                GroundParameterMap{}),
            "Unknown region accepted parameters")
        || !Expect(!regions.SetBehavior(
                "missing",
                GroundBehavior::Collision),
            "Unknown region accepted a behavior change"))
    {
        return 1;
    }

    byId = regions.FindById("storageDoor");
    return Expect(byId != nullptr
            && FindParameter<double>(*byId, "amount") != nullptr
            && *FindParameter<double>(*byId, "amount") == 10.0,
        "Stored parameters were not replaced")
        ? 0
        : 1;
}
