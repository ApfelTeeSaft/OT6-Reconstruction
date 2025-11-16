// Copyright Epic Games, Inc. All Rights Reserved.
// Source: D:\BuildFarm\buildmachine_++Fortnite+Release-Live\FortniteGame\Source\FortniteGame\Private\Weapons\Data\FortWeaponItemDefinition.cpp

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FortWeaponData.h"
#include "FortWeapon.h"
#include "FortWeaponItemDefinition.generated.h"

/**
 * UFortWeaponItemDefinition - Defines weapon properties and stats
 */
UCLASS(Blueprintable)
class FORTNITEGAME_API UFortWeaponItemDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	UFortWeaponItemDefinition();

	/**
	 * Get weapon stats
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Weapon")
	const FFortWeaponStats& GetWeaponStats() const;

	/**
	 * Get weapon stats row
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Weapon")
	FFortWeaponStats GetWeaponStatsRow() const;

	/**
	 * Get weapon durability by rarity stats
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Weapon")
	float GetWeaponDurabilityByRarity(EFortRarity Rarity) const;

	/** Weapon display name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Item")
	FText DisplayName;

	/** Weapon description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Item")
	FText Description;

	/** Weapon icon */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Item")
	UTexture2D* Icon;

	/** Weapon rarity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Item")
	EFortRarity Rarity;

	/**
	 * Weapon class to spawn
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Weapon")
	TSubclassOf<AFortWeapon> WeaponActorClass;

	/**
	 * Weapon statistics
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Weapon")
	FFortWeaponStats WeaponStats;

	/**
	 * Weapon core animation type
	 * Determines what animation set to use
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Animation")
	EFortWeaponCoreAnimation WeaponCoreAnimation;

	/**
	 * Weapon trigger type
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Weapon")
	EFortWeaponTriggerType TriggerType;

	/**
	 * Weapon mesh
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Weapon")
	USkeletalMesh* WeaponMesh;

	/**
	 * Weapon fire montage
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Animation")
	UAnimMontage* WeaponFireMontage;

	/**
	 * Weapon fire downsights montage
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Animation")
	UAnimMontage* WeaponFireDownsightsMontage;

	/**
	 * Weapon reload montage
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Animation")
	UAnimMontage* WeaponReloadMontage;

	/**
	 * Weapon fire sound
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Audio")
	USoundBase* WeaponFireCue;

	/**
	 * Weapon reload sound
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Audio")
	USoundBase* WeaponReloadCue;

	/**
	 * Bullet shell FX
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Effects")
	UParticleSystem* BulletShellFX;

	/**
	 * Muzzle flash FX
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Effects")
	UParticleSystem* MuzzleFlashFX;

	/**
	 * Impact FX
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Effects")
	UParticleSystem* ImpactFX;

	/**
	 * Tracer FX
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Effects")
	UParticleSystem* TracerFX;

	/**
	 * Weapon durability by rarity
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Durability")
	TMap<EFortRarity, float> DurabilityByRarity;

	/**
	 * Looted weapons durability modifier
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Durability")
	float LootedWeaponsDurabilityModifier;

	/**
	 * Ammo data reference
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Ammo")
	FFortAmmoData AmmoData;
};

/**
 * UFortWeaponRangedItemDefinition - Ranged weapon item definition
 */
UCLASS()
class FORTNITEGAME_API UFortWeaponRangedItemDefinition : public UFortWeaponItemDefinition
{
	GENERATED_BODY()

public:
	UFortWeaponRangedItemDefinition();

	/** Projectile class for projectile weapons */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Weapon")
	TSubclassOf<AActor> ProjectileClass;

	/** Use projectile instead of hitscan */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Weapon")
	bool bUseProjectile;

	/** Projectile speed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Weapon")
	float ProjectileSpeed;
};

/**
 * UFortWeaponMeleeItemDefinition - Melee weapon item definition
 */
UCLASS()
class FORTNITEGAME_API UFortWeaponMeleeItemDefinition : public UFortWeaponItemDefinition
{
	GENERATED_BODY()

public:
	UFortWeaponMeleeItemDefinition();

	/** Swing radius */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Weapon")
	float SwingRadius;

	/** Swing angle */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Weapon")
	float SwingAngle;

	/** Can hit multiple targets */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Weapon")
	bool bCanHitMultiple;

	/** Maximum number of targets */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fort|Weapon")
	int32 MaxTargets;
};
