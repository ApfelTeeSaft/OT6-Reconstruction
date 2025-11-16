// Copyright Epic Games, Inc. All Rights Reserved.
// FortAbilityTask_SpawnProjectileAndWait Implementation

#include "FortAbilityTask_SpawnProjectileAndWait.h"
#include "Engine/World.h"
#include "GameFramework/ProjectileMovementComponent.h"

//////////////////////////////////////////////////////////////////////////
// UFortAbilityTask_SpawnProjectileAndWait

UFortAbilityTask_SpawnProjectileAndWait::UFortAbilityTask_SpawnProjectileAndWait()
	: OwningAbility(nullptr)
	, SpawnedProjectile(nullptr)
	, ProjectileClass(nullptr)
	, SpawnLocation(FVector::ZeroVector)
	, SpawnRotation(FRotator::ZeroRotator)
	, ProjectileSpeed(3000.0f)
{
}

UFortAbilityTask_SpawnProjectileAndWait* UFortAbilityTask_SpawnProjectileAndWait::SpawnProjectileAndWait(
	UFortGameplayAbility* OwningAbility,
	TSubclassOf<AActor> ProjectileClass,
	FVector SpawnLocation,
	FRotator SpawnRotation,
	float ProjectileSpeed)
{
	UFortAbilityTask_SpawnProjectileAndWait* Task = NewObject<UFortAbilityTask_SpawnProjectileAndWait>();
	if (Task)
	{
		Task->OwningAbility = OwningAbility;
		Task->ProjectileClass = ProjectileClass;
		Task->SpawnLocation = SpawnLocation;
		Task->SpawnRotation = SpawnRotation;
		Task->ProjectileSpeed = ProjectileSpeed;
	}

	return Task;
}

void UFortAbilityTask_SpawnProjectileAndWait::Activate()
{
	if (!OwningAbility || !ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UFortAbilityTask_SpawnProjectileAndWait: Cannot activate - missing ability or projectile class"));
		OnDestroyed.Broadcast(nullptr);
		return;
	}

	UWorld* World = OwningAbility->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("UFortAbilityTask_SpawnProjectileAndWait: Cannot activate - no world"));
		OnDestroyed.Broadcast(nullptr);
		return;
	}

	// Spawn the projectile
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwningAbility->GetAvatarActor();
	SpawnParams.Instigator = Cast<APawn>(OwningAbility->GetAvatarActor());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	SpawnedProjectile = World->SpawnActor<AActor>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (SpawnedProjectile)
	{
		UE_LOG(LogTemp, Log, TEXT("UFortAbilityTask_SpawnProjectileAndWait: Spawned projectile %s at %s"),
			*SpawnedProjectile->GetName(), *SpawnLocation.ToString());

		// Set projectile speed if it has a movement component
		UProjectileMovementComponent* MovementComp = SpawnedProjectile->FindComponentByClass<UProjectileMovementComponent>();
		if (MovementComp)
		{
			MovementComp->InitialSpeed = ProjectileSpeed;
			MovementComp->MaxSpeed = ProjectileSpeed;
		}

		// Bind to destroyed event
		SpawnedProjectile->OnDestroyed.AddDynamic(this, &UFortAbilityTask_SpawnProjectileAndWait::OnProjectileDestroyed);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UFortAbilityTask_SpawnProjectileAndWait: Failed to spawn projectile"));
		OnDestroyed.Broadcast(nullptr);
	}
}

void UFortAbilityTask_SpawnProjectileAndWait::OnProjectileDestroyed(AActor* DestroyedActor)
{
	UE_LOG(LogTemp, Verbose, TEXT("UFortAbilityTask_SpawnProjectileAndWait: Projectile destroyed"));

	OnDestroyed.Broadcast(DestroyedActor);
}

void UFortAbilityTask_SpawnProjectileAndWait::OnProjectileHit(AActor* HitActor, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Verbose, TEXT("UFortAbilityTask_SpawnProjectileAndWait: Projectile hit %s"),
		HitActor ? *HitActor->GetName() : TEXT("null"));

	OnHit.Broadcast(HitActor, Hit);
}
