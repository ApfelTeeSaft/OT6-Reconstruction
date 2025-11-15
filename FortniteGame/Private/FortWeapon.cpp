// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite Weapon Implementation

#include "FortWeapon.h"
#include "FortPawn.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

//////////////////////////////////////////////////////////////////////////
// AFortWeapon

AFortWeapon::AFortWeapon(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, TriggerType(EFortWeaponTriggerType::OnPress)
	, WeaponCoreAnimation(EFortWeaponCoreAnimation::Rifle)
	, AmmoCount(999)
	, MagazineAmmoCount(30)
	, bIsReloading(false)
	, bIsFiring(false)
	, CurrentDurability(100.0f)
	, MaxDurability(100.0f)
	, OwnerPawn(nullptr)
	, WeaponFireMontage(nullptr)
	, WeaponFireDownsightsMontage(nullptr)
	, WeaponFireCue(nullptr)
{

	bReplicates = true;
	bNetUseOwnerRelevancy = true;

	PrimaryActorTick.bCanEverTick = true;
}

void AFortWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		MagazineAmmoCount = WeaponStats.MagazineSize;
		CurrentDurability = MaxDurability;
	}
}

void AFortWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFortWeapon::OnPawnMontageStarted(UAnimMontage* Montage)
{

	if (!Montage || !OwnerPawn)
	{
		return;
	}

	UE_LOG(LogNet, Verbose, TEXT("AFortWeapon::OnPawnMontageStarted - Montage: %s"), *Montage->GetName());

	// Handle animation synchronization with weapon
	// Fire montages, reload montages, etc.
}

void AFortWeapon::PlayWeaponFireFX()
{

	if (Role != ROLE_Authority && GetNetMode() != NM_DedicatedServer)
	{
		// Play montage
		if (WeaponFireMontage && OwnerPawn)
		{
			UAnimInstance* AnimInstance = OwnerPawn->GetMesh() ? OwnerPawn->GetMesh()->GetAnimInstance() : nullptr;
			if (AnimInstance)
			{
				AnimInstance->Montage_Play(WeaponFireMontage);
			}
		}

		// Play sound
		if (WeaponFireCue)
		{
			UGameplayStatics::PlaySoundAtLocation(this, WeaponFireCue, GetActorLocation());
		}

		// Call blueprint event
		OnPlayWeaponFireFX();
	}
}

void AFortWeapon::StopWeaponFireFX()
{

	if (Role != ROLE_Authority && GetNetMode() != NM_DedicatedServer)
	{
		// Stop montage
		if (OwnerPawn)
		{
			UAnimInstance* AnimInstance = OwnerPawn->GetMesh() ? OwnerPawn->GetMesh()->GetAnimInstance() : nullptr;
			if (AnimInstance && WeaponFireMontage)
			{
				AnimInstance->Montage_Stop(0.2f, WeaponFireMontage);
			}
		}

		// Call blueprint event
		OnStopWeaponFireFX();
	}
}

bool AFortWeapon::TryFire()
{
	if (!CanFire())
	{
		return false;
	}

	if (Role == ROLE_Authority)
	{
		FireWeaponInternal();
		return true;
	}

	return false;
}

void AFortWeapon::StartFire()
{
	if (!CanFire())
	{
		return;
	}

	bIsFiring = true;

	// Immediate first shot
	TryFire();

	// Set up automatic fire if applicable
	if (TriggerType == EFortWeaponTriggerType::Automatic)
	{
		float FireInterval = 1.0f / WeaponStats.FireRate;
		GetWorldTimerManager().SetTimer(
			FireTimerHandle,
			this,
			&AFortWeapon::TryFire,
			FireInterval,
			true
		);
	}
}

void AFortWeapon::StopFire()
{
	bIsFiring = false;

	// Clear fire timer
	if (FireTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(FireTimerHandle);
	}

	// Stop FX
	StopWeaponFireFX();
}

void AFortWeapon::Reload()
{
	if (bIsReloading || MagazineAmmoCount >= WeaponStats.MagazineSize)
	{
		return;
	}

	if (Role == ROLE_Authority)
	{
		bIsReloading = true;

		// Stop firing
		StopFire();

		// Set reload timer
		GetWorldTimerManager().SetTimer(
			ReloadTimerHandle,
			this,
			&AFortWeapon::CompleteReload,
			WeaponStats.ReloadTime,
			false
		);

		UE_LOG(LogNet, Log, TEXT("AFortWeapon: Reloading - %.2f seconds"), WeaponStats.ReloadTime);
	}
}

bool AFortWeapon::CanFire() const
{
	// Check for out of ammo condition

	if (bIsReloading)
	{
		return false;
	}

	if (MagazineAmmoCount <= 0)
	{
		UE_LOG(LogNet, Verbose, TEXT("AFortWeapon: Cannot fire - OutOfAmmo"));
		return false;
	}

	if (CurrentDurability <= 0.0f)
	{
		UE_LOG(LogNet, Verbose, TEXT("AFortWeapon: Cannot fire - Weapon broken"));
		return false;
	}

	return true;
}

void AFortWeapon::FireWeaponInternal()
{
	if (Role != ROLE_Authority)
	{
		return;
	}

	// Consume ammo
	MagazineAmmoCount--;

	// Reduce durability
	CurrentDurability -= 0.1f;
	if (CurrentDurability < 0.0f)
	{
		CurrentDurability = 0.0f;
	}

	// Play FX (replicated)
	PlayWeaponFireFX();

	UE_LOG(LogNet, Verbose, TEXT("AFortWeapon: FireWeaponInternal - Ammo: %d/%d"), MagazineAmmoCount, AmmoCount);

	// Auto reload if empty
	if (MagazineAmmoCount <= 0 && AmmoCount > 0)
	{
		Reload();
	}
}

void AFortWeapon::CompleteReload()
{
	if (Role != ROLE_Authority)
	{
		return;
	}

	bIsReloading = false;

	// Calculate ammo to reload
	int32 AmmoNeeded = WeaponStats.MagazineSize - MagazineAmmoCount;
	int32 AmmoToReload = FMath::Min(AmmoNeeded, AmmoCount);

	// Transfer ammo
	MagazineAmmoCount += AmmoToReload;
	AmmoCount -= AmmoToReload;

	UE_LOG(LogNet, Log, TEXT("AFortWeapon: Reload complete - Magazine: %d, Reserve: %d"), MagazineAmmoCount, AmmoCount);
}

void AFortWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFortWeapon, AmmoCount);
	DOREPLIFETIME(AFortWeapon, MagazineAmmoCount);
	DOREPLIFETIME(AFortWeapon, bIsReloading);
	DOREPLIFETIME(AFortWeapon, bIsFiring);
	DOREPLIFETIME(AFortWeapon, CurrentDurability);
	DOREPLIFETIME(AFortWeapon, OwnerPawn);
}

//////////////////////////////////////////////////////////////////////////
// AFortWeaponRanged

AFortWeaponRanged::AFortWeaponRanged(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, BulletSpread(0.0f)
	, BulletsPerShot(1)
	, bUseProjectile(false)
	, ProjectileClass(nullptr)
	, ProjectileSpeed(10000.0f)
{

	WeaponCoreAnimation = EFortWeaponCoreAnimation::Rifle;
}

bool AFortWeaponRanged::TryFire()
{
	if (!CanFire())
	{
		return false;
	}

	if (Role == ROLE_Authority)
	{
		FireWeaponInternal();
		return true;
	}

	return false;
}

void AFortWeaponRanged::FireWeaponInternal()
{
	Super::FireWeaponInternal();

	if (Role != ROLE_Authority)
	{
		return;
	}

	// Fire multiple bullets if applicable (shotgun)
	for (int32 i = 0; i < BulletsPerShot; ++i)
	{
		if (bUseProjectile)
		{
			// Spawn projectile
			FVector FireDirection = GetActorForwardVector();
			SpawnProjectile(FireDirection);
		}
		else
		{
			// Hitscan
			FHitResult Hit;
			if (PerformHitscan(Hit))
			{
				ApplyWeaponDamage(Hit);
			}
		}
	}
}

bool AFortWeaponRanged::PerformHitscan(FHitResult& OutHit)
{
	if (!OwnerPawn)
	{
		return false;
	}

	// Get fire start location and direction
	FVector StartLocation = GetActorLocation();
	FVector ForwardVector = GetActorForwardVector();

	// Apply spread
	FRotator SpreadRotation = FRotator(
		FMath::RandRange(-BulletSpread, BulletSpread),
		FMath::RandRange(-BulletSpread, BulletSpread),
		0.0f
	);
	FVector FireDirection = SpreadRotation.RotateVector(ForwardVector);

	FVector EndLocation = StartLocation + (FireDirection * WeaponStats.Range);

	// Perform line trace
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(OwnerPawn);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		OutHit,
		StartLocation,
		EndLocation,
		ECC_Visibility,
		QueryParams
	);

	// Debug draw
	if (GetWorld()->GetNetMode() != NM_DedicatedServer)
	{
		DrawDebugLine(GetWorld(), StartLocation, EndLocation, bHit ? FColor::Red : FColor::Green, false, 1.0f);
	}

	return bHit;
}

void AFortWeaponRanged::ApplyWeaponDamage(const FHitResult& Hit)
{
	if (!Hit.GetActor() || Role != ROLE_Authority)
	{
		return;
	}

	// Apply damage
	FDamageEvent DamageEvent;
	Hit.GetActor()->TakeDamage(WeaponStats.Damage, DamageEvent, OwnerPawn ? OwnerPawn->GetController() : nullptr, this);

	UE_LOG(LogNet, Verbose, TEXT("AFortWeaponRanged: Applied %.2f damage to %s"), WeaponStats.Damage, *Hit.GetActor()->GetName());
}

AActor* AFortWeaponRanged::SpawnProjectile(const FVector& Direction)
{
	if (!ProjectileClass || Role != ROLE_Authority)
	{
		return nullptr;
	}

	FVector SpawnLocation = GetActorLocation() + (Direction * 100.0f);
	FRotator SpawnRotation = Direction.Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = OwnerPawn;

	AActor* Projectile = GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

	UE_LOG(LogNet, Verbose, TEXT("AFortWeaponRanged: Spawned projectile"));

	return Projectile;
}

void AFortWeaponRanged::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

//////////////////////////////////////////////////////////////////////////
// AFortWeaponMelee

AFortWeaponMelee::AFortWeaponMelee(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SwingRadius(200.0f)
	, SwingAngle(90.0f)
	, bCanHitMultiple(true)
	, MaxTargets(5)
{

	WeaponCoreAnimation = EFortWeaponCoreAnimation::Melee;
	TriggerType = EFortWeaponTriggerType::OnPress;

	// Melee weapons don't use ammo in traditional sense
	WeaponStats.MagazineSize = 999;
}

bool AFortWeaponMelee::TryFire()
{
	if (!CanFire())
	{
		return false;
	}

	if (Role == ROLE_Authority)
	{
		FireWeaponInternal();
		return true;
	}

	return false;
}

void AFortWeaponMelee::FireWeaponInternal()
{
	// Don't call Super::FireWeaponInternal() as melee doesn't consume ammo the same way

	if (Role != ROLE_Authority)
	{
		return;
	}

	// Perform melee swing
	TArray<FHitResult> Hits;
	if (PerformMeleeSwing(Hits))
	{
		ApplyMeleeDamage(Hits);
	}

	// Reduce durability
	CurrentDurability -= 0.5f;
	if (CurrentDurability < 0.0f)
	{
		CurrentDurability = 0.0f;
	}

	// Play FX
	PlayWeaponFireFX();

	UE_LOG(LogNet, Verbose, TEXT("AFortWeaponMelee: Performed melee swing - Hit %d targets"), Hits.Num());
}

bool AFortWeaponMelee::PerformMeleeSwing(TArray<FHitResult>& OutHits)
{
	if (!OwnerPawn)
	{
		return false;
	}

	// Get swing origin and direction
	FVector StartLocation = GetActorLocation();
	FVector ForwardVector = GetActorForwardVector();

	// Perform sphere sweep
	FCollisionShape SweepShape = FCollisionShape::MakeSphere(SwingRadius);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(OwnerPawn);

	bool bHit = GetWorld()->SweepMultiByChannel(
		OutHits,
		StartLocation,
		StartLocation + (ForwardVector * SwingRadius * 2.0f),
		FQuat::Identity,
		ECC_Pawn,
		SweepShape,
		QueryParams
	);

	// Limit to max targets if applicable
	if (!bCanHitMultiple && OutHits.Num() > 0)
	{
		FHitResult ClosestHit = OutHits[0];
		OutHits.Empty();
		OutHits.Add(ClosestHit);
	}
	else if (OutHits.Num() > MaxTargets)
	{
		OutHits.SetNum(MaxTargets);
	}

	return bHit;
}

void AFortWeaponMelee::ApplyMeleeDamage(const TArray<FHitResult>& Hits)
{
	if (Role != ROLE_Authority)
	{
		return;
	}

	for (const FHitResult& Hit : Hits)
	{
		if (Hit.GetActor())
		{
			FDamageEvent DamageEvent;
			Hit.GetActor()->TakeDamage(WeaponStats.Damage, DamageEvent, OwnerPawn ? OwnerPawn->GetController() : nullptr, this);

			UE_LOG(LogNet, Verbose, TEXT("AFortWeaponMelee: Applied %.2f damage to %s"), WeaponStats.Damage, *Hit.GetActor()->GetName());
		}
	}
}

void AFortWeaponMelee::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

//////////////////////////////////////////////////////////////////////////
// AFortWeaponHarvest

AFortWeaponHarvest::AFortWeaponHarvest(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, HarvestDamageMultiplier(2.0f)
	, ResourceGatherMultiplier(1.0f)
{
	// Source: EFortItemType::WeaponHarvest

	WeaponCoreAnimation = EFortWeaponCoreAnimation::MeleeOneHand;

	// Harvest tools have high durability
	MaxDurability = 600.0f;

	// Harvest tools hit fewer targets
	bCanHitMultiple = false;
	MaxTargets = 1;
}

void AFortWeaponHarvest::FireWeaponInternal()
{
	Super::FireWeaponInternal();

	// Additional resource harvesting logic
	// Would check if hit actor is a resource node and harvest materials
}

void AFortWeaponHarvest::HarvestResources(AActor* Target)
{
	if (!Target || Role != ROLE_Authority)
	{
		return;
	}

	// Check if target is harvestable (building, resource node, etc.)
	// Apply extra damage to buildings
	// Spawn resource pickups

	float HarvestDamage = WeaponStats.Damage * HarvestDamageMultiplier;

	FDamageEvent DamageEvent;
	Target->TakeDamage(HarvestDamage, DamageEvent, OwnerPawn ? OwnerPawn->GetController() : nullptr, this);

	UE_LOG(LogNet, Log, TEXT("AFortWeaponHarvest: Harvested resources from %s - Damage: %.2f"),
		*Target->GetName(), HarvestDamage);
}
