// Copyright Epic Games, Inc. All Rights Reserved.
// FortAbilityTask_DirectedMovement

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FortAbilitySystem.h"
#include "FortAbilityTask_DirectedMovement.generated.h"

class USceneComponent;

/**
 * UFortAbilityTask_DirectedMovement - Move AI to target location/actor
 */
UCLASS()
class FORTNITEGAME_API UFortAbilityTask_DirectedMovement : public UObject
{
	GENERATED_BODY()

public:
	UFortAbilityTask_DirectedMovement();

	/**
	 * Move to target actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (BlueprintInternalUseOnly = "TRUE"))
	static UFortAbilityTask_DirectedMovement* DirectedMovementToActor(
		UFortGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		AActor* TargetActor,
		float AcceptanceRadius = 50.0f,
		bool bStopOnOverlap = true
	);

	/**
	 * Move to target component
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (BlueprintInternalUseOnly = "TRUE"))
	static UFortAbilityTask_DirectedMovement* DirectedMovementToComponent(
		UFortGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		USceneComponent* TargetComponent,
		float AcceptanceRadius = 50.0f,
		bool bStopOnOverlap = true
	);

	/**
	 * Move to target location
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (BlueprintInternalUseOnly = "TRUE"))
	static UFortAbilityTask_DirectedMovement* DirectedMovementToLocation(
		UFortGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		FVector TargetLocation,
		float AcceptanceRadius = 50.0f
	);

	/**
	 * Activate the task
	 */
	UFUNCTION()
	virtual void Activate();

	/**
	 * Called on movement tick
	 */
	UFUNCTION()
	void TickMovement(float DeltaTime);

	/**
	 * Called when target is reached
	 */
	UFUNCTION()
	void OnTargetReached();

	// Declare delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMovementComplete);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMovementFailed);

	/** Called when movement completes successfully */
	UPROPERTY(BlueprintAssignable)
	FOnMovementComplete OnCompleted;

	/** Called when movement fails */
	UPROPERTY(BlueprintAssignable)
	FOnMovementFailed OnFailed;

protected:
	UPROPERTY()
	UFortGameplayAbility* OwningAbility;

	UPROPERTY()
	AActor* TargetActor;

	UPROPERTY()
	USceneComponent* TargetComponent;

	FName TaskInstanceName;
	FVector TargetLocation;
	float AcceptanceRadius;
	bool bStopOnOverlap;
	bool bUseTargetActor;
	bool bUseTargetComponent;
	bool bUseTargetLocation;

	FTimerHandle MovementTickHandle;
};
