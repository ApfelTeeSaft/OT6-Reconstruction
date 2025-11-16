// Copyright Epic Games, Inc. All Rights Reserved.
// Source: D:\BuildFarm\buildmachine_++Fortnite+Release-Live\FortniteGame\Source\FortniteGame\Private\Abilities\Tasks\FortAbilityTask_CommitAccountCosts.cpp

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FortAbilitySystem.h"
#include "FortWeaponData.h"
#include "FortAbilityTask_CommitAccountCosts.generated.h"

/**
 * EFortAbilityCommitResult - Result of ability cost commitment
 */
UENUM(BlueprintType)
enum class EFortAbilityCommitResult : uint8
{
	Success UMETA(DisplayName = "Success"),
	FailedStaminaCost UMETA(DisplayName = "Failed - Insufficient Stamina"),
	FailedItemCost UMETA(DisplayName = "Failed - Insufficient Items"),
	FailedMCPOutOfSync UMETA(DisplayName = "Failed - MCP Out of Sync"),
	FailedAbilityGone UMETA(DisplayName = "Failed - Ability Gone"),
	EFortAbilityCommitResult_MAX UMETA(Hidden)
};

/**
 * UFortAbilityTask_CommitAccountCosts - Task to commit ability costs (stamina, items, etc.)
 */
UCLASS()
class FORTNITEGAME_API UFortAbilityTask_CommitAccountCosts : public UObject
{
	GENERATED_BODY()

public:
	UFortAbilityTask_CommitAccountCosts();

	/**
	 * Create and activate the cost commitment task
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (BlueprintInternalUseOnly = "TRUE"))
	static UFortAbilityTask_CommitAccountCosts* CommitAccountCosts(
		UFortGameplayAbility* OwningAbility,
		UFortAbilitySystemComponent* AbilitySystemComponent
	);

	/**
	 * Activate the task
	 */
	UFUNCTION()
	virtual void Activate();

	/**
	 * Cancel the task
	 */
	UFUNCTION()
	virtual void Cancel();

	// Declare delegates before using them in UPROPERTY
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCostCommitSuccess);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCostCommitFailed, EFortAbilityCommitResult, FailReason);

	/** Called when costs are successfully committed */
	UPROPERTY(BlueprintAssignable)
	FOnCostCommitSuccess OnSuccess;

	/** Called when cost commitment fails */
	UPROPERTY(BlueprintAssignable)
	FOnCostCommitFailed OnFailed;

protected:
	/**
	 * Check stamina cost
	 */
	bool CheckStaminaCost();

	/**
	 * Apply stamina cost
	 */
	void ApplyStaminaCost();

	/**
	 * Check item costs
	 */
	bool CheckItemCosts();

	/**
	 * Apply item costs
	 */
	bool ApplyItemCosts();

	/**
	 * Handle MCP out of sync
	 */
	void HandleMCPOutOfSync(const FString& ErrorMessage);

	/** Owning ability */
	UPROPERTY()
	UFortGameplayAbility* OwningAbility;

	/** Ability system component */
	UPROPERTY()
	UFortAbilitySystemComponent* AbilitySystemComponent;

	/** Is currently active */
	bool bIsActive;

	/** Stamina cost */
	float StaminaCost;

	/** Item costs */
	TArray<FFortAccountItemCost> ItemCosts;
};

/**
 * UFortAbilityTask_WaitTargetData - Wait for target data task
 */
UCLASS()
class FORTNITEGAME_API UFortAbilityTask_WaitTargetData : public UObject
{
	GENERATED_BODY()

public:
	UFortAbilityTask_WaitTargetData();

	/**
	 * Create and activate target data wait task
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (BlueprintInternalUseOnly = "TRUE"))
	static UFortAbilityTask_WaitTargetData* WaitTargetData(
		UFortGameplayAbility* OwningAbility,
		FName TaskInstanceName
	);

	/**
	 * Activate the task
	 */
	UFUNCTION()
	virtual void Activate();

	/**
	 * Confirm target data
	 */
	UFUNCTION()
	void ConfirmTargetData(const FFortAbilityTargetSelection& TargetData);

	// Declare delegate before using in UPROPERTY
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetDataReady, const FFortAbilityTargetSelection&, TargetData);

	/** Called when target data is received */
	UPROPERTY(BlueprintAssignable)
	FOnTargetDataReady OnTargetDataReady;

protected:
	UPROPERTY()
	UFortGameplayAbility* OwningAbility;

	FName TaskInstanceName;
	bool bIsWaiting;
};
