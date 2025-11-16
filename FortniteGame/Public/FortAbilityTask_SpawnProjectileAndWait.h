// Copyright Epic Games, Inc. All Rights Reserved.
// FortAbilityTask_SpawnProjectileAndWait

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FortAbilitySystem.h"
#include "FortAbilityTask_SpawnProjectileAndWait.generated.h"

/**
 * UFortAbilityTask_SpawnProjectileAndWait - Spawn projectile and wait for it to be destroyed
 */
UCLASS()
class FORTNITEGAME_API UFortAbilityTask_SpawnProjectileAndWait : public UObject
{
	GENERATED_BODY()

public:
	UFortAbilityTask_SpawnProjectileAndWait();

	/**
	 * Spawn projectile and wait for destruction
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (BlueprintInternalUseOnly = "TRUE"))
	static UFortAbilityTask_SpawnProjectileAndWait* SpawnProjectileAndWait(
		UFortGameplayAbility* OwningAbility,
		TSubclassOf<AActor> ProjectileClass,
		FVector SpawnLocation,
		FRotator SpawnRotation,
		float ProjectileSpeed = 3000.0f
	);

	/**
	 * Activate the task
	 */
	UFUNCTION()
	virtual void Activate();

	/**
	 * Called when projectile is destroyed
	 */
	UFUNCTION()
	void OnProjectileDestroyed(AActor* DestroyedActor);

	/**
	 * Called when projectile hits something
	 */
	UFUNCTION()
	void OnProjectileHit(AActor* HitActor, const FHitResult& Hit);

	// Declare delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProjectileComplete, AActor*, Projectile);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProjectileHit, AActor*, HitActor, const FHitResult&, Hit);

	/** Called when projectile is destroyed */
	UPROPERTY(BlueprintAssignable)
	FOnProjectileComplete OnDestroyed;

	/** Called when projectile hits */
	UPROPERTY(BlueprintAssignable)
	FOnProjectileHit OnHit;

protected:
	UPROPERTY()
	UFortGameplayAbility* OwningAbility;

	UPROPERTY()
	AActor* SpawnedProjectile;

	TSubclassOf<AActor> ProjectileClass;
	FVector SpawnLocation;
	FRotator SpawnRotation;
	float ProjectileSpeed;
};
