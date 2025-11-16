// Copyright Epic Games, Inc. All Rights Reserved.
// Source: D:\BuildFarm\buildmachine_++Fortnite+Release-Live\FortniteGame\Source\FortniteGame\Private\Abilities\FortGameplayAbility_RangedWeapon.cpp

#pragma once

#include "CoreMinimal.h"
#include "FortAbilitySystem.h"
#include "FortWeaponData.h"
#include "FortGameplayAbility_RangedWeapon.generated.h"

/**
 * UFortGameplayAbility_RangedWeapon - Ranged weapon firing ability
 */
UCLASS(Abstract)
class FORTNITEGAME_API UFortGameplayAbility_RangedWeapon : public UFortGameplayAbility
{
	GENERATED_BODY()

public:
	UFortGameplayAbility_RangedWeapon(const FObjectInitializer& ObjectInitializer);

	//~ Begin UFortGameplayAbility Interface
	virtual bool CanActivateAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent) const override;
	virtual void ActivateAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, const FFortAbilityActivationInfo& ActivationInfo) override;
	virtual void EndAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, bool bWasCancelled) override;
	//~ End UFortGameplayAbility Interface

	/**
	 * Fire the weapon
	 * Source: Referenced in ability activation
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|RangedWeapon")
	virtual void FireWeapon();

	/**
	 * Start reload
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|RangedWeapon")
	virtual void StartReload();

	/**
	 * Complete reload
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|RangedWeapon")
	virtual void CompleteReload();

	/**
	 * Consume ammo
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|RangedWeapon")
	virtual bool ConsumeAmmo(int32 AmmoToConsume);

	/**
	 * Apply weapon durability cost
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|RangedWeapon")
	virtual void ApplyWeaponDurabilityCost();

	/** Weapon stats */
	UPROPERTY(BlueprintReadOnly, Category = "Ability|RangedWeapon")
	FFortWeaponStats WeaponStats;

	/** Replicated weapon data */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Ability|RangedWeapon")
	FFortReplicatedWeaponData ReplicatedWeaponData;

	/** Current ammo count */
	UPROPERTY(BlueprintReadOnly, Category = "Ability|RangedWeapon")
	int32 CurrentAmmo;

	/** Is currently reloading */
	UPROPERTY(BlueprintReadOnly, Category = "Ability|RangedWeapon")
	bool bIsReloading;

	/** Fire timer handle */
	FTimerHandle FireTimerHandle;

	/** Reload timer handle */
	FTimerHandle ReloadTimerHandle;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Check ammo availability
	 */
	bool HasAmmo() const;

	/**
	 * Check weapon durability
	 */
	bool HasDurability() const;

	/**
	 * Play fire effects
	 */
	void PlayFireEffects();

	/**
	 * Play reload effects
	 */
	void PlayReloadEffects();

	/**
	 * Apply recoil
	 */
	void ApplyRecoil();

	/**
	 * Apply spread
	 */
	FVector ApplySpread(const FVector& Direction);

	/**
	 * Perform line trace for hit
	 */
	bool PerformWeaponTrace(FHitResult& OutHit);

	/**
	 * Apply damage to hit target
	 */
	void ApplyDamageToTarget(const FHitResult& Hit);

	/**
	 * Calculate damage with falloff
	 */
	float CalculateDamageWithFalloff(float Distance) const;

	/**
	 * Check for critical hit
	 */
	bool RollForCriticalHit() const;
};
