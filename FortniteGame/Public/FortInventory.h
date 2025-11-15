// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite Inventory and Item Systems
// Source: D:\BuildFarm\buildmachine_++Fortnite+Release-Live\FortniteGame\Source\FortniteGame\Private\Items\FortInventory.cpp

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FortEnums.h"
#include "FortInventory.generated.h"

/**
 * FFortItemGuid - Unique identifier for items
 */
USTRUCT(BlueprintType)
struct FFortItemGuid
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGuid Guid;

	FFortItemGuid()
	{
		Guid = FGuid::NewGuid();
	}

	bool IsValid() const
	{
		return Guid.IsValid();
	}

	bool operator==(const FFortItemGuid& Other) const
	{
		return Guid == Other.Guid;
	}
};

/**
 * FFortItemEntry - Single item in inventory
 */
USTRUCT(BlueprintType)
struct FFortItemEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FFortItemGuid ItemGuid;

	UPROPERTY(BlueprintReadOnly)
	FName ItemDefinitionName;

	UPROPERTY(BlueprintReadOnly)
	int32 Count;

	UPROPERTY(BlueprintReadOnly)
	int32 Level;

	UPROPERTY(BlueprintReadOnly)
	float Durability;

	UPROPERTY(BlueprintReadOnly)
	int32 LoadedAmmo;

	FFortItemEntry()
		: Count(1)
		, Level(1)
		, Durability(100.0f)
		, LoadedAmmo(0)
	{}
};

/**
 * UFortInventory - Inventory component
 */
UCLASS(BlueprintType)
class FORTNITEGAME_API UFortInventory : public UActorComponent
{
	GENERATED_BODY()

public:
	UFortInventory(const FObjectInitializer& ObjectInitializer);

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End UActorComponent Interface

	/**
	 * Add item to inventory
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Inventory")
	bool AddItem(FName ItemDefinitionName, int32 Count = 1, int32 Level = 1);

	/**
	 * Remove item from inventory
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Inventory")
	bool RemoveItem(const FFortItemGuid& ItemGuid, int32 Count = 1);

	/**
	 * Find item by GUID
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Inventory")
	FFortItemEntry* FindItemByGuid(const FFortItemGuid& ItemGuid);

	/**
	 * Find items by definition name
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Inventory")
	TArray<FFortItemEntry> FindItemsByDefinition(FName ItemDefinitionName);

	/**
	 * Check if inventory is full
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Inventory")
	bool IsInventoryFull() const;

	/**
	 * Get inventory size with bonus
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Inventory")
	int32 GetTotalInventorySize() const;

	/**
	 * Toggle inventory display
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Inventory")
	void ToggleInventory();

	/** Inventory items */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Inventory")
	TArray<FFortItemEntry> Items;

	/** Inventory configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Inventory")
	int32 BaseInventorySize;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Inventory")
	int32 InventorySizeBonus;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Inventory")
	int32 WorldInventorySizeBonus;

	/** Inventory state */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Inventory")
	bool bInventoryOpen;

protected:
	/**
	 * Callback when item is destroyed
	 */
	UFUNCTION()
	void OnFortItemDestroyed(const FFortItemGuid& ItemGuid);

	/**
	 * Callback when item is invalidated
	 */
	UFUNCTION()
	void OnFortItemInvalidated(const FFortItemGuid& ItemGuid);

	/**
	 * Callback when item is updated
	 */
	UFUNCTION()
	void OnFortItemUpdated(const FFortItemGuid& ItemGuid);

public:
	/** Delegates */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFortItemEvent, const FFortItemGuid&, ItemGuid);

	UPROPERTY(BlueprintAssignable, Category = "Fort|Inventory")
	FOnFortItemEvent OnItemDestroyed;

	UPROPERTY(BlueprintAssignable, Category = "Fort|Inventory")
	FOnFortItemEvent OnItemInvalidated;

	UPROPERTY(BlueprintAssignable, Category = "Fort|Inventory")
	FOnFortItemEvent OnItemUpdated;
};

/**
 * AFortQuickBars - Quick bar system for hotbar slots
 */
UCLASS()
class FORTNITEGAME_API AFortQuickBars : public AActor
{
	GENERATED_BODY()

public:
	AFortQuickBars(const FObjectInitializer& ObjectInitializer);

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End AActor Interface

	/**
	 * Add item to quickbar internally (server)
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAddItemInternal(const FFortItemGuid& ItemGuid, int32 SlotIndex, EFortQuickBars QuickBarType);

	/**
	 * Swap items between slots
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSwapItems(int32 SourceSlotIndex, EFortQuickBars SourceQuickBar, int32 DestSlotIndex, EFortQuickBars DestQuickBar);

	/**
	 * Remove item from quickbar
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRemoveItem(int32 SlotIndex, EFortQuickBars QuickBarType);

	/**
	 * Get item at slot
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|QuickBars")
	FFortItemEntry* GetItemAtSlot(int32 SlotIndex, EFortQuickBars QuickBarType);

	/** Quick bar slots */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|QuickBars")
	TArray<FFortItemGuid> PrimaryQuickBarSlots;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|QuickBars")
	TArray<FFortItemGuid> SecondaryQuickBarSlots;

	/** Quick bar configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|QuickBars")
	int32 PrimaryQuickBarSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|QuickBars")
	int32 SecondaryQuickBarSize;

	/** Current selection */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|QuickBars")
	int32 CurrentPrimarySlot;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|QuickBars")
	EFortQuickBars CurrentQuickBar;

protected:
	/** Validation helpers */
	bool ValidateSlotIndex(int32 SlotIndex, EFortQuickBars QuickBarType) const;
	bool CanAddItemToSlot(const FFortItemGuid& ItemGuid, int32 SlotIndex, EFortQuickBars QuickBarType) const;
};

/**
 * UFortItemDefinition - Base class for item definitions
 */
UCLASS(Abstract, Blueprintable)
class FORTNITEGAME_API UFortItemDefinition : public UObject
{
	GENERATED_BODY()

public:
	UFortItemDefinition(const FObjectInitializer& ObjectInitializer);

	/** Item properties */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Item")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Item")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Item")
	EFortItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Item")
	class UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Item")
	int32 MaxStackSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Item")
	bool bCanBeDropped;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Item")
	bool bCanBeDestroyed;
};

/**
 * UFortWeaponItemDefinition - Weapon item definition
 */
UCLASS()
class FORTNITEGAME_API UFortWeaponItemDefinition : public UFortItemDefinition
{
	GENERATED_BODY()

public:
	UFortWeaponItemDefinition(const FObjectInitializer& ObjectInitializer);

	/**
	 * Get weapon stats for this item
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Item")
	struct FWeaponStats GetWeaponStats() const;

	/** Weapon class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Weapon")
	TSubclassOf<class AFortWeapon> WeaponActorClass;

	/** Weapon stats */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Weapon")
	struct FWeaponStats WeaponStats;
};

/**
 * UFortWeaponRangedItemDefinition - Ranged weapon item definition
 */
UCLASS()
class FORTNITEGAME_API UFortWeaponRangedItemDefinition : public UFortWeaponItemDefinition
{
	GENERATED_BODY()

public:
	UFortWeaponRangedItemDefinition(const FObjectInitializer& ObjectInitializer);

	/** Ranged weapon specific properties */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Weapon")
	float BulletSpread;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Weapon")
	int32 BulletsPerShot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Weapon")
	TSubclassOf<class AActor> ProjectileClass;
};

/**
 * UFortWeaponMeleeItemDefinition - Melee weapon item definition
 */
UCLASS()
class FORTNITEGAME_API UFortWeaponMeleeItemDefinition : public UFortWeaponItemDefinition
{
	GENERATED_BODY()

public:
	UFortWeaponMeleeItemDefinition(const FObjectInitializer& ObjectInitializer);

	/** Melee weapon specific properties */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Weapon")
	float SwingRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Weapon")
	float SwingAngle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Weapon")
	bool bCanHitMultiple;
};
