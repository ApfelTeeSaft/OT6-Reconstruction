// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite Specific Building Types Implementation

#include "BuildingActorTypes.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"

//////////////////////////////////////////////////////////////////////////
// ABuildingSMActorWall

ABuildingSMActorWall::ABuildingSMActorWall(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BuildingType = EFortBuildingType::Wall;

	// Walls have standard dimensions
	// Default 1x1 wall piece
}

void ABuildingSMActorWall::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

float ABuildingSMActorWall::DetermineHealthMax()
{
	// Walls are the primary defensive structure
	float BaseHealth = 300.0f;

	switch (ResourceType)
	{
	case EBuildingResourceType::Wood:
		BaseHealth = 300.0f;
		break;
	case EBuildingResourceType::Stone:
		BaseHealth = 600.0f;
		break;
	case EBuildingResourceType::Metal:
		BaseHealth = 900.0f;
		break;
	default:
		BaseHealth = 300.0f;
		break;
	}

	return BaseHealth * HealthModifierPerLevel;
}

int32 ABuildingSMActorWall::DetermineMaxResourcesToSpawn()
{
	// Walls return ~60% of cost when destroyed
	return 6;
}

//////////////////////////////////////////////////////////////////////////
// ABuildingSMActorFloor

ABuildingSMActorFloor::ABuildingSMActorFloor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BuildingType = EFortBuildingType::Floor;

	// Floor buildings can have AI sleeping on them
}

void ABuildingSMActorFloor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

float ABuildingSMActorFloor::DetermineHealthMax()
{
	// Floors have slightly less health than walls
	float BaseHealth = 250.0f;

	switch (ResourceType)
	{
	case EBuildingResourceType::Wood:
		BaseHealth = 250.0f;
		break;
	case EBuildingResourceType::Stone:
		BaseHealth = 500.0f;
		break;
	case EBuildingResourceType::Metal:
		BaseHealth = 750.0f;
		break;
	default:
		BaseHealth = 250.0f;
		break;
	}

	return BaseHealth * HealthModifierPerLevel;
}

int32 ABuildingSMActorFloor::DetermineMaxResourcesToSpawn()
{
	// Floors return ~60% of cost
	return 5;
}

//////////////////////////////////////////////////////////////////////////
// ABuildingSMActorStairs

ABuildingSMActorStairs::ABuildingSMActorStairs(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BuildingType = EFortBuildingType::Stairs;

	// Stairs provide vertical mobility
}

void ABuildingSMActorStairs::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

float ABuildingSMActorStairs::DetermineHealthMax()
{
	// Stairs have more health due to importance for mobility
	float BaseHealth = 350.0f;

	switch (ResourceType)
	{
	case EBuildingResourceType::Wood:
		BaseHealth = 350.0f;
		break;
	case EBuildingResourceType::Stone:
		BaseHealth = 700.0f;
		break;
	case EBuildingResourceType::Metal:
		BaseHealth = 1050.0f;
		break;
	default:
		BaseHealth = 350.0f;
		break;
	}

	return BaseHealth * HealthModifierPerLevel;
}

int32 ABuildingSMActorStairs::DetermineMaxResourcesToSpawn()
{
	// Stairs cost more materials
	return 7;
}

//////////////////////////////////////////////////////////////////////////
// ABuildingSMActorRoof

ABuildingSMActorRoof::ABuildingSMActorRoof(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BuildingType = EFortBuildingType::Roof;

	// Roofs provide overhead protection
}

void ABuildingSMActorRoof::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

float ABuildingSMActorRoof::DetermineHealthMax()
{
	// Roofs have similar health to floors
	float BaseHealth = 250.0f;

	switch (ResourceType)
	{
	case EBuildingResourceType::Wood:
		BaseHealth = 250.0f;
		break;
	case EBuildingResourceType::Stone:
		BaseHealth = 500.0f;
		break;
	case EBuildingResourceType::Metal:
		BaseHealth = 750.0f;
		break;
	default:
		BaseHealth = 250.0f;
		break;
	}

	return BaseHealth * HealthModifierPerLevel;
}

int32 ABuildingSMActorRoof::DetermineMaxResourcesToSpawn()
{
	// Roofs return ~60% of cost
	return 5;
}

//////////////////////////////////////////////////////////////////////////
// ABuildingSMActorPillar

ABuildingSMActorPillar::ABuildingSMActorPillar(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BuildingType = EFortBuildingType::Pillar;

	// Pillars provide structural support
}

void ABuildingSMActorPillar::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

float ABuildingSMActorPillar::DetermineHealthMax()
{
	// Pillars have very high health as structural support
	float BaseHealth = 400.0f;

	switch (ResourceType)
	{
	case EBuildingResourceType::Wood:
		BaseHealth = 400.0f;
		break;
	case EBuildingResourceType::Stone:
		BaseHealth = 800.0f;
		break;
	case EBuildingResourceType::Metal:
		BaseHealth = 1200.0f;
		break;
	default:
		BaseHealth = 400.0f;
		break;
	}

	return BaseHealth * HealthModifierPerLevel;
}

int32 ABuildingSMActorPillar::DetermineMaxResourcesToSpawn()
{
	// Pillars return resources
	return 6;
}

//////////////////////////////////////////////////////////////////////////
// ABuildingSMActorTrap

ABuildingSMActorTrap::ABuildingSMActorTrap(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, TriggerRange(300.0f)
	, TrapDamage(100.0f)
	, CooldownTime(5.0f)
	, bIsArmed(true)
	, LastTriggerTime(0.0f)
{
	BuildingType = EFortBuildingType::Trap;

	// Traps are placed on walls/floors/ceilings
	PrimaryActorTick.bCanEverTick = true;
}

void ABuildingSMActorTrap::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

float ABuildingSMActorTrap::DetermineHealthMax()
{
	// Traps have low health
	return 150.0f;
}

int32 ABuildingSMActorTrap::DetermineMaxResourcesToSpawn()
{
	// Traps return minimal resources
	return 2;
}

bool ABuildingSMActorTrap::TryTriggerTrap(AActor* TargetActor)
{
	if (!TargetActor || !bIsArmed || Role != ROLE_Authority)
	{
		return false;
	}

	// Check cooldown
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastTriggerTime < CooldownTime)
	{
		return false;
	}

	// Check range
	float Distance = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
	if (Distance > TriggerRange)
	{
		return false;
	}

	// Trigger trap
	LastTriggerTime = CurrentTime;
	ApplyTrapEffects(TargetActor);

	UE_LOG(LogNet, Log, TEXT("ABuildingSMActorTrap: Triggered on target %s"), *TargetActor->GetName());

	return true;
}

void ABuildingSMActorTrap::ApplyTrapEffects(AActor* TargetActor)
{
	if (!TargetActor || Role != ROLE_Authority)
	{
		return;
	}

	// Apply damage to target
	FDamageEvent DamageEvent;
	TargetActor->TakeDamage(TrapDamage, DamageEvent, nullptr, this);

	UE_LOG(LogNet, Log, TEXT("ABuildingSMActorTrap: Applied %.2f damage to %s"), TrapDamage, *TargetActor->GetName());
}

void ABuildingSMActorTrap::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABuildingSMActorTrap, bIsArmed);
	DOREPLIFETIME(ABuildingSMActorTrap, LastTriggerTime);
}

//////////////////////////////////////////////////////////////////////////
// ABuildingSMActorContainer

ABuildingSMActorContainer::ABuildingSMActorContainer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, LootTableName(NAME_None)
	, bHasBeenOpened(false)
	, bHasLoot(true)
{
	BuildingType = EFortBuildingType::Container;

	// Containers have ContainerLoot
}

void ABuildingSMActorContainer::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ABuildingSMActorContainer::OpenContainer(APlayerController* Opener)
{
	if (Role != ROLE_Authority)
	{
		return;
	}

	if (bHasBeenOpened)
	{
		UE_LOG(LogNet, Log, TEXT("ABuildingSMActorContainer: Container already opened"));
		return;
	}

	if (!Opener)
	{
		UE_LOG(LogNet, Warning, TEXT("ABuildingSMActorContainer: No opener specified"));
		return;
	}

	bHasBeenOpened = true;

	UE_LOG(LogNet, Log, TEXT("ABuildingSMActorContainer: Opened by %s"), *Opener->GetName());

	if (bHasLoot)
	{
		SpawnLoot();
	}
}

void ABuildingSMActorContainer::SpawnLoot()
{
	if (Role != ROLE_Authority)
	{
		return;
	}

	UE_LOG(LogNet, Log, TEXT("ABuildingSMActorContainer: SpawnLoot from table: %s"), *LootTableName.ToString());

	// Spawn loot items based on LootTableName
	// Would use loot table system to determine items

	bHasLoot = false;
}

void ABuildingSMActorContainer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABuildingSMActorContainer, bHasBeenOpened);
	DOREPLIFETIME(ABuildingSMActorContainer, bHasLoot);
}

//////////////////////////////////////////////////////////////////////////
// ABuildingSMActorSpawnedItem

ABuildingSMActorSpawnedItem::ABuildingSMActorSpawnedItem(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ItemDefinitionName(NAME_None)
	, ItemQuantity(1)
{
	BuildingType = EFortBuildingType::SpawnedItem;

	// Spawned items are small pickups
}

void ABuildingSMActorSpawnedItem::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ABuildingSMActorSpawnedItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABuildingSMActorSpawnedItem, ItemDefinitionName);
	DOREPLIFETIME(ABuildingSMActorSpawnedItem, ItemQuantity);
}
