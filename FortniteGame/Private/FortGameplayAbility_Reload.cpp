// Copyright Epic Games, Inc. All Rights Reserved.
// FortGameplayAbility_Reload Implementation

#include "FortGameplayAbility_Reload.h"
#include "FortWeapon.h"
#include "FortPawn.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "TimerManager.h"

//////////////////////////////////////////////////////////////////////////
// UFortGameplayAbility_Reload

UFortGameplayAbility_Reload::UFortGameplayAbility_Reload()
	: ReloadMontage(nullptr)
	, ReloadTime(2.0f)
	, WeaponToReload(nullptr)
{

	InstancingPolicy = EFortAbilityInstancingPolicy::InstancedPerActor;
}

bool UFortGameplayAbility_Reload::CanActivateAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent) const
{
	if (!Super::CanActivateAbility_Implementation(AbilitySystemComponent))
	{
		return false;
	}

	// Check if pawn has a weapon
	AFortPawn* FortPawn = Cast<AFortPawn>(GetAvatarActor());
	if (!FortPawn)
	{
		return false;
	}

	// would check if currently equipped weapon needs reload
	// For now, allow activation
	return true;
}

void UFortGameplayAbility_Reload::ActivateAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, const FFortAbilityActivationInfo& ActivationInfo)
{
	Super::ActivateAbility_Implementation(AbilitySystemComponent, ActivationInfo);

	AFortPawn* FortPawn = Cast<AFortPawn>(GetAvatarActor());
	if (!FortPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("UFortGameplayAbility_Reload: Cannot activate - no pawn"));
		EndAbility(AbilitySystemComponent, true);
		return;
	}

	// would get currently equipped weapon
	// For now, find first weapon actor owned by pawn
	TArray<AActor*> AttachedActors;
	FortPawn->GetAttachedActors(AttachedActors);

	for (AActor* Actor : AttachedActors)
	{
		AFortWeapon* Weapon = Cast<AFortWeapon>(Actor);
		if (Weapon)
		{
			WeaponToReload = Weapon;
			break;
		}
	}

	if (!WeaponToReload)
	{
		UE_LOG(LogTemp, Warning, TEXT("UFortGameplayAbility_Reload: Cannot activate - no weapon found"));
		EndAbility(AbilitySystemComponent, true);
		return;
	}

	// Check if weapon needs reloading
	if (WeaponToReload->MagazineAmmoCount >= WeaponToReload->WeaponStats.ClipSize)
	{
		UE_LOG(LogTemp, Verbose, TEXT("UFortGameplayAbility_Reload: Weapon already fully loaded"));
		EndAbility(AbilitySystemComponent, true);
		return;
	}

	// Check if we have reserve ammo
	if (WeaponToReload->AmmoCount <= 0)
	{
		UE_LOG(LogTemp, Verbose, TEXT("UFortGameplayAbility_Reload: No reserve ammo available"));
		EndAbility(AbilitySystemComponent, true);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("UFortGameplayAbility_Reload: Starting reload - Current: %d/%d, Reserve: %d"),
		WeaponToReload->MagazineAmmoCount,
		WeaponToReload->WeaponStats.ClipSize,
		WeaponToReload->AmmoCount);

	// Play reload montage if available
	if (ReloadMontage && FortPawn->GetMesh())
	{
		UAnimInstance* AnimInstance = FortPawn->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			float MontageLength = AnimInstance->Montage_Play(ReloadMontage);
			if (MontageLength > 0.0f)
			{
				// Use montage duration as reload time
				ReloadTime = MontageLength;
			}
		}
	}

	// Set timer for reload completion
	UWorld* World = GetWorld();
	if (World)
	{
		float ActualReloadTime = WeaponToReload->WeaponStats.ReloadSeconds > 0.0f ?
			WeaponToReload->WeaponStats.ReloadSeconds : ReloadTime;

		World->GetTimerManager().SetTimer(
			ReloadTimerHandle,
			this,
			&UFortGameplayAbility_Reload::CompleteReload,
			ActualReloadTime,
			false
		);

		UE_LOG(LogTemp, Verbose, TEXT("UFortGameplayAbility_Reload: Reload will complete in %.2f seconds"), ActualReloadTime);
	}
}

void UFortGameplayAbility_Reload::EndAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, bool bWasCancelled)
{
	// Clear timer if ability is ended early
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(ReloadTimerHandle);
	}

	if (bWasCancelled)
	{
		UE_LOG(LogTemp, Verbose, TEXT("UFortGameplayAbility_Reload: Reload cancelled"));
	}

	Super::EndAbility_Implementation(AbilitySystemComponent, bWasCancelled);
}

void UFortGameplayAbility_Reload::CompleteReload()
{
	if (!WeaponToReload)
	{
		UE_LOG(LogTemp, Warning, TEXT("UFortGameplayAbility_Reload: CompleteReload called but weapon is null"));
		EndAbility(OwningAbilitySystemComponent, true);
		return;
	}

	// Calculate ammo to reload
	int32 AmmoNeeded = WeaponToReload->WeaponStats.ClipSize - WeaponToReload->MagazineAmmoCount;
	int32 AmmoToLoad = FMath::Min(AmmoNeeded, WeaponToReload->AmmoCount);

	if (AmmoToLoad > 0)
	{
		// Transfer ammo from reserve to magazine
		WeaponToReload->MagazineAmmoCount += AmmoToLoad;
		WeaponToReload->AmmoCount -= AmmoToLoad;

		// Update replicated weapon data
		WeaponToReload->ReplicatedWeaponData.AmmoInClip = WeaponToReload->MagazineAmmoCount;

		UE_LOG(LogTemp, Log, TEXT("UFortGameplayAbility_Reload: Reload complete - Loaded %d rounds, Magazine: %d/%d, Reserve: %d"),
			AmmoToLoad,
			WeaponToReload->MagazineAmmoCount,
			WeaponToReload->WeaponStats.ClipSize,
			WeaponToReload->AmmoCount);
	}

	// End the ability
	EndAbility(OwningAbilitySystemComponent, false);
}

void UFortGameplayAbility_Reload::CancelReload()
{
	UE_LOG(LogTemp, Verbose, TEXT("UFortGameplayAbility_Reload: CancelReload called"));

	// Cancel the ability
	EndAbility(OwningAbilitySystemComponent, true);
}
