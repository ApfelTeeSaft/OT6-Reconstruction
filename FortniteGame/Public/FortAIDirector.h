// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite AI Director System - Encounter Spawning and Difficulty Management
// Source: D:\BuildFarm\buildmachine_++Fortnite+Release-Live\FortniteGame\Source\FortniteGame\Private\AI\FortAIDirector.cpp

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Info.h"
#include "Engine/DataAsset.h"
#include "Engine/CurveTable.h"
#include "FortEnums.h"
#include "FortAIDirector.generated.h"

// Forward declarations
class AFortAIPawn;
class AFortAIController;
class UFortAISpawnGroup;
class UFortAIEncounterInfo;
class AFortMission;

/**
 * EFortEncounterPacingMode - Pacing mode for encounters
 */
UENUM(BlueprintType)
enum class EFortEncounterPacingMode : uint8
{
	SpawnPointsPercentage = 0,
	IntensityCurve = 1,
	Burst = 2,
	Fixed = 3,
};

/**
 * FFortAIEncounterSpawnGroupCap - Spawn group cap configuration
 */
USTRUCT(BlueprintType)
struct FFortAIEncounterSpawnGroupCap
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SpawnGroupName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxActiveAI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxSpawnedDuringEncounter;
};

/**
 * FFortAIEncounterSpawnGroupCapsCategory - Category-based spawn caps
 */
USTRUCT(BlueprintType)
struct FFortAIEncounterSpawnGroupCapsCategory
{
	GENERATED_BODY()

	/**
	 * Get max spawn groups for this category
	 */
	int32 GetMaxSpawnGroups() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CategoryName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FFortAIEncounterSpawnGroupCap> SpawnGroupCaps;
};

/**
 * FFortAIEncounterSpawnGroupCapsProfile - Profile containing all spawn caps
 */
USTRUCT(BlueprintType)
struct FFortAIEncounterSpawnGroupCapsProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FFortAIEncounterSpawnGroupCapsCategory> Categories;
};

/**
 * FSpawnGroupInstanceInfo - Instance information for spawn groups
 */
USTRUCT(BlueprintType)
struct FSpawnGroupInstanceInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGuid SpawnGroupGuid;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentActiveAI;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalSpawnedDuringEncounter;

	UPROPERTY(BlueprintReadOnly)
	bool bIsReadyToSpawnAI;
};

/**
 * FFortSpawnAIRequest - Request to spawn AI
 */
USTRUCT(BlueprintType)
struct FFortSpawnAIRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	UFortAISpawnGroup* SpawnGroup;

	UPROPERTY(BlueprintReadWrite)
	int32 EnemyIndexInSpawnGroup;

	UPROPERTY(BlueprintReadWrite)
	FVector SpawnLocation;

	UPROPERTY(BlueprintReadWrite)
	FRotator SpawnRotation;
};

/**
 * UFortAISpawnGroup - Spawn group data asset
 */
UCLASS(Blueprintable)
class FORTNITEGAME_API UFortAISpawnGroup : public UDataAsset
{
	GENERATED_BODY()

public:
	UFortAISpawnGroup(const FObjectInitializer& ObjectInitializer);

	/**
	 * Get group respawn delay
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	float GetGroupRespawnDelay() const;

	/**
	 * Get category max population density
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	float GetCategoryMaxPopulationDensity() const;

	/** Enemies to spawn in this group */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpawnGroup")
	TArray<TSubclassOf<AFortAIPawn>> EnemyClasses;

	/** Number of each enemy type to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpawnGroup")
	TArray<int32> EnemyCounts;

	/** Respawn delay after group is defeated */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpawnGroup")
	float RespawnDelay;

	/** Max population density for this group's category */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpawnGroup")
	float MaxPopulationDensity;

	/** Is this a large spawn group? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpawnGroup")
	bool bIsLargeSpawnGroup;
};

/**
 * UFortAISpawnGroupProgressionInfo - Spawn group progression over time
 */
UCLASS(Blueprintable)
class FORTNITEGAME_API UFortAISpawnGroupProgressionInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	UFortAISpawnGroupProgressionInfo(const FObjectInitializer& ObjectInitializer);

	/** Spawn groups available at different progression levels */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	TMap<int32, TArray<UFortAISpawnGroup*>> ProgressionLevelSpawnGroups;
};

/**
 * UFortIntensityCurveSequenceProgression - Intensity curve progression
 */
UCLASS(Blueprintable)
class FORTNITEGAME_API UFortIntensityCurveSequenceProgression : public UDataAsset
{
	GENERATED_BODY()

public:
	UFortIntensityCurveSequenceProgression(const FObjectInitializer& ObjectInitializer);

	/**
	 * Get intensity curve sequence for current progression
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	UCurveTable* GetIntensityCurveSequence(int32 ProgressionLevel) const;

	/** Curve tables for different progression levels */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intensity")
	TMap<int32, UCurveTable*> IntensityCurveTables;
};

/**
 * UFortAIEncounterInfo - Encounter information and state
 */
UCLASS(Blueprintable)
class FORTNITEGAME_API UFortAIEncounterInfo : public UObject
{
	GENERATED_BODY()

public:
	UFortAIEncounterInfo(const FObjectInitializer& ObjectInitializer);

	/**
	 * Initialize encounter with settings
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	void InitializeEncounter();

	/**
	 * Validate encounter settings
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	bool ValidateEncounterSettings();

	/**
	 * Get current intensity error
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	float GetCurrentIntensityError() const;

	/**
	 * Get current desired hostility level
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	float GetCurrentDesiredHostility() const;

	/**
	 * Start respawn delay for spawn group
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	void StartGroupRespawnDelay(UFortAISpawnGroup* SpawnGroup);

	/**
	 * Get max possible spawn points for encounter
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	int32 GetMaxPossibleSpawnPoints() const;

	/**
	 * Get min possible spawn points for encounter
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	int32 GetMinPossibleSpawnPoints() const;

	/**
	 * Get breather time between waves
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	float GetBreatherTime() const;

	/**
	 * Get utility effectiveness contribution
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	float GetUtilityEffectivenessContribution() const;

	/**
	 * Get spawn group population availability
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	int32 GetSpawnGroupPopulationAvailability(UFortAISpawnGroup* SpawnGroup) const;

	/**
	 * Check if spawn group is spawnable
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	bool IsSpawnGroupSpawnable(UFortAISpawnGroup* SpawnGroup) const;

	/**
	 * Handle goal taking damage
	 */
	UFUNCTION()
	void OnGoalTakeDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, AActor* DamageCauser);

	/** Pacing mode for this encounter */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
	EFortEncounterPacingMode PacingMode;

	/** Spawn caps profile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
	FFortAIEncounterSpawnGroupCapsProfile SpawnCapsProfile;

	/** Current intensity level */
	UPROPERTY(BlueprintReadOnly, Category = "Encounter")
	float CurrentIntensity;

	/** Desired hostility level */
	UPROPERTY(BlueprintReadOnly, Category = "Encounter")
	float DesiredHostility;

	/** Active spawn group instances */
	UPROPERTY(BlueprintReadOnly, Category = "Encounter")
	TArray<FSpawnGroupInstanceInfo> ActiveSpawnGroups;

	/** Intensity curve progression */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
	UFortIntensityCurveSequenceProgression* IntensityCurveProgression;

	/** Spawn group progression */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
	UFortAISpawnGroupProgressionInfo* SpawnGroupProgression;

protected:
	/** Current spawn point count */
	int32 CurrentSpawnPoints;

	/** Breather time between waves */
	float BreatherTime;
};

/**
 * AFortAIDirectorDataManager - Manages AI director data and events
 */
UCLASS()
class FORTNITEGAME_API AFortAIDirectorDataManager : public AInfo
{
	GENERATED_BODY()

public:
	AFortAIDirectorDataManager(const FObjectInitializer& ObjectInitializer);

	//~ Begin AActor Interface
	virtual void Tick(float DeltaSeconds) override;
	//~ End AActor Interface

	/**
	 * Trigger AI director event
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	void TriggerEvent(FName EventName, const TArray<AActor*>& EventActors);

	/** Active events */
	UPROPERTY(BlueprintReadOnly, Category = "AIDirector")
	TMap<FName, float> ActiveEvents;
};

/**
 * AFortAIDirectorEventManager - Manages AI director events
 */
UCLASS()
class FORTNITEGAME_API AFortAIDirectorEventManager : public AInfo
{
	GENERATED_BODY()

public:
	AFortAIDirectorEventManager(const FObjectInitializer& ObjectInitializer);

	/** Event handlers */
	UPROPERTY(BlueprintReadOnly, Category = "AIDirector")
	TMap<FName, FTimerHandle> EventTimers;
};

/**
 * AFortAIGoalManager - Manages AI goals and objectives
 */
UCLASS()
class FORTNITEGAME_API AFortAIGoalManager : public AInfo
{
	GENERATED_BODY()

public:
	AFortAIGoalManager(const FObjectInitializer& ObjectInitializer);

	/** Receive goal query result */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	void OnReceiveGoalQueryResult(AFortAIController* Controller, AActor* GoalActor);

	/** Active goals */
	UPROPERTY(BlueprintReadOnly, Category = "AIGoals")
	TArray<AActor*> ActiveGoals;
};

/**
 * AFortAIDirector - Main AI director that coordinates spawning and difficulty
 */
UCLASS()
class FORTNITEGAME_API AFortAIDirector : public AInfo
{
	GENERATED_BODY()

public:
	AFortAIDirector(const FObjectInitializer& ObjectInitializer);

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	//~ End AActor Interface

	/**
	 * Handle day phase changes
	 */
	UFUNCTION()
	void OnDayPhaseChanged(EFortDayPhase NewPhase);

	/**
	 * Spawn AI group
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	bool SpawnAIGroup(UFortAISpawnGroup* SpawnGroup, const FVector& SpawnLocation);

	/**
	 * Spawn AI group from external spawner
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	bool SpawnAIGroupFromExternalSpawner(UFortAISpawnGroup* SpawnGroup, AActor* Spawner);

	/**
	 * Spawn AI group with mutator
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	bool SpawnAIGroupWithMutator(UFortAISpawnGroup* SpawnGroup, FName MutatorName, const FVector& SpawnLocation);

	/**
	 * Check if ready to receive new spawn group
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	bool IsReadyToReceiveNewSpawnGroup() const;

	/**
	 * Spawn AI event callback
	 */
	UFUNCTION()
	void OnSpawnAI(AFortAIPawn* SpawnedPawn);

	/** Data manager reference */
	UPROPERTY(BlueprintReadOnly, Category = "AIDirector")
	AFortAIDirectorDataManager* DataManager;

	/** Event manager reference */
	UPROPERTY(BlueprintReadOnly, Category = "AIDirector")
	AFortAIDirectorEventManager* EventManager;

	/** Goal manager reference */
	UPROPERTY(BlueprintReadOnly, Category = "AIDirector")
	AFortAIGoalManager* GoalManager;

	/** Active encounter */
	UPROPERTY(BlueprintReadOnly, Category = "AIDirector")
	UFortAIEncounterInfo* ActiveEncounter;

	/** All spawned AI pawns */
	UPROPERTY(BlueprintReadOnly, Category = "AIDirector")
	TArray<AFortAIPawn*> SpawnedAIPawns;

	/** Is ready to spawn AI? */
	UPROPERTY(BlueprintReadOnly, Category = "AIDirector")
	bool bIsReadyToSpawnAI;

protected:
	/** Spawn individual AI from request */
	AFortAIPawn* SpawnAIFromRequest(const FFortSpawnAIRequest& Request);

	/** Update encounter intensity */
	void UpdateEncounterIntensity(float DeltaTime);

	/** Process spawn queue */
	void ProcessSpawnQueue(float DeltaTime);

	/** Current spawn queue */
	TArray<FFortSpawnAIRequest> SpawnQueue;
};
