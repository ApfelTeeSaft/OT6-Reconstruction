// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite Specific Building Types - Wall, Floor, Stairs, Roof, Trap
// Extracted from EFortBuildingType enum values

#pragma once

#include "CoreMinimal.h"
#include "BuildingActor.h"
#include "BuildingActorTypes.generated.h"

/**
 * ABuildingSMActorWall - Wall building piece
 */
UCLASS()
class FORTNITEGAME_API ABuildingSMActorWall : public ABuildingSMActor
{
	GENERATED_BODY()

public:
	ABuildingSMActorWall(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;
	virtual float DetermineHealthMax() override;
	virtual int32 DetermineMaxResourcesToSpawn() override;
};

/**
 * ABuildingSMActorFloor - Floor building piece
 */
UCLASS()
class FORTNITEGAME_API ABuildingSMActorFloor : public ABuildingSMActor
{
	GENERATED_BODY()

public:
	ABuildingSMActorFloor(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;
	virtual float DetermineHealthMax() override;
	virtual int32 DetermineMaxResourcesToSpawn() override;
};

/**
 * ABuildingSMActorStairs - Stairs building piece
 */
UCLASS()
class FORTNITEGAME_API ABuildingSMActorStairs : public ABuildingSMActor
{
	GENERATED_BODY()

public:
	ABuildingSMActorStairs(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;
	virtual float DetermineHealthMax() override;
	virtual int32 DetermineMaxResourcesToSpawn() override;
};

/**
 * ABuildingSMActorRoof - Roof building piece
 */
UCLASS()
class FORTNITEGAME_API ABuildingSMActorRoof : public ABuildingSMActor
{
	GENERATED_BODY()

public:
	ABuildingSMActorRoof(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;
	virtual float DetermineHealthMax() override;
	virtual int32 DetermineMaxResourcesToSpawn() override;
};

/**
 * ABuildingSMActorPillar - Pillar/column building piece
 */
UCLASS()
class FORTNITEGAME_API ABuildingSMActorPillar : public ABuildingSMActor
{
	GENERATED_BODY()

public:
	ABuildingSMActorPillar(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;
	virtual float DetermineHealthMax() override;
	virtual int32 DetermineMaxResourcesToSpawn() override;
};

/**
 * ABuildingSMActorTrap - Trap building piece
 */
UCLASS()
class FORTNITEGAME_API ABuildingSMActorTrap : public ABuildingSMActor
{
	GENERATED_BODY()

public:
	ABuildingSMActorTrap(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;
	virtual float DetermineHealthMax() override;
	virtual int32 DetermineMaxResourcesToSpawn() override;

	/** Trap activation properties */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Trap")
	float TriggerRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Trap")
	float TrapDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Trap")
	float CooldownTime;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Trap")
	bool bIsArmed;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Trap")
	float LastTriggerTime;

	/**
	 * Attempt to trigger trap
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Trap")
	virtual bool TryTriggerTrap(AActor* TargetActor);

	/**
	 * Apply trap effects to target
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Trap")
	virtual void ApplyTrapEffects(AActor* TargetActor);

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

/**
 * ABuildingSMActorContainer - Container building (loot chest, etc.)
 */
UCLASS()
class FORTNITEGAME_API ABuildingSMActorContainer : public ABuildingSMActor
{
	GENERATED_BODY()

public:
	ABuildingSMActorContainer(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

	/** Container properties */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Container")
	FName LootTableName;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Container")
	bool bHasBeenOpened;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Container")
	bool bHasLoot;

	/**
	 * Open container and spawn loot
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Container")
	virtual void OpenContainer(APlayerController* Opener);

	/**
	 * Spawn loot from container
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Container")
	virtual void SpawnLoot();

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

/**
 * ABuildingSMActorSpawnedItem - Spawned item building
 */
UCLASS()
class FORTNITEGAME_API ABuildingSMActorSpawnedItem : public ABuildingSMActor
{
	GENERATED_BODY()

public:
	ABuildingSMActorSpawnedItem(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

	/** Spawned item properties */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Item")
	FName ItemDefinitionName;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Item")
	int32 ItemQuantity;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
