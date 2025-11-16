// Copyright Epic Games, Inc. All Rights Reserved.
// Source: D:\BuildFarm\buildmachine_++Fortnite+Release-Live\FortniteGame\Source\FortniteGame\Private\Abilities\FortGameplayAbility_RangedWeapon.cpp

#include "FortGameplayAbility_RangedWeapon.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "FortWeapon.h"
#include "FortPlayerController.h"

UFortGameplayAbility_RangedWeapon::UFortGameplayAbility_RangedWeapon(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentAmmo(0)
	, bIsReloading(false)
{
	// Must be instantiated, not blueprint-only
}

void UFortGameplayAbility_RangedWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UFortGameplayAbility_RangedWeapon, ReplicatedWeaponData);
}

bool UFortGameplayAbility_RangedWeapon::CanActivateAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent) const
{
	if (!Super::CanActivateAbility_Implementation(AbilitySystemComponent))
	{
		return false;
	}

	// Check if reloading
	if (bIsReloading)
	{
		return false;
	}

	// Check ammo
	if (!HasAmmo() && !ReplicatedWeaponData.bInfiniteAmmo)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Cannot fire - no ammo"));
		return false;
	}

	// Check durability
	if (!HasDurability() && !ReplicatedWeaponData.bInfiniteDurability)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot fire - weapon durability depleted"));
		return false;
	}

	return true;
}

void UFortGameplayAbility_RangedWeapon::ActivateAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, const FFortAbilityActivationInfo& ActivationInfo)
{
	UE_LOG(LogTemp, Log, TEXT("Ranged Weapon Ability %s Activated"), *GetName());

	Super::ActivateAbility_Implementation(AbilitySystemComponent, ActivationInfo);

	// Fire weapon
	FireWeapon();
}

void UFortGameplayAbility_RangedWeapon::EndAbility_Implementation(UFortAbilitySystemComponent* AbilitySystemComponent, bool bWasCancelled)
{
	// Clear timers
	if (AbilitySystemComponent && AbilitySystemComponent->GetOwner())
	{
		AbilitySystemComponent->GetOwner()->GetWorldTimerManager().ClearTimer(FireTimerHandle);
		AbilitySystemComponent->GetOwner()->GetWorldTimerManager().ClearTimer(ReloadTimerHandle);
	}

	Super::EndAbility_Implementation(AbilitySystemComponent, bWasCancelled);
}

void UFortGameplayAbility_RangedWeapon::FireWeapon()
{
	if (!OwningAbilitySystemComponent)
	{
		return;
	}

	AActor* OwnerActor = OwningAbilitySystemComponent->GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	// Check ammo
	if (!HasAmmo() && !ReplicatedWeaponData.bInfiniteAmmo)
	{
		// Out of ammo, start reload
		StartReload();
		return;
	}

	// Consume ammo if not infinite
	if (!ReplicatedWeaponData.bInfiniteAmmo)
	{
		if (!ConsumeAmmo(WeaponStats.BulletsPerCartridge))
		{
			return;
		}
	}

	// Apply durability cost if not infinite
	if (!ReplicatedWeaponData.bInfiniteDurability)
	{
		ApplyWeaponDurabilityCost();
	}

	// Perform weapon trace for each bullet
	for (int32 i = 0; i < WeaponStats.BulletsPerCartridge; ++i)
	{
		FHitResult HitResult;
		if (PerformWeaponTrace(HitResult))
		{
			ApplyDamageToTarget(HitResult);
		}
	}

	// Play effects
	PlayFireEffects();

	// Apply recoil
	ApplyRecoil();

	UE_LOG(LogTemp, Verbose, TEXT("Fired weapon - Ammo remaining: %d"), CurrentAmmo);
}

void UFortGameplayAbility_RangedWeapon::StartReload()
{
	if (!OwningAbilitySystemComponent || bIsReloading)
	{
		return;
	}

	AActor* OwnerActor = OwningAbilitySystemComponent->GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	bIsReloading = true;

	// Play reload effects
	PlayReloadEffects();

	// Set timer to complete reload
	FTimerDelegate ReloadDelegate;
	ReloadDelegate.BindUObject(this, &UFortGameplayAbility_RangedWeapon::CompleteReload);

	OwnerActor->GetWorldTimerManager().SetTimer(
		ReloadTimerHandle,
		ReloadDelegate,
		WeaponStats.ReloadTime,
		false
	);

	UE_LOG(LogTemp, Log, TEXT("Started reload - Duration: %.1fs"), WeaponStats.ReloadTime);
}

void UFortGameplayAbility_RangedWeapon::CompleteReload()
{
	bIsReloading = false;

	// Refill clip
	CurrentAmmo = WeaponStats.ClipSize;
	ReplicatedWeaponData.AmmoInClip = CurrentAmmo;

	UE_LOG(LogTemp, Log, TEXT("Reload complete - Clip refilled to %d"), CurrentAmmo);
}

bool UFortGameplayAbility_RangedWeapon::ConsumeAmmo(int32 AmmoToConsume)
{
	if (CurrentAmmo >= AmmoToConsume)
	{
		CurrentAmmo -= AmmoToConsume;
		ReplicatedWeaponData.AmmoInClip = CurrentAmmo;
		return true;
	}

	return false;
}

void UFortGameplayAbility_RangedWeapon::ApplyWeaponDurabilityCost()
{
	float NewDurability = ReplicatedWeaponData.Durability - WeaponStats.DurabilityCostPerFire;

	if (NewDurability <= 0.0f)
	{
		// Weapon destroyed
		NewDurability = 0.0f;
		UE_LOG(LogTemp, Warning, TEXT("Weapon durability depleted - weapon destroyed"));

		// TODO: Trigger weapon destroyed event
	}

	ReplicatedWeaponData.Durability = NewDurability;
}

bool UFortGameplayAbility_RangedWeapon::HasAmmo() const
{
	return CurrentAmmo > 0 || ReplicatedWeaponData.bInfiniteAmmo;
}

bool UFortGameplayAbility_RangedWeapon::HasDurability() const
{
	return ReplicatedWeaponData.Durability > 0.0f || ReplicatedWeaponData.bInfiniteDurability;
}

void UFortGameplayAbility_RangedWeapon::PlayFireEffects()
{
	// Play muzzle flash, fire sound, etc.
	// this would trigger particle effects and sounds
	UE_LOG(LogTemp, Verbose, TEXT("Playing fire effects"));
}

void UFortGameplayAbility_RangedWeapon::PlayReloadEffects()
{
	// Play reload animation and sounds
	// this would play montages and sounds
	UE_LOG(LogTemp, Verbose, TEXT("Playing reload effects"));
}

void UFortGameplayAbility_RangedWeapon::ApplyRecoil()
{
	if (!OwningAbilitySystemComponent)
	{
		return;
	}

	AActor* OwnerActor = OwningAbilitySystemComponent->GetOwner();
	AController* Controller = Cast<AController>(OwnerActor);
	if (!Controller)
	{
		APawn* Pawn = Cast<APawn>(OwnerActor);
		if (Pawn)
		{
			Controller = Pawn->GetController();
		}
	}

	if (Controller)
	{
		// Apply recoil to view
		float VerticalRecoil = WeaponStats.RecoilVertical;
		float HorizontalRecoil = FMath::FRandRange(-WeaponStats.RecoilHorizontal, WeaponStats.RecoilHorizontal);

		FRotator RecoilRotation(VerticalRecoil, HorizontalRecoil, 0.0f);
		Controller->SetControlRotation(Controller->GetControlRotation() + RecoilRotation);
	}
}

FVector UFortGameplayAbility_RangedWeapon::ApplySpread(const FVector& Direction)
{
	// Calculate spread angle
	float SpreadAngle = WeaponStats.Spread;

	// Random spread
	float RandomX = FMath::FRandRange(-SpreadAngle, SpreadAngle);
	float RandomY = FMath::FRandRange(-SpreadAngle, SpreadAngle);

	// Create spread rotation
	FRotator SpreadRotation(RandomX, RandomY, 0.0f);

	// Apply to direction
	return SpreadRotation.RotateVector(Direction);
}

bool UFortGameplayAbility_RangedWeapon::PerformWeaponTrace(FHitResult& OutHit)
{
	if (!OwningAbilitySystemComponent)
	{
		return false;
	}

	AActor* OwnerActor = OwningAbilitySystemComponent->GetOwner();
	if (!OwnerActor)
	{
		return false;
	}

	// Get view point
	FVector ViewLocation;
	FRotator ViewRotation;

	AController* Controller = Cast<AController>(OwnerActor);
	if (!Controller)
	{
		APawn* Pawn = Cast<APawn>(OwnerActor);
		if (Pawn)
		{
			Controller = Pawn->GetController();
		}
	}

	if (Controller)
	{
		Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
	}
	else
	{
		ViewLocation = OwnerActor->GetActorLocation();
		ViewRotation = OwnerActor->GetActorRotation();
	}

	// Calculate trace direction with spread
	FVector TraceDirection = ApplySpread(ViewRotation.Vector());

	// Trace distance
	float TraceDistance = 10000.0f; // 100 meters

	FVector TraceStart = ViewLocation;
	FVector TraceEnd = ViewLocation + (TraceDirection * TraceDistance);

	// Perform line trace
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;

	UWorld* World = OwnerActor->GetWorld();
	if (!World)
	{
		return false;
	}

	bool bHit = World->LineTraceSingleByChannel(
		OutHit,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	// Debug draw
	#if !UE_BUILD_SHIPPING
	DrawDebugLine(World, TraceStart, bHit ? OutHit.Location : TraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);
	#endif

	return bHit;
}

void UFortGameplayAbility_RangedWeapon::ApplyDamageToTarget(const FHitResult& Hit)
{
	if (!Hit.GetActor())
	{
		return;
	}

	// Calculate distance for falloff
	float Distance = Hit.Distance;
	float Damage = CalculateDamageWithFalloff(Distance);

	// Check for critical hit
	if (RollForCriticalHit())
	{
		Damage *= WeaponStats.CriticalHitDamageMultiplier;
		UE_LOG(LogTemp, Log, TEXT("Critical hit! Damage: %.1f"), Damage);
	}

	UE_LOG(LogTemp, Verbose, TEXT("Base Damage: %f, Final Damage: %.1f, Distance: %.1f"),
		WeaponStats.BaseDamage, Damage, Distance);

	// Apply damage
	if (OwningAbilitySystemComponent)
	{
		AActor* OwnerActor = OwningAbilitySystemComponent->GetOwner();
		if (OwnerActor)
		{
			FDamageEvent DamageEvent;
			Hit.GetActor()->TakeDamage(Damage, DamageEvent, OwnerActor->GetInstigatorController(), OwnerActor);
		}
	}
}

float UFortGameplayAbility_RangedWeapon::CalculateDamageWithFalloff(float Distance) const
{
	if (Distance <= WeaponStats.FalloffStartDistance)
	{
		// No falloff
		return WeaponStats.BaseDamage;
	}
	else if (Distance >= WeaponStats.FalloffEndDistance)
	{
		// Maximum falloff (50% damage)
		return WeaponStats.BaseDamage * 0.5f;
	}
	else
	{
		// Linear interpolation between start and end
		float FalloffRange = WeaponStats.FalloffEndDistance - WeaponStats.FalloffStartDistance;
		float DistanceInRange = Distance - WeaponStats.FalloffStartDistance;
		float FalloffPercent = DistanceInRange / FalloffRange;

		// Interpolate from 100% to 50%
		float DamageMultiplier = FMath::Lerp(1.0f, 0.5f, FalloffPercent);
		return WeaponStats.BaseDamage * DamageMultiplier;
	}
}

bool UFortGameplayAbility_RangedWeapon::RollForCriticalHit() const
{
	float RandomValue = FMath::FRand();
	return RandomValue <= WeaponStats.CriticalHitChance;
}
