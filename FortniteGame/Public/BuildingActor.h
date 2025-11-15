// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite Building Actor - Base class for all building pieces
// Source: D:\BuildFarm\buildmachine_++Fortnite+Release-Live\FortniteGame\Source\FortniteGame\Private\Building\BuildingActor.cpp

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FortEnums.h"
#include "BuildingActor.generated.h"

/**
 * Building resource/material type
 */
UENUM(BlueprintType)
enum class EBuildingResourceType : uint8
{
	Wood = 0,
	Stone = 1,
	Metal = 2,
	None = 3
};

/**
 * Building edit mode state
 */
UENUM(BlueprintType)
enum class EBuildingEditMode : uint8
{
	None = 0,
	Editing = 1,
	Confirming = 2
};

/**
 * FBuildingHealthInfo - Structure for tracking building health
 * Source: MaxHealth, GetBuildingHealthPercentage references
 */
USTRUCT(BlueprintType)
struct FBuildingHealthInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly)
	float MaxHealth;

	FBuildingHealthInfo()
		: CurrentHealth(100.0f)
		, MaxHealth(100.0f)
	{}

	float GetHealthPercentage() const
	{
		return MaxHealth > 0.0f ? (CurrentHealth / MaxHealth) : 0.0f;
	}
};

/**
 * ABuildingActor - Base class for all building actors
 */
UCLASS(Abstract)
class FORTNITEGAME_API ABuildingActor : public AActor
{
	GENERATED_BODY()

public:
	ABuildingActor(const FObjectInitializer& ObjectInitializer);

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	//~ End AActor Interface

	/**
	 * Called when day phase changes (day/night cycle)
	 */
	UFUNCTION()
	virtual void OnDayPhaseChanged(EFortDayPhase DayPhase);

	/**
	 * Get interaction time required for player interaction
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Building")
	virtual float GetInteractionTime() const;

	/**
	 * Get building health percentage
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Building")
	float GetBuildingHealthPercentage() const;

	/**
	 * Initialize kismet-spawned building actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Building")
	virtual void InitializeKismetSpawnedBuildingActor(EFortBuildingInitializationReason Reason);

	/**
	 * Callback when building actor is initialized
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Fort|Building")
	void OnBuildingActorInitialized();

	/** Building type */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Building")
	EFortBuildingType BuildingType;

	/** Material/resource type */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Building")
	EBuildingResourceType ResourceType;

	/** Health information */
	UPROPERTY(ReplicatedUsing=OnRep_MaxHealth, BlueprintReadOnly, Category = "Fort|Building")
	float MaxHealth;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Building")
	float CurrentHealth;

	/** Team ownership */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Building")
	EFortTeam Team;

	/** Building state */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Building")
	EFortBuildingPersistentState BuildingState;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Building")
	EFortBuildingInitializationReason InitializationReason;

	/** Edit mode */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Building")
	EBuildingEditMode EditMode;

	/** Editor player */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Building")
	APlayerController* CurrentEditor;

	/** Building grid position */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Building")
	FVector GridPosition;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Building")
	FRotator GridRotation;

protected:
	/**
	 * OnRep for MaxHealth changes
	 */
	UFUNCTION()
	virtual void OnRep_MaxHealth();

	/** Interaction delegate */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBuildingActorOnInteract, APlayerController*, InteractingPlayer);

	UPROPERTY(BlueprintAssignable, Category = "Fort|Building")
	FBuildingActorOnInteract OnInteract;
};

/**
 * ABuildingSMActor - Building actor with static mesh (SM = Static Mesh)
 */
UCLASS()
class FORTNITEGAME_API ABuildingSMActor : public ABuildingActor
{
	GENERATED_BODY()

public:
	ABuildingSMActor(const FObjectInitializer& ObjectInitializer);

	//~ Begin AActor Interface
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	//~ End AActor Interface

	//~ Begin ABuildingActor Interface
	virtual void OnDayPhaseChanged(EFortDayPhase DayPhase) override;
	//~ End ABuildingActor Interface

	/**
	 * Determine max health for this building
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Building")
	virtual float DetermineHealthMax();

	/**
	 * Determine maximum resources that can be spawned when destroyed
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Building")
	virtual int32 DetermineMaxResourcesToSpawn();

	/**
	 * Get health modifier based on expected player level
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Building")
	virtual float GetHealthModifierBasedOnExpectedLevel(int32 ExpectedLevel);

	/**
	 * Update repair material animation
	 */
	UFUNCTION()
	virtual void UpdateRepairMaterialAnim();

	/**
	 * Select mesh set based on loot tier
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Building")
	virtual void SelectMeshSet(int32 LootTier);

	/**
	 * Select mesh set based on loot tier key name
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Building")
	virtual void SelectMeshSetByName(FName LootTierKey);

	/**
	 * Attempt to spawn resources when destroyed
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Building")
	virtual void AttemptSpawnResources();

	/**
	 * Post-update callback
	 */
	virtual void PostUpdate();

	/**
	 * Update LOD override effect
	 */
	UFUNCTION()
	virtual void UpdateLODOverrideEffect();

	/**
	 * Called when LOD override effect finishes
	 */
	UFUNCTION()
	virtual void OnLODOverrideEffectFinished();

	/**
	 * Update dynamic shrink and destroy effect
	 */
	UFUNCTION()
	virtual void UpdateDynamicShrinkAndDestroyEffect();

	/**
	 * Called when dynamic shrink and destroy effect finishes
	 */
	UFUNCTION()
	virtual void OnDynamicShrinkAndDestroyEffectFinished();

	/** Static mesh component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fort|Building")
	class UStaticMeshComponent* StaticMeshComponent;

	/** Alternative mesh index for mesh sets */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Building")
	int32 AltMeshIdx;

	/** Loot tier for visual variation */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Building")
	int32 LootTier;

	/** Resource amounts to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Building")
	int32 WoodToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Building")
	int32 StoneToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Building")
	int32 MetalToSpawn;

	/** Health modifiers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Building")
	float HealthModifierPerLevel;

protected:
	/** Effect timers */
	FTimerHandle RepairAnimTimerHandle;
	FTimerHandle LODEffectTimerHandle;
	FTimerHandle DestroyEffectTimerHandle;

	/** Visual state */
	UPROPERTY()
	bool bPlayingDestroyEffect;

	UPROPERTY()
	bool bPlayingLODEffect;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

/**
 * AStrategicBuildingActor - Strategic building (mission objective buildings)
 */
UCLASS()
class FORTNITEGAME_API AStrategicBuildingActor : public ABuildingSMActor
{
	GENERATED_BODY()

public:
	AStrategicBuildingActor(const FObjectInitializer& ObjectInitializer);

	/** Strategic value for AI targeting */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Strategic")
	float StrategicValue;

	/** Whether this is a mission objective */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Strategic")
	bool bIsObjective;

	/** Team strategic building references */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fort|Strategic")
	TArray<AStrategicBuildingActor*> TeamStrategicBuildings;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
