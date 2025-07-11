#include "otbtypes.h"
#include "item.h" // For ItemBase, which ServerItem methods will interact with

namespace OTB {

void ServerItem::updatePropertiesFromFlags() {
    unpassable = hasFlag(ServerItemFlag::Unpassable);
    blockMissiles = hasFlag(ServerItemFlag::BlockMissiles);
    blockPathfinder = hasFlag(ServerItemFlag::BlockPathfinder);
    hasElevation = hasFlag(ServerItemFlag::HasElevation);
    forceUse = hasFlag(ServerItemFlag::ForceUse);
    multiUse = hasFlag(ServerItemFlag::MultiUse);
    pickupable = hasFlag(ServerItemFlag::Pickupable);
    movable = hasFlag(ServerItemFlag::Movable); // Note: C# Item default is true, OTB flag might override
    stackable = hasFlag(ServerItemFlag::Stackable);
    // hasStackOrder is typically set if the StackOrder attribute is present,
    // and the ServerItemFlag::StackOrder flag should align.
    // If OTB has StackOrder flag but no attribute, it's an old/inconsistent format.
    // If attribute is present, this flag should be set.
    hasStackOrder = hasFlag(ServerItemFlag::StackOrder); // This flag means "has stack order attribute"
    readable = hasFlag(ServerItemFlag::Readable);
    rotatable = hasFlag(ServerItemFlag::Rotatable);
    hangable = hasFlag(ServerItemFlag::Hangable);
    hookSouth = hasFlag(ServerItemFlag::HookSouth);
    hookEast = hasFlag(ServerItemFlag::HookEast);
    allowDistanceRead = hasFlag(ServerItemFlag::AllowDistanceRead);
    hasCharges = hasFlag(ServerItemFlag::ClientCharges); // ClientCharges is the OTB flag name
    ignoreLook = hasFlag(ServerItemFlag::IgnoreLook);
    fullGround = hasFlag(ServerItemFlag::FullGround);
    isAnimation = hasFlag(ServerItemFlag::IsAnimation);
}

void ServerItem::updateFlagsFromProperties() {
    flags = 0; // Reset flags
    setFlag(ServerItemFlag::Unpassable, unpassable);
    setFlag(ServerItemFlag::BlockMissiles, blockMissiles);
    setFlag(ServerItemFlag::BlockPathfinder, blockPathfinder);
    setFlag(ServerItemFlag::HasElevation, hasElevation);
    setFlag(ServerItemFlag::ForceUse, forceUse);
    setFlag(ServerItemFlag::MultiUse, multiUse);
    setFlag(ServerItemFlag::Pickupable, pickupable);
    setFlag(ServerItemFlag::Movable, movable);
    setFlag(ServerItemFlag::Stackable, stackable);
    // ServerItemFlag::StackOrder should be set if 'stackOrder' attribute is actually written
    // This boolean 'hasStackOrder' (if true) implies the flag should be set.
    setFlag(ServerItemFlag::StackOrder, hasStackOrder);
    setFlag(ServerItemFlag::Readable, readable);
    setFlag(ServerItemFlag::Rotatable, rotatable);
    setFlag(ServerItemFlag::Hangable, hangable);
    setFlag(ServerItemFlag::HookSouth, hookSouth);
    setFlag(ServerItemFlag::HookEast, hookEast);
    setFlag(ServerItemFlag::AllowDistanceRead, allowDistanceRead);
    setFlag(ServerItemFlag::ClientCharges, hasCharges);
    setFlag(ServerItemFlag::IgnoreLook, ignoreLook);
    setFlag(ServerItemFlag::FullGround, fullGround);
    setFlag(ServerItemFlag::IsAnimation, isAnimation);
}

bool ServerItem::equals(const ItemBase& other) const {
    // Compare all common properties defined in ItemBase
    if (this->type != other.type ||
        this->stackOrder != other.stackOrder || // Compare actual stack order value
        this->unpassable != other.unpassable ||
        this.blockMissiles != other.blockMissiles ||
        this->blockPathfinder != other.blockPathfinder ||
        this.hasElevation != other.hasElevation ||
        this->forceUse != other.forceUse ||
        this->multiUse != other.multiUse ||
        this.pickupable != other.pickupable ||
        this->movable != other.movable ||
        this->stackable != other.stackable ||
        this.readable != other.readable ||
        this->rotatable != other.rotatable ||
        this->hangable != other.hangable ||
        this->hookSouth != other.hookSouth ||
        this->hookEast != other.hookEast ||
        this->ignoreLook != other.ignoreLook ||
        this->fullGround != other.fullGround ||
        this->isAnimation != other.isAnimation || // isAnimation from flags
        this->allowDistanceRead != other.allowDistanceRead ||
        this.hasCharges != other.hasCharges ||
        this.groundSpeed != other.groundSpeed ||
        this.lightLevel != other.lightLevel ||
        this.lightColor != other.lightColor ||
        this->maxReadChars != other.maxReadChars ||
        this->maxReadWriteChars != other.maxReadWriteChars ||
        this->minimapColor != other.minimapColor ||
        this->tradeAs != other.tradeAs ||
        this.name != other.name) {
        return false;
    }
    // ServerItem specific comparisons (if any beyond ItemBase) could go here.
    // Note: SpriteHash comparison is often done separately (e.g. C# CompareItem(item, compareHash))
    return true;
}

void ServerItem::copyPropertiesFrom(const ItemBase& source) {
    // Copy all common properties from ItemBase
    // Note: ServerItem's 'id' is its ServerID, source.id could be ClientID if source is ClientItem
    // This method is primarily for ServerItem <- ClientItem or ServerItem <- ServerItem (via ItemBase)
    // Be careful about ID semantics if source is a ServerItem.
    // For ServerItem <- ClientItem, C# logic is:
    //   ushort tmpId = item.ID; (stores original server ID)
    //   item.CopyPropertiesFrom(clientItem);
    //   item.ID = tmpId; (restores original server ID)
    //   item.ClientId = clientItem.ID; (sets client ID)

    this->name = source.name;
    this->type = source.type;
    this->stackOrder = source.stackOrder;
    this->hasStackOrder = source.hasStackOrder; // Ensure this is set if source.stackOrder is not None

    this->unpassable = source.unpassable;
    this->blockMissiles = source.blockMissiles;
    this->blockPathfinder = source.blockPathfinder;
    this->hasElevation = source.hasElevation;
    this->forceUse = source.forceUse;
    this->multiUse = source.multiUse;
    this->pickupable = source.pickupable;
    this->movable = source.movable;
    this->stackable = source.stackable;
    this->readable = source.readable;
    this->rotatable = source.rotatable;
    this.hangable = source.hangable;
    this->hookSouth = source.hookSouth;
    this->hookEast = source.hookEast;
    this->allowDistanceRead = source.allowDistanceRead;
    this->hasCharges = source.hasCharges;
    this->ignoreLook = source.ignoreLook;
    this->fullGround = source.fullGround;
    this->isAnimation = source.isAnimation;

    this->groundSpeed = source.groundSpeed;
    this->lightLevel = source.lightLevel;
    this->lightColor = source.lightColor;
    this->maxReadChars = source.maxReadChars;
    this->maxReadWriteChars = source.maxReadWriteChars;
    this->minimapColor = source.minimapColor;
    this->tradeAs = source.tradeAs;

    // SpriteHash is usually handled separately or explicitly copied if needed.
    // C# Item.CopyPropertiesFrom explicitly skips SpriteHash.
    // if (source.spriteHash.size() == 16) {
    //    this->spriteHash = source.spriteHash;
    // }

    // After copying boolean properties, update the internal flags field
    this->updateFlagsFromProperties();
}


} // namespace OTB
