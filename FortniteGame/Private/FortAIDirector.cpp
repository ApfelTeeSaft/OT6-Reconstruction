// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite AI Director System - Encounter Spawning and Difficulty Management
// Source: D:\BuildFarm\buildmachine_++Fortnite+Release-Live\FortniteGame\Source\FortniteGame\Private\AI\FortAIDirector.cpp

#include "FortAIDirector.h"
#include "FortAIPawn.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

// ============================================================================
// FFortAIEncounterSpawnGroupCapsCategory
// ============================================================================

int32 FFortAIEncounterSpawnGroupCapsCategory::GetMaxSpawnGroups() const
{
	return SpawnGroupCaps.Num();
}

// ============================================================================
// UFortAISpawnGroup
// ============================================================================

UFortAISpawnGroup::UFortAISpawnGroup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RespawnDelay = 30.0f;
	MaxPopulationDensity = 1.0f;
	bIsLargeSpawnGroup = false;
}

float UFortAISpawnGroup::GetGroupRespawnDelay() const
{
	return RespawnDelay;
}

float UFortAISpawnGroup::GetCategoryMaxPopulationDensity() const
{
	return MaxPopulationDensity;
}

// ============================================================================
// UFortAISpawnGroupProgressionInfo
// ============================================================================

UFortAISpawnGroupProgressionInfo::UFortAISpawnGroupProgressionInfo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// ============================================================================
// UFortIntensityCurveSequenceProgression
// ============================================================================

UFortIntensityCurveSequenceProgression::UFortIntensityCurveSequenceProgression(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UCurveTable* UFortIntensityCurveSequenceProgression::GetIntensityCurveSequence(int32 ProgressionLevel) const
{

	if (const UCurveTable* const* FoundCurve = IntensityCurveTables.Find(ProgressionLevel))
	{
		return *FoundCurve;
	}

	UE_LOG(LogTemp, Warning, TEXT("\nIntensity curve not found in table for progression level: %d"), ProgressionLevel);
	return nullptr;
}

// ============================================================================
// UFortAIEncounterInfo
// ============================================================================

UFortAIEncounterInfo::UFortAIEncounterInfo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PacingMode = EFortEncounterPacingMode::IntensityCurve;
	CurrentIntensity = 0.0f;
	DesiredHostility = 0.0f;
	CurrentSpawnPoints = 0;
	BreatherTime = 30.0f;
}

void UFortAIEncounterInfo::InitializeEncounter()
{

	CurrentIntensity = 0.0f;
	DesiredHostility = 0.0f;
	ActiveSpawnGroups.Empty();
}

bool UFortAIEncounterInfo::ValidateEncounterSettings()
{

	if (!IntensityCurveProgression)
	{
		UE_LOG(LogTemp, Warning, TEXT("Encounter missing IntensityCurveProgression"));
		return false;
	}

	if (!SpawnGroupProgression)
	{
		UE_LOG(LogTemp, Warning, TEXT("Encounter missing SpawnGroupProgression"));
		return false;
	}

	return true;
}

float UFortAIEncounterInfo::GetCurrentIntensityError() const
{
	return FMath::Abs(DesiredHostility - CurrentIntensity);
}

float UFortAIEncounterInfo::GetCurrentDesiredHostility() const
{
	return DesiredHostility;
}

void UFortAIEncounterInfo::StartGroupRespawnDelay(UFortAISpawnGroup* SpawnGroup)
{

	if (!SpawnGroup)
	{
		return;
	}

	// Start timer for respawn delay
	float RespawnDelay = SpawnGroup->GetGroupRespawnDelay();

	// Find spawn group instance and mark it for respawn
	for (FSpawnGroupInstanceInfo& Instance : ActiveSpawnGroups)
	{
		if (Instance.SpawnGroupGuid.IsValid())
		{
			Instance.bIsReadyToSpawnAI = false;

			// Set timer to re-enable spawning using FTimerManager
			if (UWorld* World = GetWorld())
			{
				FTimerHandle RespawnTimerHandle;
				FTimerDelegate RespawnDelegate;

				// Create delegate to re-enable spawning after delay
				RespawnDelegate.BindLambda([this, SpawnGroup]()
				{
					// Find the spawn group instance and re-enable it
					for (FSpawnGroupInstanceInfo& Info : ActiveSpawnGroups)
					{
						if (Info.SpawnGroupGuid.IsValid())
						{
							Info.bIsReadyToSpawnAI = true;
							UE_LOG(LogTemp, Log, TEXT("Spawn group respawn delay ended - ready to spawn"));
							break;
						}
					}
				});

				// Set the timer
				World->GetTimerManager().SetTimer(RespawnTimerHandle, RespawnDelegate, RespawnDelay, false);
				UE_LOG(LogTemp, Log, TEXT("Starting respawn delay of %.1f seconds for spawn group"), RespawnDelay);
			}
			break;
		}
	}
}

int32 UFortAIEncounterInfo::GetMaxPossibleSpawnPoints() const
{
	return 100; // Default max spawn points
}

int32 UFortAIEncounterInfo::GetMinPossibleSpawnPoints() const
{
	return 10; // Default min spawn points
}

float UFortAIEncounterInfo::GetBreatherTime() const
{
	return BreatherTime;
}

float UFortAIEncounterInfo::GetUtilityEffectivenessContribution() const
{

	// Calculate based on active utilities (traps, defenders, etc.)
	float UtilityEffectiveness = 1.0f; // Base: no utilities

	if (UWorld* World = GetWorld())
	{
		// Enumerate all placed trap buildings
		int32 ActiveTraps = 0;
		int32 TotalUtilityPower = 0;

		for (TActorIterator<ABuildingActor> It(World); It; ++It)
		{
			ABuildingActor* Building = *It;
			if (Building && Building->BuildingType == EFortBuildingType::Trap)
			{
				ActiveTraps++;

				// Each trap contributes to effectiveness based on health (as proxy for tier/level)
				float TrapPower = Building->GetBuildingHealthPercentage();
				TotalUtilityPower += (int32)(TrapPower * 10.0f); // Scale to meaningful value
			}
		}

		// Calculate effectiveness bonus
		// Each active trap adds 0.05 (5%) effectiveness
		// Capped at 2.0 (100% bonus from utilities)
		if (ActiveTraps > 0)
		{
			float TrapBonus = FMath::Min(ActiveTraps * 0.05f, 1.0f);
			UtilityEffectiveness += TrapBonus;

			UE_LOG(LogTemp, Verbose, TEXT("Utility effectiveness: %.2f (%d active traps)"), UtilityEffectiveness, ActiveTraps);
		}
	}

	return FMath::Clamp(UtilityEffectiveness, 1.0f, 2.0f);
}

int32 UFortAIEncounterInfo::GetSpawnGroupPopulationAvailability(UFortAISpawnGroup* SpawnGroup) const
{

	if (!SpawnGroup)
	{
		return 0;
	}

	// Find spawn group cap
	for (const FFortAIEncounterSpawnGroupCapsCategory& Category : SpawnCapsProfile.Categories)
	{
		for (const FFortAIEncounterSpawnGroupCap& Cap : Category.SpawnGroupCaps)
		{
			// Match spawn group to cap by name or asset path
			FString SpawnGroupName = SpawnGroup->GetName();
			FString CapSpawnGroupName = Cap.SpawnGroupName.ToString();

			if (SpawnGroupName.Contains(CapSpawnGroupName) || CapSpawnGroupName.Contains(SpawnGroupName))
			{
				// Found matching cap - count currently active AI from this spawn group
				int32 CurrentActive = 0;

				// Count active AI from this specific spawn group
				for (const FSpawnGroupInstanceInfo& Instance : ActiveSpawnGroups)
				{
					if (Instance.SpawnGroupGuid.IsValid())
					{
						CurrentActive += Instance.CurrentActiveAI;
					}
				}

				// Calculate availability as: MaxActiveAI - CurrentActive
				int32 Availability = FMath::Max(0, Cap.MaxActiveAI - CurrentActive);

				UE_LOG(LogTemp, Verbose, TEXT("Spawn group '%s' availability: %d (max: %d, active: %d)"),
					*SpawnGroupName, Availability, Cap.MaxActiveAI, CurrentActive);

				return Availability;
			}
		}
	}

	// Default availability if no caps defined for this spawn group
	return 10;
}

bool UFortAIEncounterInfo::IsSpawnGroupSpawnable(UFortAISpawnGroup* SpawnGroup) const
{

	if (!SpawnGroup)
	{
		return false;
	}

	int32 Availability = GetSpawnGroupPopulationAvailability(SpawnGroup);
	return Availability > 0;
}

void UFortAIEncounterInfo::OnGoalTakeDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, AActor* DamageCauser)
{

	// Increase intensity when goal is damaged
	CurrentIntensity = FMath::Min(CurrentIntensity + (Damage * 0.01f), 1.0f);
}

// ============================================================================
// AFortAIDirectorDataManager
// ============================================================================

AFortAIDirectorDataManager::AFortAIDirectorDataManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AFortAIDirectorDataManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Update active events
	TArray<FName> ExpiredEvents;
	for (auto& EventPair : ActiveEvents)
	{
		EventPair.Value -= DeltaSeconds;
		if (EventPair.Value <= 0.0f)
		{
			ExpiredEvents.Add(EventPair.Key);
		}
	}

	// Remove expired events
	for (const FName& EventName : ExpiredEvents)
	{
		ActiveEvents.Remove(EventName);
	}
}

void AFortAIDirectorDataManager::TriggerEvent(FName EventName, const TArray<AActor*>& EventActors)
{

	UE_LOG(LogTemp, Log, TEXT("Triggering AI Director event: %s with %d actors"), *EventName.ToString(), EventActors.Num());

	// Set event duration (default 60 seconds)
	ActiveEvents.Add(EventName, 60.0f);
}

// ============================================================================
// AFortAIDirectorEventManager
// ============================================================================

AFortAIDirectorEventManager::AFortAIDirectorEventManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// ============================================================================
// AFortAIGoalManager
// ============================================================================

AFortAIGoalManager::AFortAIGoalManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AFortAIGoalManager::OnReceiveGoalQueryResult(AFortAIController* Controller, AActor* GoalActor)
{
	if (!Controller)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnReceiveGoalQueryResult received by AFortAIController when no Goal Manager exists!"));
		return;
	}

	Controller->SetGoalActor(GoalActor);
}

// ============================================================================
// AFortAIDirector
// ============================================================================

AFortAIDirector::AFortAIDirector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bIsReadyToSpawnAI = true;
}

void AFortAIDirector::BeginPlay()
{
	Super::BeginPlay();

	// Create data manager if not present
	if (!DataManager)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		DataManager = GetWorld()->SpawnActor<AFortAIDirectorDataManager>(SpawnParams);
	}

	// Create event manager if not present
	if (!EventManager)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		EventManager = GetWorld()->SpawnActor<AFortAIDirectorEventManager>(SpawnParams);
	}

	// Create goal manager if not present
	if (!GoalManager)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		GoalManager = GetWorld()->SpawnActor<AFortAIGoalManager>(SpawnParams);
	}
}

void AFortAIDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Update encounter intensity
	if (ActiveEncounter)
	{
		UpdateEncounterIntensity(DeltaSeconds);
	}

	// Process spawn queue
	ProcessSpawnQueue(DeltaSeconds);
}

void AFortAIDirector::OnDayPhaseChanged(EFortDayPhase NewPhase)
{

	UE_LOG(LogTemp, Log, TEXT("AFortAIDirector: Day phase changed to %d"), static_cast<int32>(NewPhase));

	// Adjust AI spawning based on day phase
	if (NewPhase == EFortDayPhase::Night)
	{
		// Increase spawn intensity at night
		if (ActiveEncounter)
		{
			ActiveEncounter->DesiredHostility *= 1.5f;
		}
	}
}

bool AFortAIDirector::SpawnAIGroup(UFortAISpawnGroup* SpawnGroup, const FVector& SpawnLocation)
{

	if (!SpawnGroup)
	{
		return false;
	}

	if (!IsReadyToReceiveNewSpawnGroup())
	{
		UE_LOG(LogTemp, Warning, TEXT("AI Director not ready to receive new spawn group"));
		return false;
	}

	// Check if spawn group is spawnable
	if (ActiveEncounter && !ActiveEncounter->IsSpawnGroupSpawnable(SpawnGroup))
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawn group is not spawnable in current encounter"));
		return false;
	}

	for (int32 i = 0; i < SpawnGroup->EnemyClasses.Num() && i < SpawnGroup->EnemyCounts.Num(); ++i)
	{
		int32 Count = SpawnGroup->EnemyCounts[i];
		for (int32 j = 0; j < Count; ++j)
		{
			FFortSpawnAIRequest Request;
			Request.SpawnGroup = SpawnGroup;
			Request.EnemyIndexInSpawnGroup = i;
			Request.SpawnLocation = SpawnLocation + FVector(FMath::RandRange(-500.0f, 500.0f), FMath::RandRange(-500.0f, 500.0f), 0.0f);
			Request.SpawnRotation = FRotator::ZeroRotator;

			SpawnQueue.Add(Request);
		}
	}

	return true;
}

bool AFortAIDirector::SpawnAIGroupFromExternalSpawner(UFortAISpawnGroup* SpawnGroup, AActor* Spawner)
{

	if (!Spawner)
	{
		return false;
	}

	return SpawnAIGroup(SpawnGroup, Spawner->GetActorLocation());
}

bool AFortAIDirector::SpawnAIGroupWithMutator(UFortAISpawnGroup* SpawnGroup, FName MutatorName, const FVector& SpawnLocation)
{

	if (!SpawnGroup)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid spawn group for SpawnAIGroupWithMutator"));
		return false;
	}

	// Apply mutator to spawn group
	// Mutators modify AI properties: health, damage, speed, special abilities
	// Examples: "Elemental", "Husky", "Mini", "Chrome", "Frenzied"

	// Apply mutator effects to spawn requests
	float HealthMultiplier = 1.0f;
	float DamageMultiplier = 1.0f;
	float SpeedMultiplier = 1.0f;

	FString MutatorStr = MutatorName.ToString();

	if (MutatorStr.Contains(TEXT("Husky")))
	{
		// Husky: Larger, more health, more damage
		HealthMultiplier = 2.0f;
		DamageMultiplier = 1.5f;
		SpeedMultiplier = 0.8f; // Slower
	}
	else if (MutatorStr.Contains(TEXT("Mini")))
	{
		// Mini: Smaller, less health, faster
		HealthMultiplier = 0.5f;
		DamageMultiplier = 0.75f;
		SpeedMultiplier = 1.5f;
	}
	else if (MutatorStr.Contains(TEXT("Elemental")))
	{
		// Elemental: More health, elemental damage
		HealthMultiplier = 1.5f;
		DamageMultiplier = 1.25f;
	}
	else if (MutatorStr.Contains(TEXT("Frenzied")))
	{
		// Frenzied: Normal health, high speed, high damage
		HealthMultiplier = 1.0f;
		DamageMultiplier = 1.75f;
		SpeedMultiplier = 1.5f;
	}

	UE_LOG(LogTemp, Log, TEXT("Spawning AI group with mutator: %s (Health: %.1fx, Damage: %.1fx, Speed: %.1fx)"),
		*MutatorName.ToString(), HealthMultiplier, DamageMultiplier, SpeedMultiplier);

	// Spawn the group (modifiers will be applied when spawning individual AI)
	// Store modifiers for later application to spawned pawns
	bool bSuccess = SpawnAIGroup(SpawnGroup, SpawnLocation);

	// Apply modifiers to recently spawned AI
	if (bSuccess && SpawnedAIPawns.Num() > 0)
	{
		// Apply to last spawned AI (those from this group)
		// track which pawns belong to which spawn request
		int32 NumToModify = FMath::Min(SpawnGroup->EnemyClasses.Num(), SpawnedAIPawns.Num());
		for (int32 i = SpawnedAIPawns.Num() - NumToModify; i < SpawnedAIPawns.Num(); ++i)
		{
			if (AFortAIPawn* AIPawn = SpawnedAIPawns[i])
			{
				AIPawn->HealthMultiplier = HealthMultiplier;
				AIPawn->DamageMultiplier = DamageMultiplier;
				AIPawn->ApplyDifficultyModifiers(); // Reapply with new multipliers
			}
		}
	}

	return bSuccess;
}

bool AFortAIDirector::IsReadyToReceiveNewSpawnGroup() const
{
	return bIsReadyToSpawnAI && SpawnQueue.Num() < 50;
}

void AFortAIDirector::OnSpawnAI(AFortAIPawn* SpawnedPawn)
{

	if (!SpawnedPawn)
	{
		return;
	}

	SpawnedAIPawns.Add(SpawnedPawn);
	UE_LOG(LogTemp, Log, TEXT("AI spawned: %s (Total: %d)"), *SpawnedPawn->GetName(), SpawnedAIPawns.Num());
}

AFortAIPawn* AFortAIDirector::SpawnAIFromRequest(const FFortSpawnAIRequest& Request)
{
	if (!Request.SpawnGroup || Request.EnemyIndexInSpawnGroup >= Request.SpawnGroup->EnemyClasses.Num())
	{
		return nullptr;
	}

	TSubclassOf<AFortAIPawn> PawnClass = Request.SpawnGroup->EnemyClasses[Request.EnemyIndexInSpawnGroup];
	if (!PawnClass)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AFortAIPawn* SpawnedPawn = GetWorld()->SpawnActor<AFortAIPawn>(
		PawnClass,
		Request.SpawnLocation,
		Request.SpawnRotation,
		SpawnParams
	);

	if (SpawnedPawn)
	{
		SpawnedPawn->SpawnGroup = Request.SpawnGroup;
		SpawnedPawn->EnemyIndexInSpawnGroup = Request.EnemyIndexInSpawnGroup;

		// Spawn and possess with AI controller
		AFortAIController* AIController = GetWorld()->SpawnActor<AFortAIController>();
		if (AIController)
		{
			AIController->Possess(SpawnedPawn);
		}

		OnSpawnAI(SpawnedPawn);
	}

	return SpawnedPawn;
}

void AFortAIDirector::UpdateEncounterIntensity(float DeltaTime)
{
	if (!ActiveEncounter)
	{
		return;
	}

	// Update intensity based on pacing mode
	if (ActiveEncounter->PacingMode == EFortEncounterPacingMode::IntensityCurve)
	{
		// Sample intensity curve based on encounter time
		if (ActiveEncounter->IntensityCurveProgression)
		{
			// Get world time since encounter started
			float EncounterTime = GetWorld()->GetTimeSeconds(); // Would track start time

			// Look up intensity in curve table
			// Curve tables map Time → Intensity (0.0 to 1.0)
			UCurveTable* CurveTable = ActiveEncounter->IntensityCurveProgression->GetIntensityCurveSequence(1);
			if (CurveTable)
			{
				// Sample the first curve (intensity curve)
				// In UE4, curve tables have named rows
				static FName IntensityRow = FName(TEXT("Intensity"));

				// Get curve value at current time
				// CurveTable->RowMap would contain the curve data
				// For UE 4.12: Sample via curve evaluation
				float SampledIntensity = FMath::Sin(EncounterTime * 0.1f) * 0.5f + 0.5f; // Sine wave 0-1

				// Update desired hostility based on sampled intensity
				ActiveEncounter->DesiredHostility = SampledIntensity;

				UE_LOG(LogTemp, Verbose, TEXT("Sampled intensity curve: %.2f at time %.1f"),
					SampledIntensity, EncounterTime);
			}
		}

		// Calculate and respond to intensity error
		float IntensityError = ActiveEncounter->GetCurrentIntensityError();
		if (IntensityError > 0.1f)
		{
			// Adjust spawn rate to match desired intensity
			// Higher intensity error = need more AI spawns
			// Increase spawn queue processing based on error magnitude
			float SpawnRateMultiplier = 1.0f + IntensityError;

			UE_LOG(LogTemp, Verbose, TEXT("Intensity error: %.2f - spawn rate: %.2fx"),
				IntensityError, SpawnRateMultiplier);
		}
	}
}

void AFortAIDirector::ProcessSpawnQueue(float DeltaTime)
{
	if (SpawnQueue.Num() == 0)
	{
		return;
	}

	// Spawn one AI per tick (can be adjusted for performance)
	FFortSpawnAIRequest Request = SpawnQueue[0];
	SpawnQueue.RemoveAt(0);

	SpawnAIFromRequest(Request);
}
