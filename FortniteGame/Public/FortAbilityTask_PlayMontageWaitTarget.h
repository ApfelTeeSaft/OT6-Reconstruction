// Copyright Epic Games, Inc. All Rights Reserved.
// FortAbilityTask_PlayMontageWaitTarget

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FortAbilitySystem.h"
#include "FortAbilityTask_PlayMontageWaitTarget.generated.h"

class UAnimMontage;
class UAnimInstance;

/**
 * UFortAbilityTask_PlayMontageWaitTarget - Play montage and wait for target data
 */
UCLASS()
class FORTNITEGAME_API UFortAbilityTask_PlayMontageWaitTarget : public UObject
{
	GENERATED_BODY()

public:
	UFortAbilityTask_PlayMontageWaitTarget();

	/**
	 * Create and activate montage + target wait task
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (BlueprintInternalUseOnly = "TRUE"))
	static UFortAbilityTask_PlayMontageWaitTarget* PlayMontageAndWaitForTarget(
		UFortGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		UAnimMontage* MontageToPlay,
		float PlayRate = 1.0f,
		FName StartSectionName = NAME_None
	);

	/**
	 * Activate the task
	 */
	UFUNCTION()
	virtual void Activate();

	/**
	 * Called when target data is received
	 */
	UFUNCTION()
	void OnTargetDataReceived(const FFortAbilityTargetSelection& TargetData);

	/**
	 * Called when montage ends
	 */
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/**
	 * Called when montage is cancelled
	 */
	UFUNCTION()
	void OnMontageCancelled();

	// Declare delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMontageComplete);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetDataReady, const FFortAbilityTargetSelection&, TargetData);

	/** Called when montage completes */
	UPROPERTY(BlueprintAssignable)
	FOnMontageComplete OnCompleted;

	/** Called when montage is cancelled */
	UPROPERTY(BlueprintAssignable)
	FOnMontageComplete OnCancelled;

	/** Called when target data is ready */
	UPROPERTY(BlueprintAssignable)
	FOnTargetDataReady OnTargetReady;

protected:
	UPROPERTY()
	UFortGameplayAbility* OwningAbility;

	UPROPERTY()
	UAnimMontage* MontageToPlay;

	FName TaskInstanceName;
	float PlayRate;
	FName StartSectionName;

	bool bMontageFinished;
	bool bTargetReceived;
};

/**
 * UFortAbilityTask_SetNextMontageSectionAndWait - Set next montage section and wait
 */
UCLASS()
class FORTNITEGAME_API UFortAbilityTask_SetNextMontageSectionAndWait : public UObject
{
	GENERATED_BODY()

public:
	UFortAbilityTask_SetNextMontageSectionAndWait();

	/**
	 * Set next montage section and wait for it to complete
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (BlueprintInternalUseOnly = "TRUE"))
	static UFortAbilityTask_SetNextMontageSectionAndWait* SetNextMontageSectionAndWait(
		UFortGameplayAbility* OwningAbility,
		FName SectionName
	);

	/**
	 * Activate the task
	 */
	UFUNCTION()
	virtual void Activate();

	/**
	 * Called when section ends
	 */
	UFUNCTION()
	void OnSectionEnded();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSectionComplete);

	/** Called when section completes */
	UPROPERTY(BlueprintAssignable)
	FOnSectionComplete OnCompleted;

protected:
	UPROPERTY()
	UFortGameplayAbility* OwningAbility;

	FName SectionName;
};
