// Copyright Epic Games, Inc. All Rights Reserved.
// FortGameplayAbility_Reload

#pragma once

#include "CoreMinimal.h"
#include "FortAbilitySystem.h"
#include "FortGameplayAbility_Reload.generated.h"

class AFortWeapon;

/**
 * UFortGameplayAbility_Reload - Weapon reload ability
 */
UCLASS()
class FORTNITEGAME_API UFortGameplayAbility_Reload : public UFortGameplayAbility
{
	GENERATED_BODY()

public:
	UFortGameplayAbility_Reload();

	//~ Begin UFortGameplayAbility Interface
	virtual bool CanActivateAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent) const override;
	virtual void ActivateAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, const FFortAbilityActivationInfo& ActivationInfo) override;
	virtual void EndAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, bool bWasCancelled) override;
	//~ End UFortGameplayAbility Interface

	/**
	 * Complete the reload
	 */
	UFUNCTION()
	void CompleteReload();

	/**
	 * Cancel the reload
	 */
	UFUNCTION()
	void CancelReload();

	/** Reload montage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reload")
	UAnimMontage* ReloadMontage;

	/** Reload time (if not using montage) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reload")
	float ReloadTime;

protected:
	/** Timer handle for reload */
	FTimerHandle ReloadTimerHandle;

	/** Weapon being reloaded */
	UPROPERTY()
	AFortWeapon* WeaponToReload;
};
