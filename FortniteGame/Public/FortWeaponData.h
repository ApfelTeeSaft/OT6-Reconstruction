// Copyright Epic Games, Inc. All Rights Reserved.
// Source: D:\BuildFarm\buildmachine_++Fortnite+Release-Live\FortniteGame\Source\FortniteGame\Private\Weapons\

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FortEnums.h"
#include "FortWeaponData.generated.h"

/**
 * FFortWeaponStats - Weapon statistics data table row
 */
USTRUCT(BlueprintType)
struct FFortWeaponStats : public FTableRowBase
{
	GENERATED_BODY()

	/** Base damage per shot */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float BaseDamage;

	/** Damage falloff start distance */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float FalloffStartDistance;

	/** Damage falloff end distance */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float FalloffEndDistance;

	/** Fire rate (shots per second) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float FireRate;

	/** Reload time in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float ReloadTime;

	/** Magazine size */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	int32 ClipSize;

	/** Bullets per shot (for shotguns) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	int32 BulletsPerCartridge;

	/** Base spread angle */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float Spread;

	/** Spread increase per shot */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float SpreadDownsightsMultiplier;

	/** Recoil magnitude */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float RecoilVertical;

	/** Horizontal recoil */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float RecoilHorizontal;

	/** Critical hit chance */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float CriticalHitChance;

	/** Critical hit multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float CriticalHitDamageMultiplier;

	/**
	 * Durability cost per fire
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float DurabilityCostPerFire;

	/** Max weapon durability */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float MaxDurability;

	/** Stamina cost per shot */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float StaminaCost;

	FFortWeaponStats()
		: BaseDamage(20.0f)
		, FalloffStartDistance(2500.0f)
		, FalloffEndDistance(5000.0f)
		, FireRate(5.0f)
		, ReloadTime(2.5f)
		, ClipSize(30)
		, BulletsPerCartridge(1)
		, Spread(1.0f)
		, SpreadDownsightsMultiplier(0.5f)
		, RecoilVertical(1.0f)
		, RecoilHorizontal(0.5f)
		, CriticalHitChance(0.1f)
		, CriticalHitDamageMultiplier(2.0f)
		, DurabilityCostPerFire(1.0f)
		, MaxDurability(100.0f)
		, StaminaCost(0.0f)
	{}
};

/**
 * FFortAmmoData - Ammo type data
 */
USTRUCT(BlueprintType)
struct FFortAmmoData : public FTableRowBase
{
	GENERATED_BODY()

	/** Ammo type name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	FName AmmoType;

	/** Max stack size */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	int32 MaxStackSize;

	/** Drop amount on death */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	int32 DropAmountOnDeath;

	FFortAmmoData()
		: AmmoType(NAME_None)
		, MaxStackSize(999)
		, DropAmountOnDeath(0)
	{}
};

/**
 * FFortReplicatedWeaponData - Replicated weapon runtime data
 */
USTRUCT(BlueprintType)
struct FFortReplicatedWeaponData
{
	GENERATED_BODY()

	/** Current ammo in clip */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	int32 AmmoInClip;

	/** Current weapon durability */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	float Durability;

	/** Current weapon level */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	int32 WeaponLevel;

	/** Is weapon equipped */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	bool bIsEquipped;

	/** Infinite durability cheat */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	bool bInfiniteDurability;

	/** Infinite ammo cheat */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	bool bInfiniteAmmo;

	FFortReplicatedWeaponData()
		: AmmoInClip(0)
		, Durability(100.0f)
		, WeaponLevel(1)
		, bIsEquipped(false)
		, bInfiniteDurability(false)
		, bInfiniteAmmo(false)
	{}
};

/**
 * EFortJumpStaminaCost - Jump stamina cost modes
 */
UENUM(BlueprintType)
enum class EFortJumpStaminaCost : uint8
{
	None UMETA(DisplayName = "None"),
	Trigger UMETA(DisplayName = "Trigger"),
	SprintTrigger UMETA(DisplayName = "Sprint Trigger"),
	SprintAir UMETA(DisplayName = "Sprint Air"),
	EFortJumpStaminaCost_MAX UMETA(Hidden)
};

/**
 * FFortAbilityTargetSelection - Target selection data
 */
USTRUCT(BlueprintType)
struct FFortAbilityTargetSelection
{
	GENERATED_BODY()

	/** Target actor */
	UPROPERTY(BlueprintReadWrite, Category = "Targeting")
	AActor* TargetActor;

	/** Target location */
	UPROPERTY(BlueprintReadWrite, Category = "Targeting")
	FVector TargetLocation;

	/** Target normal */
	UPROPERTY(BlueprintReadWrite, Category = "Targeting")
	FVector TargetNormal;

	/** Is target obstructed */
	UPROPERTY(BlueprintReadOnly, Category = "Targeting")
	bool bIsObstructed;

	/**
	 * Check if actor is obstructed by world geometry
	 */
	bool IsActorObstructedByWorld(AActor* Actor, UWorld* World);

	FFortAbilityTargetSelection()
		: TargetActor(nullptr)
		, TargetLocation(FVector::ZeroVector)
		, TargetNormal(FVector::UpVector)
		, bIsObstructed(false)
	{}
};

/**
 * FFortAccountItemCost - Account item costs for abilities
 */
USTRUCT(BlueprintType)
struct FFortAccountItemCost
{
	GENERATED_BODY()

	/** Item template ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
	FString ItemTemplateId;

	/** Quantity required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
	int32 Quantity;

	FFortAccountItemCost()
		: Quantity(1)
	{}
};

/**
 * UFortWeaponItemDefinition - Forward declaration for weapon data
 */
UCLASS()
class FORTNITEGAME_API UFortWeaponItemDefinition : public UObject
{
	GENERATED_BODY()

public:
	/** Weapon stats data table row */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FDataTableRowHandle WeaponStatsHandle;

	/** Ammo data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FDataTableRowHandle AmmoDataHandle;

	/** Weapon tier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 Tier;

	/** Get weapon stats */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool GetWeaponStats(FFortWeaponStats& OutStats) const;

	/** Get ammo data */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool GetAmmoData(FFortAmmoData& OutAmmoData) const;
};
