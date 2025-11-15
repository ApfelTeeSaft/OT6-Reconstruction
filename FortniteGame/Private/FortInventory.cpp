// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite Inventory Implementation

#include "FortInventory.h"
#include "FortWeapon.h"
#include "Net/UnrealNetwork.h"

//////////////////////////////////////////////////////////////////////////
// UFortInventory

UFortInventory::UFortInventory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, BaseInventorySize(20)
	, InventorySizeBonus(0)
	, WorldInventorySizeBonus(0)
	, bInventoryOpen(false)
{
	// Source: line 145906 - FortInventory.cpp
	// Source: line 145302 - "LogFortInventory"

	SetIsReplicatedByDefault(true);
}

void UFortInventory::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogNet, Log, TEXT("UFortInventory: BeginPlay - Total size: %d"), GetTotalInventorySize());
}

bool UFortInventory::AddItem(FName ItemDefinitionName, int32 Count, int32 Level)
{
	if (!GetOwner() || GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	// Check if inventory is full
	// Source: line 598531 - "InventoryFull"
	if (IsInventoryFull())
	{
		UE_LOG(LogNet, Warning, TEXT("UFortInventory: Cannot add item - InventoryFull"));
		return false;
	}

	// Try to stack with existing item
	TArray<FFortItemEntry> ExistingItems = FindItemsByDefinition(ItemDefinitionName);
	if (ExistingItems.Num() > 0)
	{
		// Stack with first matching item
		for (FFortItemEntry& Item : Items)
		{
			if (Item.ItemDefinitionName == ItemDefinitionName)
			{
				Item.Count += Count;
				OnItemUpdated.Broadcast(Item.ItemGuid);
				return true;
			}
		}
	}

	// Add new item
	FFortItemEntry NewItem;
	NewItem.ItemGuid = FFortItemGuid();
	NewItem.ItemDefinitionName = ItemDefinitionName;
	NewItem.Count = Count;
	NewItem.Level = Level;

	Items.Add(NewItem);

	UE_LOG(LogNet, Log, TEXT("UFortInventory: Added item - %s, Count: %d, Level: %d"),
		*ItemDefinitionName.ToString(), Count, Level);

	return true;
}

bool UFortInventory::RemoveItem(const FFortItemGuid& ItemGuid, int32 Count)
{
	if (!GetOwner() || GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	FFortItemEntry* Item = FindItemByGuid(ItemGuid);
	if (!Item)
	{
		UE_LOG(LogNet, Warning, TEXT("UFortInventory: Cannot remove item - Item not found"));
		return false;
	}

	if (Item->Count <= Count)
	{
		// Remove entire stack
		for (int32 i = Items.Num() - 1; i >= 0; --i)
		{
			if (Items[i].ItemGuid == ItemGuid)
			{
				Items.RemoveAt(i);
				OnItemDestroyed.Broadcast(ItemGuid);
				break;
			}
		}
	}
	else
	{
		// Reduce count
		Item->Count -= Count;
		OnItemUpdated.Broadcast(ItemGuid);
	}

	UE_LOG(LogNet, Log, TEXT("UFortInventory: Removed item - Count: %d"), Count);

	return true;
}

FFortItemEntry* UFortInventory::FindItemByGuid(const FFortItemGuid& ItemGuid)
{
	// Source: line 145938 - "AFortQuickBars::ServerAddItemInternal could not find Item with GUID %s"

	for (FFortItemEntry& Item : Items)
	{
		if (Item.ItemGuid == ItemGuid)
		{
			return &Item;
		}
	}

	return nullptr;
}

TArray<FFortItemEntry> UFortInventory::FindItemsByDefinition(FName ItemDefinitionName)
{
	TArray<FFortItemEntry> FoundItems;

	for (const FFortItemEntry& Item : Items)
	{
		if (Item.ItemDefinitionName == ItemDefinitionName)
		{
			FoundItems.Add(Item);
		}
	}

	return FoundItems;
}

bool UFortInventory::IsInventoryFull() const
{
	// Source: line 598531 - "InventoryFull"
	return Items.Num() >= GetTotalInventorySize();
}

int32 UFortInventory::GetTotalInventorySize() const
{
	// Source: line 141543 - "InventorySizeBonus"
	// Source: line 141548 - "WorldInventorySizeBonus"
	return BaseInventorySize + InventorySizeBonus + WorldInventorySizeBonus;
}

void UFortInventory::ToggleInventory()
{
	// Source: line 149552 - "ToggleInventory"
	bInventoryOpen = !bInventoryOpen;

	UE_LOG(LogNet, Log, TEXT("UFortInventory: ToggleInventory - Open: %d"), bInventoryOpen);
}

void UFortInventory::OnFortItemDestroyed(const FFortItemGuid& ItemGuid)
{
	// Source: line 681070 - "OnFortItemDestroyed"
	UE_LOG(LogNet, Log, TEXT("UFortInventory: OnFortItemDestroyed"));
	OnItemDestroyed.Broadcast(ItemGuid);
}

void UFortInventory::OnFortItemInvalidated(const FFortItemGuid& ItemGuid)
{
	// Source: line 681072 - "OnFortItemInvalidated"
	UE_LOG(LogNet, Log, TEXT("UFortInventory: OnFortItemInvalidated"));
	OnItemInvalidated.Broadcast(ItemGuid);
}

void UFortInventory::OnFortItemUpdated(const FFortItemGuid& ItemGuid)
{
	// Source: line 681074 - "OnFortItemUpdated"
	UE_LOG(LogNet, Verbose, TEXT("UFortInventory: OnFortItemUpdated"));
	OnItemUpdated.Broadcast(ItemGuid);
}

void UFortInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UFortInventory, Items);
	DOREPLIFETIME(UFortInventory, InventorySizeBonus);
	DOREPLIFETIME(UFortInventory, WorldInventorySizeBonus);
	DOREPLIFETIME(UFortInventory, bInventoryOpen);
}

//////////////////////////////////////////////////////////////////////////
// AFortQuickBars

AFortQuickBars::AFortQuickBars(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, PrimaryQuickBarSize(6)
	, SecondaryQuickBarSize(3)
	, CurrentPrimarySlot(0)
	, CurrentQuickBar(EFortQuickBars::Primary)
{
	// Source: line 145421 - FortQuickBars.cpp
	// Source: line 145826 - "PrimaryQuickBar"
	// Source: line 145828 - "SecondaryQuickBar"
	// Source: line 149666 - "QuickBars"

	bReplicates = true;
	bAlwaysRelevant = true;

	// Initialize quickbar arrays
	PrimaryQuickBarSlots.SetNum(PrimaryQuickBarSize);
	SecondaryQuickBarSlots.SetNum(SecondaryQuickBarSize);
}

void AFortQuickBars::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogNet, Log, TEXT("AFortQuickBars: BeginPlay - Primary: %d slots, Secondary: %d slots"),
		PrimaryQuickBarSize, SecondaryQuickBarSize);
}

void AFortQuickBars::ServerAddItemInternal_Implementation(const FFortItemGuid& ItemGuid, int32 SlotIndex, EFortQuickBars QuickBarType)
{
	// Source: line 145421 - "AFortQuickBars::ServerAddItemInternal did not add item '%s' with definition '%s' because it was gifted"
	// Source: line 145434 - "Bad call to AFortQuickBars::ServerAddItemInternal with slot %d!"
	// Source: line 145458 - "AFortQuickBars::ServerAddItemInternal did not add slot-required item..."
	// Source: line 145938 - "AFortQuickBars::ServerAddItemInternal could not find Item with GUID %s"
	// Source: line 145945 - "AFortQuickBars::ServerAddItemInternal did not add item '%s' with definition '%s' because it does not have the RequiredEquipTags"
	// Source: line 145958 - "AFortQuickBars::ServerAddItemInternal did not add item '%s' with definition '%s' because it was in the backpack overflow"

	if (!ValidateSlotIndex(SlotIndex, QuickBarType))
	{
		UE_LOG(LogNet, Error, TEXT("Bad call to AFortQuickBars::ServerAddItemInternal with slot %d!"), SlotIndex);
		return;
	}

	if (!ItemGuid.IsValid())
	{
		UE_LOG(LogNet, Error, TEXT("AFortQuickBars::ServerAddItemInternal could not find Item with GUID (invalid)"));
		return;
	}

	if (!CanAddItemToSlot(ItemGuid, SlotIndex, QuickBarType))
	{
		UE_LOG(LogNet, Warning, TEXT("AFortQuickBars::ServerAddItemInternal did not add item - Cannot add to slot %d"), SlotIndex);
		return;
	}

	// Add to appropriate quickbar
	if (QuickBarType == EFortQuickBars::Primary)
	{
		PrimaryQuickBarSlots[SlotIndex] = ItemGuid;
	}
	else if (QuickBarType == EFortQuickBars::Secondary)
	{
		SecondaryQuickBarSlots[SlotIndex] = ItemGuid;
	}

	UE_LOG(LogNet, Log, TEXT("AFortQuickBars: Added item to slot %d on %s quickbar"),
		SlotIndex, QuickBarType == EFortQuickBars::Primary ? TEXT("Primary") : TEXT("Secondary"));
}

bool AFortQuickBars::ServerAddItemInternal_Validate(const FFortItemGuid& ItemGuid, int32 SlotIndex, EFortQuickBars QuickBarType)
{
	return true;
}

void AFortQuickBars::ServerSwapItems_Implementation(int32 SourceSlotIndex, EFortQuickBars SourceQuickBar, int32 DestSlotIndex, EFortQuickBars DestQuickBar)
{
	// Source: line 145876 - "Bad call to AFortQuickBars::SwapItems with index %d %d!"

	if (!ValidateSlotIndex(SourceSlotIndex, SourceQuickBar) || !ValidateSlotIndex(DestSlotIndex, DestQuickBar))
	{
		UE_LOG(LogNet, Error, TEXT("Bad call to AFortQuickBars::SwapItems with index %d %d!"), SourceSlotIndex, DestSlotIndex);
		return;
	}

	// Get source and dest arrays
	TArray<FFortItemGuid>* SourceArray = (SourceQuickBar == EFortQuickBars::Primary) ? &PrimaryQuickBarSlots : &SecondaryQuickBarSlots;
	TArray<FFortItemGuid>* DestArray = (DestQuickBar == EFortQuickBars::Primary) ? &PrimaryQuickBarSlots : &SecondaryQuickBarSlots;

	// Swap items
	FFortItemGuid TempGuid = (*SourceArray)[SourceSlotIndex];
	(*SourceArray)[SourceSlotIndex] = (*DestArray)[DestSlotIndex];
	(*DestArray)[DestSlotIndex] = TempGuid;

	UE_LOG(LogNet, Log, TEXT("AFortQuickBars: Swapped items - Source: %d, Dest: %d"), SourceSlotIndex, DestSlotIndex);
}

bool AFortQuickBars::ServerSwapItems_Validate(int32 SourceSlotIndex, EFortQuickBars SourceQuickBar, int32 DestSlotIndex, EFortQuickBars DestQuickBar)
{
	return true;
}

void AFortQuickBars::ServerRemoveItem_Implementation(int32 SlotIndex, EFortQuickBars QuickBarType)
{
	if (!ValidateSlotIndex(SlotIndex, QuickBarType))
	{
		return;
	}

	// Clear slot
	if (QuickBarType == EFortQuickBars::Primary)
	{
		PrimaryQuickBarSlots[SlotIndex] = FFortItemGuid();
	}
	else if (QuickBarType == EFortQuickBars::Secondary)
	{
		SecondaryQuickBarSlots[SlotIndex] = FFortItemGuid();
	}

	UE_LOG(LogNet, Log, TEXT("AFortQuickBars: Removed item from slot %d"), SlotIndex);
}

bool AFortQuickBars::ServerRemoveItem_Validate(int32 SlotIndex, EFortQuickBars QuickBarType)
{
	return true;
}

FFortItemEntry* AFortQuickBars::GetItemAtSlot(int32 SlotIndex, EFortQuickBars QuickBarType)
{
	if (!ValidateSlotIndex(SlotIndex, QuickBarType))
	{
		return nullptr;
	}

	FFortItemGuid ItemGuid;

	if (QuickBarType == EFortQuickBars::Primary)
	{
		ItemGuid = PrimaryQuickBarSlots[SlotIndex];
	}
	else if (QuickBarType == EFortQuickBars::Secondary)
	{
		ItemGuid = SecondaryQuickBarSlots[SlotIndex];
	}

	// Would look up item from inventory using ItemGuid
	// For now return nullptr
	return nullptr;
}

bool AFortQuickBars::ValidateSlotIndex(int32 SlotIndex, EFortQuickBars QuickBarType) const
{
	if (QuickBarType == EFortQuickBars::Primary)
	{
		return SlotIndex >= 0 && SlotIndex < PrimaryQuickBarSize;
	}
	else if (QuickBarType == EFortQuickBars::Secondary)
	{
		return SlotIndex >= 0 && SlotIndex < SecondaryQuickBarSize;
	}

	return false;
}

bool AFortQuickBars::CanAddItemToSlot(const FFortItemGuid& ItemGuid, int32 SlotIndex, EFortQuickBars QuickBarType) const
{
	// Check various conditions
	// - Item has required equip tags (line 145945)
	// - Item is not in backpack overflow (line 145958)
	// - Item is not gifted (line 145421)
	// - Slot is not occupied or can be replaced

	return true;  // Simplified for now
}

void AFortQuickBars::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFortQuickBars, PrimaryQuickBarSlots);
	DOREPLIFETIME(AFortQuickBars, SecondaryQuickBarSlots);
	DOREPLIFETIME(AFortQuickBars, CurrentPrimarySlot);
	DOREPLIFETIME(AFortQuickBars, CurrentQuickBar);
}

//////////////////////////////////////////////////////////////////////////
// UFortItemDefinition

UFortItemDefinition::UFortItemDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ItemType(EFortItemType::WorldItem)
	, Icon(nullptr)
	, MaxStackSize(1)
	, bCanBeDropped(true)
	, bCanBeDestroyed(true)
{
}

//////////////////////////////////////////////////////////////////////////
// UFortWeaponItemDefinition

UFortWeaponItemDefinition::UFortWeaponItemDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, WeaponActorClass(nullptr)
{
	// Source: line 142267 - "UFortWeaponItemDefinition::GetWeaponStats"
	// Source: line 152680 - "UFortWeaponItemDefinition"
	// File: FortWeaponItemDefinition.cpp (line 151207)

	ItemType = EFortItemType::Weapon;
	MaxStackSize = 1;  // Weapons don't stack
}

FWeaponStats UFortWeaponItemDefinition::GetWeaponStats() const
{
	// Source: line 142267 - "UFortWeaponItemDefinition::GetWeaponStats"
	// Confidence: HIGH - Exact function name (42 characters)

	return WeaponStats;
}

//////////////////////////////////////////////////////////////////////////
// UFortWeaponRangedItemDefinition

UFortWeaponRangedItemDefinition::UFortWeaponRangedItemDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, BulletSpread(0.0f)
	, BulletsPerShot(1)
	, ProjectileClass(nullptr)
{
	// Source: line 159975 - "UFortWeaponRangedItemDefinition"

	ItemType = EFortItemType::WeaponRanged;
	WeaponActorClass = AFortWeaponRanged::StaticClass();
}

//////////////////////////////////////////////////////////////////////////
// UFortWeaponMeleeItemDefinition

UFortWeaponMeleeItemDefinition::UFortWeaponMeleeItemDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SwingRadius(200.0f)
	, SwingAngle(90.0f)
	, bCanHitMultiple(true)
{
	// Source: line 158431 - "UFortWeaponMeleeItemDefinition"

	ItemType = EFortItemType::WeaponMelee;
	WeaponActorClass = AFortWeaponMelee::StaticClass();
}
