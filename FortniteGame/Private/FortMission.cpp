// Copyright Epic Games, Inc. All Rights Reserved.
// Source: D:\BuildFarm\buildmachine_++Fortnite+Release-Live\FortniteGame\Source\FortniteGame\Private\Missions\

#include "FortMission.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "FortPlayerController.h"
#include "FortEncounter.h"

// ============================================================================
// UFortMissionEventParams
// ============================================================================

UFortMissionEventParams::UFortMissionEventParams()
	: EventInstigator(nullptr)
{
}

// ============================================================================
// UFortMissionConfigData
// ============================================================================

UFortMissionConfigData::UFortMissionConfigData()
	: MissionType(EFortMissionType::Primary)
	, TimeLimit(0.0f)
	, MinPlayers(1)
	, MaxPlayers(4)
	, DifficultyLevel(1)
{
}

// ============================================================================
// UFortMissionInfo
// ============================================================================

UFortMissionInfo::UFortMissionInfo()
	: MissionConfig(nullptr)
	, MissionStatus(EFortMissionStatus::Created)
	, MissionActivationTime(0.0f)
	, MissionElapsedTime(0.0f)
{
	MissionGuid = FGuid::NewGuid();
}

FFortObjectiveBlock UFortMissionInfo::GetCurrentObjectiveBlock() const
{
	for (const FFortObjectiveBlock& Block : ActiveObjectives)
	{
		if (Block.bIsActive && !Block.bIsCompleted)
		{
			return Block;
		}
	}

	return FFortObjectiveBlock();
}

bool UFortMissionInfo::IsMissionRunning() const
{
	return MissionStatus == EFortMissionStatus::InProgress;
}

float UFortMissionInfo::GetMissionProgress() const
{
	if (ActiveObjectives.Num() == 0)
	{
		return 0.0f;
	}

	int32 TotalObjectives = 0;
	int32 CompletedObjectives = 0;

	for (const FFortObjectiveBlock& Block : ActiveObjectives)
	{
		for (const FFortObjectiveEntry& Objective : Block.Objectives)
		{
			if (!Objective.bIsOptional)
			{
				TotalObjectives++;
				if (Objective.bIsCompleted)
				{
					CompletedObjectives++;
				}
			}
		}
	}

	return TotalObjectives > 0 ? (float)CompletedObjectives / (float)TotalObjectives * 100.0f : 0.0f;
}

// ============================================================================
// UFortMissionWeightedRewards
// ============================================================================

UFortMissionWeightedRewards::UFortMissionWeightedRewards()
{
}

FName UFortMissionWeightedRewards::SelectReward() const
{
	if (RewardWeights.Num() == 0)
	{
		return NAME_None;
	}

	// Calculate total weight
	float TotalWeight = 0.0f;
	for (const auto& Pair : RewardWeights)
	{
		TotalWeight += Pair.Value;
	}

	// Random selection
	float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
	float CurrentWeight = 0.0f;

	for (const auto& Pair : RewardWeights)
	{
		CurrentWeight += Pair.Value;
		if (RandomValue <= CurrentWeight)
		{
			return Pair.Key;
		}
	}

	return NAME_None;
}

// ============================================================================
// UFortMissionTimerComponent
// ============================================================================

UFortMissionTimerComponent::UFortMissionTimerComponent()
	: RemainingTime(0.0f)
	, bIsPaused(false)
	, bIsRunning(false)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFortMissionTimerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsRunning || bIsPaused)
	{
		return;
	}

	RemainingTime -= DeltaTime;

	if (RemainingTime <= 0.0f)
	{
		RemainingTime = 0.0f;
		bIsRunning = false;
		OnTimerComplete.Broadcast();
	}
}

void UFortMissionTimerComponent::StartTimer(float Duration)
{
	RemainingTime = Duration;
	bIsRunning = true;
	bIsPaused = false;
	UE_LOG(LogTemp, Log, TEXT("Mission timer started: %.1f seconds"), Duration);
}

void UFortMissionTimerComponent::PauseTimer()
{
	bIsPaused = true;
	UE_LOG(LogTemp, Log, TEXT("Mission timer paused: %.1f seconds remaining"), RemainingTime);
}

void UFortMissionTimerComponent::ResumeTimer()
{
	bIsPaused = false;
	UE_LOG(LogTemp, Log, TEXT("Mission timer resumed: %.1f seconds remaining"), RemainingTime);
}

void UFortMissionTimerComponent::StopTimer()
{
	bIsRunning = false;
	bIsPaused = false;
	RemainingTime = 0.0f;
	UE_LOG(LogTemp, Log, TEXT("Mission timer stopped"));
}

// ============================================================================
// AFortMission
// ============================================================================

AFortMission::AFortMission()
	: MissionInfo(nullptr)
	, MissionStatus(EFortMissionStatus::Created)
	, CurrentObjectiveBlockIndex(0)
	, MissionStartTime(0.0f)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Create timer component
	MissionTimerComponent = CreateDefaultSubobject<UFortMissionTimerComponent>(TEXT("MissionTimerComponent"));
}

void AFortMission::BeginPlay()
{
	Super::BeginPlay();

	if (MissionTimerComponent)
	{
		MissionTimerComponent->OnTimerComplete.AddDynamic(this, &AFortMission::OnTimerExpired);
	}
}

void AFortMission::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MissionStatus == EFortMissionStatus::InProgress && MissionInfo)
	{
		MissionInfo->MissionElapsedTime += DeltaTime;
	}
}

void AFortMission::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFortMission, MissionStatus);
	DOREPLIFETIME(AFortMission, ObjectiveBlocks);
}

void AFortMission::StartMission()
{
	if (!MissionInfo || !MissionInfo->MissionConfig)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot start mission - MissionInfo or MissionConfig is null"));
		return;
	}

	if (MissionStatus == EFortMissionStatus::InProgress)
	{
		UE_LOG(LogTemp, Warning, TEXT("MissionInfo: %s is already running!"), *MissionInfo->MissionConfig->MissionName.ToString());
		return;
	}

	if (MissionInfo->MissionConfig->ObjectiveBlocks.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Mission Info is Null or Number of Objective Blocks is 0 in Mission %s!! Setting to Silent Fail!!"),
			*GetName());
		return;
	}

	EFortMissionStatus OldStatus = MissionStatus;
	MissionStatus = EFortMissionStatus::InProgress;

	// Initialize objectives
	ObjectiveBlocks = MissionInfo->MissionConfig->ObjectiveBlocks;
	MissionInfo->ActiveObjectives = ObjectiveBlocks;

	// Activate first objective block
	if (ObjectiveBlocks.Num() > 0)
	{
		ObjectiveBlocks[0].bIsActive = true;
		CurrentObjectiveBlockIndex = 0;
	}

	// Start timer if time limit is set
	if (MissionInfo->MissionConfig->TimeLimit > 0.0f && MissionTimerComponent)
	{
		MissionTimerComponent->StartTimer(MissionInfo->MissionConfig->TimeLimit);
	}

	MissionStartTime = GetWorld()->GetTimeSeconds();
	MissionInfo->MissionActivationTime = MissionStartTime;

	// Spawn mission actors
	SpawnMissionActors();

	UE_LOG(LogTemp, Log, TEXT("Mission %s: New Mission status is %s"),
		*MissionInfo->MissionConfig->MissionName.ToString(),
		*UEnum::GetValueAsString(TEXT("EFortMissionStatus"), MissionStatus));

	OnRep_MissionStatus(OldStatus);
}

void AFortMission::CompleteMission(EFortMissionStatus CompletionStatus)
{
	if (MissionStatus != EFortMissionStatus::InProgress)
	{
		return;
	}

	EFortMissionStatus OldStatus = MissionStatus;
	MissionStatus = CompletionStatus;

	if (MissionTimerComponent)
	{
		MissionTimerComponent->StopTimer();
	}

	UE_LOG(LogTemp, Log, TEXT("Mission completed: %s with status: %s"),
		MissionInfo ? *MissionInfo->MissionConfig->MissionName.ToString() : TEXT("Unknown"),
		*UEnum::GetValueAsString(TEXT("EFortMissionStatus"), CompletionStatus));

	OnRep_MissionStatus(OldStatus);
}

void AFortMission::FailMission()
{
	CompleteMission(EFortMissionStatus::Failed);
}

void AFortMission::UpdateObjectiveProgress(int32 BlockIndex, int32 ObjectiveIndex, int32 Count)
{
	if (!ObjectiveBlocks.IsValidIndex(BlockIndex))
	{
		return;
	}

	FFortObjectiveBlock& Block = ObjectiveBlocks[BlockIndex];
	if (!Block.Objectives.IsValidIndex(ObjectiveIndex))
	{
		return;
	}

	FFortObjectiveEntry& Objective = Block.Objectives[ObjectiveIndex];
	Objective.CurrentCount += Count;

	if (Objective.CurrentCount >= Objective.RequiredCount)
	{
		Objective.CurrentCount = Objective.RequiredCount;
		Objective.bIsCompleted = true;

		OnObjectiveCompleted.Broadcast(BlockIndex, ObjectiveIndex);

		UE_LOG(LogTemp, Log, TEXT("Objective completed: Block %d, Objective %d"), BlockIndex, ObjectiveIndex);
	}

	CheckObjectiveCompletion();
}

void AFortMission::PopulateMissionSave(FFortMissionSave& OutSave)
{
	if (!MissionInfo)
	{
		return;
	}

	if (OutSave.MissionGuid.IsValid() && OutSave.MissionGuid != MissionInfo->MissionGuid)
	{
		UE_LOG(LogTemp, Error, TEXT("AFortMission::PopulateMissionSave, Mission GUIDs don't match when saving %s!"),
			*GetName());
		return;
	}

	OutSave.MissionGuid = MissionInfo->MissionGuid;
	OutSave.Status = MissionStatus;
	OutSave.ObjectiveProgress = ObjectiveBlocks;
	OutSave.MissionElapsedTime = MissionInfo->MissionElapsedTime;

	UE_LOG(LogTemp, Log, TEXT("Populated mission save for: %s"), *GetName());
}

void AFortMission::SpawnMissionActors()
{
	if (!MissionInfo || !MissionInfo->MissionConfig)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("SpawnAtPlacementActors: Attempting to spawn mission actors for mission \"%s\""),
		*MissionInfo->MissionConfig->MissionName.ToString());

	// Mission actor spawning logic would go here
	// This would involve finding placement actors in the world and spawning mission-specific actors
}

void AFortMission::OnRep_MissionStatus(EFortMissionStatus OldStatus)
{
	OnMissionStatusChanged.Broadcast(OldStatus, MissionStatus);
}

void AFortMission::OnTimerExpired()
{
	UE_LOG(LogTemp, Log, TEXT("Mission timer expired for: %s"), MissionInfo ? *MissionInfo->MissionConfig->MissionName.ToString() : TEXT("Unknown"));
	FailMission();
}

void AFortMission::CheckObjectiveCompletion()
{
	if (!ObjectiveBlocks.IsValidIndex(CurrentObjectiveBlockIndex))
	{
		return;
	}

	FFortObjectiveBlock& CurrentBlock = ObjectiveBlocks[CurrentObjectiveBlockIndex];

	// Check if all required objectives in current block are complete
	bool bAllCompleted = true;
	for (const FFortObjectiveEntry& Objective : CurrentBlock.Objectives)
	{
		if (!Objective.bIsOptional && !Objective.bIsCompleted)
		{
			bAllCompleted = false;
			break;
		}
	}

	if (bAllCompleted)
	{
		CurrentBlock.bIsCompleted = true;
		CurrentBlock.bIsActive = false;

		// Activate next block or complete mission
		if (CurrentObjectiveBlockIndex + 1 < ObjectiveBlocks.Num())
		{
			ActivateNextObjectiveBlock();
		}
		else
		{
			// All objectives complete
			CompleteMission(EFortMissionStatus::Succeeded);
		}
	}
}

void AFortMission::ActivateNextObjectiveBlock()
{
	CurrentObjectiveBlockIndex++;

	if (ObjectiveBlocks.IsValidIndex(CurrentObjectiveBlockIndex))
	{
		ObjectiveBlocks[CurrentObjectiveBlockIndex].bIsActive = true;
		UE_LOG(LogTemp, Log, TEXT("Activated objective block %d"), CurrentObjectiveBlockIndex);
	}
}

// ============================================================================
// UFortMissionLibrary
// ============================================================================

bool UFortMissionLibrary::StartAIEncounter(UObject* WorldContextObject, UFortEncounterInfo* EncounterTemplate)
{
	if (!WorldContextObject || !EncounterTemplate)
	{
		UE_LOG(LogTemp, Error, TEXT("UFortMissionLibrary::StartAIEncounter: No EncounterTemplate or WorldContext!"));
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject);
	if (!World)
	{
		return false;
	}

	// Find AI Director
	for (TActorIterator<AFortAIDirector> It(World); It; ++It)
	{
		AFortAIDirector* AIDirector = *It;
		if (AIDirector)
		{
			// Start encounter via AI Director
			UFortEncounterInfo* ActiveEncounter = AIDirector->StartEncounter(EncounterTemplate);
			return ActiveEncounter != nullptr;
		}
	}

	return false;
}

void UFortMissionLibrary::StopAIEncounter(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("UFortMissionLibrary::StopAIEncounter: No ActiveEncounter or WorldContext!"));
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject);
	if (!World)
	{
		return;
	}

	// Find AI Director and stop active encounter
	for (TActorIterator<AFortAIDirector> It(World); It; ++It)
	{
		AFortAIDirector* AIDirector = *It;
		if (AIDirector)
		{
			AIDirector->StopEncounter();
			return;
		}
	}
}

void UFortMissionLibrary::StopAIEncounterAgainstActor(UObject* WorldContextObject, AActor* TargetActor)
{
	if (!WorldContextObject || !TargetActor)
	{
		UE_LOG(LogTemp, Error, TEXT("UFortMissionLibrary::StopAIEncounterAgainstActor: No TargetActor or WorldContext!"));
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject);
	if (!World)
	{
		return;
	}

	// Find AI Director and stop encounter against target
	for (TActorIterator<AFortAIDirector> It(World); It; ++It)
	{
		AFortAIDirector* AIDirector = *It;
		if (AIDirector)
		{
			// Stop encounter logic would go here
			UE_LOG(LogTemp, Log, TEXT("Stopping AI encounter against actor: %s"), *TargetActor->GetName());
			return;
		}
	}
}

void UFortMissionLibrary::SpawnAtPlacementActors(UObject* WorldContextObject, AFortMission* Mission)
{
	if (!WorldContextObject || !Mission)
	{
		return;
	}

	Mission->SpawnMissionActors();
}

void UFortMissionLibrary::StartTutorialNotification(AFortPlayerController* Player, const FText& NotificationText)
{
	if (!Player)
	{
		UE_LOG(LogTemp, Error, TEXT("UFortMissionLibrary::StartTutorialNotification Cannot send team notifications, please specify player."));
		return;
	}

	// Send notification to player
	UE_LOG(LogTemp, Log, TEXT("Tutorial notification: %s"), *NotificationText.ToString());
}

void UFortMissionLibrary::GiveMissionRewardsToPlayer(AFortPlayerController* Player, UFortMissionInfo* MissionInfo)
{
	if (!Player || !MissionInfo)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Giving mission rewards to player: %s"), *Player->GetName());
	// Reward logic would go here
}

void UFortMissionLibrary::GiveMissionRewardsToPlayerAsPickups(AFortPlayerController* Player, UFortMissionInfo* MissionInfo, FVector SpawnLocation)
{
	if (!Player || !MissionInfo)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Giving mission rewards as pickups at: %s"), *SpawnLocation.ToString());
	// Spawn reward pickups at location
}

// ============================================================================
// AFortMissionManager
// ============================================================================

AFortMissionManager::AFortMissionManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AFortMissionManager::BeginPlay()
{
	Super::BeginPlay();
}

AFortMission* AFortMissionManager::StartNewMission(UFortMissionConfigData* MissionConfig)
{
	if (!MissionConfig)
	{
		return nullptr;
	}

	// Create mission info
	UFortMissionInfo* MissionInfo = NewObject<UFortMissionInfo>(this);
	MissionInfo->MissionConfig = MissionConfig;

	// Spawn mission actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	AFortMission* NewMission = GetWorld()->SpawnActor<AFortMission>(AFortMission::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (NewMission)
	{
		NewMission->MissionInfo = MissionInfo;
		ActiveMissions.Add(NewMission);

		NewMission->StartMission();

		UE_LOG(LogTemp, Log, TEXT("Started new mission: %s"), *MissionConfig->MissionName.ToString());
	}

	return NewMission;
}

void AFortMissionManager::StopMission(UFortMissionInfo* MissionInfo)
{
	AFortMission* Mission = GetMissionByInfo(MissionInfo);
	if (Mission)
	{
		Mission->FailMission();
		ActiveMissions.Remove(Mission);
		Mission->Destroy();
	}
}

AFortMission* AFortMissionManager::GetMissionByInfo(UFortMissionInfo* MissionInfo) const
{
	if (!MissionInfo)
	{
		return nullptr;
	}

	for (AFortMission* Mission : ActiveMissions)
	{
		if (Mission && Mission->MissionInfo == MissionInfo)
		{
			return Mission;
		}
	}

	return nullptr;
}

bool AFortMissionManager::IsMissionRunning(UFortMissionInfo* MissionInfo) const
{
	AFortMission* Mission = GetMissionByInfo(MissionInfo);
	return Mission && Mission->MissionStatus == EFortMissionStatus::InProgress;
}

void AFortMissionManager::OnMissionCompleted(AFortMission* Mission)
{
	if (Mission)
	{
		ActiveMissions.Remove(Mission);
		UE_LOG(LogTemp, Log, TEXT("Mission completed and removed from active missions: %s"),
			Mission->MissionInfo ? *Mission->MissionInfo->MissionConfig->MissionName.ToString() : TEXT("Unknown"));
	}
}
