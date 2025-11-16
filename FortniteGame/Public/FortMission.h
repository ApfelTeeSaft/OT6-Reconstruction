// Copyright Epic Games, Inc. All Rights Reserved.
// Source: D:\BuildFarm\buildmachine_++Fortnite+Release-Live\FortniteGame\Source\FortniteGame\Private\Missions\

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Info.h"
#include "Engine/DataAsset.h"
#include "Components/ActorComponent.h"
#include "FortEnums.h"
#include "FortMission.generated.h"

// Forward declarations
class AFortPlayerController;
class AFortAIDirector;
class UFortEncounterInfo;

/**
 * EFortMissionStatus - Mission state enum
 */
UENUM(BlueprintType)
enum class EFortMissionStatus : uint8
{
	Created UMETA(DisplayName = "Created"),
	InProgress UMETA(DisplayName = "In Progress"),
	Succeeded UMETA(DisplayName = "Succeeded"),
	Failed UMETA(DisplayName = "Failed"),
	NeutralCompletion UMETA(DisplayName = "Neutral Completion"),
	Max_None UMETA(Hidden),
	EFortMissionStatus_MAX UMETA(Hidden)
};

/**
 * EFortMissionType - Mission category
 */
UENUM(BlueprintType)
enum class EFortMissionType : uint8
{
	Primary UMETA(DisplayName = "Primary Mission"),
	Secondary UMETA(DisplayName = "Secondary Mission"),
	Max_None UMETA(Hidden),
	EFortMissionType_MAX UMETA(Hidden)
};

/**
 * EMissionGenerationCategory - Mission generation categories
 */
UENUM(BlueprintType)
enum class EMissionGenerationCategory : uint8
{
	Primary UMETA(DisplayName = "Primary"),
	Secondary UMETA(DisplayName = "Secondary"),
	Tertiary UMETA(DisplayName = "Tertiary"),
	Max_None UMETA(Hidden),
	EMissionGenerationCategory_MAX UMETA(Hidden)
};

/**
 * EMissionReplyTypes - Mission event reply types
 */
UENUM(BlueprintType)
enum class EMissionReplyTypes : uint8
{
	Handled UMETA(DisplayName = "Handled"),
	NotHandled UMETA(DisplayName = "Not Handled"),
	EMissionReplyTypes_MAX UMETA(Hidden)
};

/**
 * FFortObjectiveEntry - Single objective entry
 */
USTRUCT(BlueprintType)
struct FFortObjectiveEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Objective")
	FText ObjectiveText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Objective")
	int32 RequiredCount;

	UPROPERTY(BlueprintReadOnly, Category = "Mission|Objective")
	int32 CurrentCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Objective")
	bool bIsOptional;

	UPROPERTY(BlueprintReadOnly, Category = "Mission|Objective")
	bool bIsCompleted;

	FFortObjectiveEntry()
		: RequiredCount(1)
		, CurrentCount(0)
		, bIsOptional(false)
		, bIsCompleted(false)
	{}
};

/**
 * FFortObjectiveBlock - Block of mission objectives
 */
USTRUCT(BlueprintType)
struct FFortObjectiveBlock
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Objective")
	FText BlockName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Objective")
	TArray<FFortObjectiveEntry> Objectives;

	UPROPERTY(BlueprintReadOnly, Category = "Mission|Objective")
	bool bIsActive;

	UPROPERTY(BlueprintReadOnly, Category = "Mission|Objective")
	bool bIsCompleted;

	FFortObjectiveBlock()
		: bIsActive(false)
		, bIsCompleted(false)
	{}
};

/**
 * FFortMissionActorRecord - Record of spawned mission actors
 */
USTRUCT()
struct FFortMissionActorRecord
{
	GENERATED_BODY()

	UPROPERTY()
	AActor* SpawnedActor;

	UPROPERTY()
	FGuid ActorGuid;

	UPROPERTY()
	FVector SpawnLocation;

	UPROPERTY()
	FRotator SpawnRotation;

	FFortMissionActorRecord()
		: SpawnedActor(nullptr)
		, SpawnLocation(FVector::ZeroVector)
		, SpawnRotation(FRotator::ZeroRotator)
	{}
};

/**
 * FFortMissionSave - Mission save data
 */
USTRUCT()
struct FFortMissionSave
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid MissionGuid;

	UPROPERTY()
	EFortMissionStatus Status;

	UPROPERTY()
	TArray<FFortObjectiveBlock> ObjectiveProgress;

	UPROPERTY()
	float MissionElapsedTime;

	UPROPERTY()
	int32 ChosenRewardIdx;

	FFortMissionSave()
		: Status(EFortMissionStatus::Created)
		, MissionElapsedTime(0.0f)
		, ChosenRewardIdx(0)
	{}
};

/**
 * UFortMissionEventParams - Mission event parameters
 */
UCLASS(BlueprintType)
class FORTNITEGAME_API UFortMissionEventParams : public UObject
{
	GENERATED_BODY()

public:
	UFortMissionEventParams();

	UPROPERTY(BlueprintReadWrite, Category = "Mission|Event")
	FName EventName;

	UPROPERTY(BlueprintReadWrite, Category = "Mission|Event")
	AActor* EventInstigator;

	UPROPERTY(BlueprintReadWrite, Category = "Mission|Event")
	TMap<FName, FString> EventData;
};

/**
 * UFortMissionConfigData - Mission configuration data asset
 */
UCLASS(BlueprintType)
class FORTNITEGAME_API UFortMissionConfigData : public UDataAsset
{
	GENERATED_BODY()

public:
	UFortMissionConfigData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	EFortMissionType MissionType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	FText MissionName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	FText MissionDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	TArray<FFortObjectiveBlock> ObjectiveBlocks;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	float TimeLimit;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	int32 MinPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	int32 MaxPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	int32 DifficultyLevel;
};

/**
 * UFortMissionInfo - Mission instance data
 */
UCLASS(BlueprintType)
class FORTNITEGAME_API UFortMissionInfo : public UObject
{
	GENERATED_BODY()

public:
	UFortMissionInfo();

	UPROPERTY(BlueprintReadOnly, Category = "Mission")
	FGuid MissionGuid;

	UPROPERTY(BlueprintReadOnly, Category = "Mission")
	UFortMissionConfigData* MissionConfig;

	UPROPERTY(BlueprintReadOnly, Category = "Mission")
	EFortMissionStatus MissionStatus;

	UPROPERTY(BlueprintReadOnly, Category = "Mission")
	TArray<FFortObjectiveBlock> ActiveObjectives;

	UPROPERTY(BlueprintReadOnly, Category = "Mission")
	float MissionActivationTime;

	UPROPERTY(BlueprintReadOnly, Category = "Mission")
	float MissionElapsedTime;

	UPROPERTY(BlueprintReadOnly, Category = "Mission")
	TArray<FFortMissionActorRecord> MissionActorRecords;

	/** Get current objective block */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	FFortObjectiveBlock GetCurrentObjectiveBlock() const;

	/** Check if mission is running */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	bool IsMissionRunning() const;

	/** Get mission progress percentage (0-100) */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	float GetMissionProgress() const;
};

/**
 * UFortMissionWeightedRewards - Weighted reward selection
 */
UCLASS(BlueprintType)
class FORTNITEGAME_API UFortMissionWeightedRewards : public UDataAsset
{
	GENERATED_BODY()

public:
	UFortMissionWeightedRewards();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards")
	TMap<FName, float> RewardWeights;

	UFUNCTION(BlueprintCallable, Category = "Rewards")
	FName SelectReward() const;
};

/**
 * UFortMissionTimerComponent - Mission timer component
 */
UCLASS(ClassGroup = (Fortnite), meta = (BlueprintSpawnableComponent))
class FORTNITEGAME_API UFortMissionTimerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFortMissionTimerComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintReadOnly, Category = "Mission|Timer")
	float RemainingTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Timer")
	bool bIsPaused;

	UFUNCTION(BlueprintCallable, Category = "Mission|Timer")
	void StartTimer(float Duration);

	UFUNCTION(BlueprintCallable, Category = "Mission|Timer")
	void PauseTimer();

	UFUNCTION(BlueprintCallable, Category = "Mission|Timer")
	void ResumeTimer();

	UFUNCTION(BlueprintCallable, Category = "Mission|Timer")
	void StopTimer();

	UPROPERTY(BlueprintAssignable, Category = "Mission|Timer")
	FOnTimerComplete OnTimerComplete;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimerComplete);

protected:
	bool bIsRunning;
};

/**
 * AFortMission - Main mission actor
 */
UCLASS(Blueprintable)
class FORTNITEGAME_API AFortMission : public AInfo
{
	GENERATED_BODY()

public:
	AFortMission();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Mission configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	UFortMissionInfo* MissionInfo;

	/** Current mission status */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MissionStatus, Category = "Mission")
	EFortMissionStatus MissionStatus;

	/** Objective blocks */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Mission")
	TArray<FFortObjectiveBlock> ObjectiveBlocks;

	/** Timer component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission")
	UFortMissionTimerComponent* MissionTimerComponent;

	/**
	 * Start the mission
	 */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	void StartMission();

	/**
	 * Complete the mission with status
	 */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	void CompleteMission(EFortMissionStatus CompletionStatus);

	/**
	 * Fail the mission
	 */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	void FailMission();

	/**
	 * Update objective progress
	 */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	void UpdateObjectiveProgress(int32 BlockIndex, int32 ObjectiveIndex, int32 Count);

	/**
	 * Populate mission save data
	 */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	void PopulateMissionSave(FFortMissionSave& OutSave);

	/**
	 * Spawn mission actors at placement points
	 */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	void SpawnMissionActors();

	/** Mission status changed event */
	UPROPERTY(BlueprintAssignable, Category = "Mission")
	FOnMissionStatusChanged OnMissionStatusChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionStatusChanged, EFortMissionStatus, OldStatus, EFortMissionStatus, NewStatus);

	/** Objective completed event */
	UPROPERTY(BlueprintAssignable, Category = "Mission")
	FOnObjectiveCompleted OnObjectiveCompleted;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveCompleted, int32, BlockIndex, int32, ObjectiveIndex);

protected:
	UFUNCTION()
	void OnRep_MissionStatus(EFortMissionStatus OldStatus);

	UFUNCTION()
	void OnTimerExpired();

	void CheckObjectiveCompletion();
	void ActivateNextObjectiveBlock();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	int32 CurrentObjectiveBlockIndex;
	float MissionStartTime;
};

/**
 * UFortMissionLibrary - Mission utility functions
 */
UCLASS()
class FORTNITEGAME_API UFortMissionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Start AI encounter for mission
	 */
	UFUNCTION(BlueprintCallable, Category = "Mission|AI", meta = (WorldContext = "WorldContextObject"))
	static bool StartAIEncounter(UObject* WorldContextObject, UFortEncounterInfo* EncounterTemplate);

	/**
	 * Stop active AI encounter
	 */
	UFUNCTION(BlueprintCallable, Category = "Mission|AI", meta = (WorldContext = "WorldContextObject"))
	static void StopAIEncounter(UObject* WorldContextObject);

	/**
	 * Stop AI encounter against specific actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Mission|AI", meta = (WorldContext = "WorldContextObject"))
	static void StopAIEncounterAgainstActor(UObject* WorldContextObject, AActor* TargetActor);

	/**
	 * Spawn mission actors at placement points
	 */
	UFUNCTION(BlueprintCallable, Category = "Mission", meta = (WorldContext = "WorldContextObject"))
	static void SpawnAtPlacementActors(UObject* WorldContextObject, AFortMission* Mission);

	/**
	 * Start tutorial notification
	 */
	UFUNCTION(BlueprintCallable, Category = "Mission|Tutorial")
	static void StartTutorialNotification(AFortPlayerController* Player, const FText& NotificationText);

	/**
	 * Give mission rewards to player
	 */
	UFUNCTION(BlueprintCallable, Category = "Mission|Rewards")
	static void GiveMissionRewardsToPlayer(AFortPlayerController* Player, UFortMissionInfo* MissionInfo);

	/**
	 * Give mission rewards as pickups
	 */
	UFUNCTION(BlueprintCallable, Category = "Mission|Rewards")
	static void GiveMissionRewardsToPlayerAsPickups(AFortPlayerController* Player, UFortMissionInfo* MissionInfo, FVector SpawnLocation);
};

/**
 * AFortMissionManager - Mission management actor
 */
UCLASS()
class FORTNITEGAME_API AFortMissionManager : public AInfo
{
	GENERATED_BODY()

public:
	AFortMissionManager();

	virtual void BeginPlay() override;

	/** Active missions */
	UPROPERTY(BlueprintReadOnly, Category = "Mission")
	TArray<AFortMission*> ActiveMissions;

	/** Start a new mission */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	AFortMission* StartNewMission(UFortMissionConfigData* MissionConfig);

	/** Stop mission by info */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	void StopMission(UFortMissionInfo* MissionInfo);

	/** Get active mission by info */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	AFortMission* GetMissionByInfo(UFortMissionInfo* MissionInfo) const;

	/** Check if mission is running */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	bool IsMissionRunning(UFortMissionInfo* MissionInfo) const;

protected:
	void OnMissionCompleted(AFortMission* Mission);
};
