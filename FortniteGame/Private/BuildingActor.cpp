// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite Building Actor Implementation

#include "BuildingActor.h"
#include "FortGameState.h"
#include "FortPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"

//////////////////////////////////////////////////////////////////////////
// ABuildingActor

ABuildingActor::ABuildingActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, BuildingType(EFortBuildingType::None)
	, ResourceType(EBuildingResourceType::Wood)
	, MaxHealth(100.0f)
	, CurrentHealth(100.0f)
	, Team(EFortTeam::HumanCampaign)
	, BuildingState(EFortBuildingPersistentState::Default)
	, InitializationReason(EFortBuildingInitializationReason::None)
	, EditMode(EBuildingEditMode::None)
	, CurrentEditor(nullptr)
	, GridPosition(FVector::ZeroVector)
	, GridRotation(FRotator::ZeroRotator)
{


	bReplicates = true;
	bAlwaysRelevant = true;
	bNetUseOwnerRelevancy = false;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ABuildingActor::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		// Initialize health
		CurrentHealth = MaxHealth;

		OnBuildingActorInitialized();
	}
}

void ABuildingActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float ABuildingActor::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (Role != ROLE_Authority)
	{
		return 0.0f;
	}

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	CurrentHealth -= ActualDamage;

	// "Building Actor Health: %.2f.  %.2f%% of %.2f.\n"
	float HealthPercent = GetBuildingHealthPercentage();
	UE_LOG(LogNet, Verbose, TEXT("Building Actor Health: %.2f.  %.2f%% of %.2f."), CurrentHealth, HealthPercent * 100.0f, MaxHealth);

	if (CurrentHealth <= 0.0f)
	{
		CurrentHealth = 0.0f;
		// Building destroyed
		Destroy();
	}

	return ActualDamage;
}

void ABuildingActor::OnDayPhaseChanged(EFortDayPhase DayPhase)
{
	// "&ABuildingActor::OnDayPhaseChanged"
	UE_LOG(LogNet, Verbose, TEXT("ABuildingActor::OnDayPhaseChanged - Phase: %d"), static_cast<int32>(DayPhase));

	// Update building visuals or behavior based on day phase here
}

float ABuildingActor::GetInteractionTime() const
{
	// Default interaction time
	return 0.5f;
}

float ABuildingActor::GetBuildingHealthPercentage() const
{
	return MaxHealth > 0.0f ? (CurrentHealth / MaxHealth) : 0.0f;
}

void ABuildingActor::InitializeKismetSpawnedBuildingActor(EFortBuildingInitializationReason Reason)
{
	if (Role != ROLE_Authority)
	{
		return;
	}

	InitializationReason = Reason;

	UE_LOG(LogNet, Log, TEXT("ABuildingActor: InitializeKismetSpawnedBuildingActor - Reason: %d"), static_cast<int32>(Reason));

	OnBuildingActorInitialized();
}

void ABuildingActor::OnRep_MaxHealth()
{
	UE_LOG(LogNet, Verbose, TEXT("ABuildingActor: OnRep_MaxHealth - New MaxHealth: %.2f"), MaxHealth);

	if (CurrentHealth > MaxHealth)
	{
		CurrentHealth = MaxHealth;
	}
}

void ABuildingActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABuildingActor, BuildingType);
	DOREPLIFETIME(ABuildingActor, ResourceType);
	DOREPLIFETIME(ABuildingActor, MaxHealth);
	DOREPLIFETIME(ABuildingActor, CurrentHealth);
	DOREPLIFETIME(ABuildingActor, Team);
	DOREPLIFETIME(ABuildingActor, BuildingState);
	DOREPLIFETIME(ABuildingActor, InitializationReason);
	DOREPLIFETIME(ABuildingActor, EditMode);
	DOREPLIFETIME(ABuildingActor, CurrentEditor);
	DOREPLIFETIME(ABuildingActor, GridPosition);
	DOREPLIFETIME(ABuildingActor, GridRotation);
}

//////////////////////////////////////////////////////////////////////////
// ABuildingSMActor

ABuildingSMActor::ABuildingSMActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, AltMeshIdx(0)
	, LootTier(0)
	, WoodToSpawn(0)
	, StoneToSpawn(0)
	, MetalToSpawn(0)
	, HealthModifierPerLevel(1.0f)
	, bPlayingDestroyEffect(false)
	, bPlayingLODEffect(false)
{
	UE_LOG(LogNet, Log, TEXT("ABuildingSMActor::ABuildingSMActor() Building: %s, AltMeshIdx: %d"), *GetName(), AltMeshIdx);

	// Create static mesh component
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = StaticMeshComponent;

	if (StaticMeshComponent)
	{
		StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		StaticMeshComponent->SetCollisionObjectType(ECC_WorldStatic);
		StaticMeshComponent->SetGenerateOverlapEvents(false);
	}
	else
	{
		UE_LOG(LogNet, Error, TEXT("Building actor %s has no StaticMeshComponent!"), *GetName());
	}
}

void ABuildingSMActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Role == ROLE_Authority)
	{
		// Initialize health based on material and level
		MaxHealth = DetermineHealthMax();
		CurrentHealth = MaxHealth;
	}
}

void ABuildingSMActor::BeginPlay()
{
	Super::BeginPlay();

	// Initialize resource spawn amounts based on material type
	switch (ResourceType)
	{
	case EBuildingResourceType::Wood:
		WoodToSpawn = DetermineMaxResourcesToSpawn();
		break;
	case EBuildingResourceType::Stone:
		StoneToSpawn = DetermineMaxResourcesToSpawn();
		break;
	case EBuildingResourceType::Metal:
		MetalToSpawn = DetermineMaxResourcesToSpawn();
		break;
	default:
		break;
	}
}

void ABuildingSMActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABuildingSMActor::OnDayPhaseChanged(EFortDayPhase DayPhase)
{
	Super::OnDayPhaseChanged(DayPhase);

	// Update building visuals for day/night
	UpdateLODOverrideEffect();
}

float ABuildingSMActor::DetermineHealthMax()
{

	// Base health values for different material types
	float BaseHealth = 100.0f;

	switch (ResourceType)
	{
	case EBuildingResourceType::Wood:
		BaseHealth = 200.0f;
		break;
	case EBuildingResourceType::Stone:
		BaseHealth = 400.0f;
		break;
	case EBuildingResourceType::Metal:
		BaseHealth = 600.0f;
		break;
	default:
		BaseHealth = 100.0f;
		break;
	}

	// Apply level-based modifier
	if (GetWorld() && GetWorld()->GetGameState())
	{
		AFortGameStateZone* FortGS = Cast<AFortGameStateZone>(GetWorld()->GetGameState());
		if (FortGS)
		{
			const FZoneDifficultyInfo& DifficultyInfo = FortGS->GetZoneDifficultyInfo();
			float LevelModifier = GetHealthModifierBasedOnExpectedLevel(DifficultyInfo.RecommendedPlayerLevel);
			BaseHealth *= LevelModifier;
		}
	}

	return BaseHealth;
}

int32 ABuildingSMActor::DetermineMaxResourcesToSpawn()
{

	// Base resource amounts by building type
	int32 BaseResources = 10;

	switch (BuildingType)
	{
	case EFortBuildingType::Wall:
		BaseResources = 20;
		break;
	case EFortBuildingType::Floor:
		BaseResources = 15;
		break;
	case EFortBuildingType::Stairs:
		BaseResources = 25;
		break;
	case EFortBuildingType::Roof:
		BaseResources = 15;
		break;
	default:
		BaseResources = 10;
		break;
	}

	return BaseResources;
}

float ABuildingSMActor::GetHealthModifierBasedOnExpectedLevel(int32 ExpectedLevel)
{

	// Scale health based on expected player level
	// Formula: 1.0 + (ExpectedLevel * HealthModifierPerLevel)
	float Modifier = 1.0f + (ExpectedLevel * HealthModifierPerLevel);

	// Clamp to reasonable range
	Modifier = FMath::Clamp(Modifier, 0.5f, 10.0f);

	return Modifier;
}

void ABuildingSMActor::UpdateRepairMaterialAnim()
{

	if (!StaticMeshComponent)
	{
		return;
	}

	// Update material parameters for repair animation
	// This would animate materials during repair process
	UE_LOG(LogNet, Verbose, TEXT("ABuildingSMActor: UpdateRepairMaterialAnim"));
}

void ABuildingSMActor::SelectMeshSet(int32 InLootTier)
{

	if (Role != ROLE_Authority)
	{
		return;
	}

	LootTier = InLootTier;

	// Determine alternate mesh index based on loot tier
	// Higher tiers might use different visual variations
	AltMeshIdx = FMath::Clamp(LootTier, 0, 5);

	UE_LOG(LogNet, Log, TEXT("ABuildingSMActor::SelectMeshSet(int32 LootTier) just set AltMeshIdx Building: %s, AltMeshIdx: %d"),
		*GetName(), AltMeshIdx);

	PostUpdate();
}

void ABuildingSMActor::SelectMeshSetByName(FName LootTierKey)
{

	if (Role != ROLE_Authority)
	{
		return;
	}

	// Convert tier key to index
	// Example: "Tier1" -> 1, "Tier2" -> 2, etc.
	FString TierString = LootTierKey.ToString();
	if (TierString.Contains(TEXT("Tier")))
	{
		FString NumStr = TierString.Replace(TEXT("Tier"), TEXT(""));
		LootTier = FCString::Atoi(*NumStr);
		AltMeshIdx = LootTier;
	}

	UE_LOG(LogNet, Log, TEXT("ABuildingSMActor::SelectMeshSet(FName LootTierKey) Building: %s, AltMeshIdx: %d"),
		*GetName(), AltMeshIdx);

	PostUpdate();
}

void ABuildingSMActor::AttemptSpawnResources()
{

	if (Role != ROLE_Authority)
	{
		return;
	}

	UE_LOG(LogNet, Log, TEXT("ABuildingSMActor: AttemptSpawnResources - Wood: %d, Stone: %d, Metal: %d"),
		WoodToSpawn, StoneToSpawn, MetalToSpawn);

	// Spawn resource pickups based on material type and amounts
	// Would create resource item actors at building location

	// This would be called when building is destroyed
}

void ABuildingSMActor::PostUpdate()
{

	UE_LOG(LogNet, Log, TEXT("ABuildingSMActor::PostUpdate() Building: %s, AltMeshIdx: %d"),
		*GetName(), AltMeshIdx);

	// Update mesh and materials based on current state
	// Would swap static mesh based on AltMeshIdx
}

void ABuildingSMActor::UpdateLODOverrideEffect()
{

	if (bPlayingLODEffect)
	{
		return;
	}

	bPlayingLODEffect = true;

	// Start LOD effect timer
	GetWorldTimerManager().SetTimer(
		LODEffectTimerHandle,
		this,
		&ABuildingSMActor::OnLODOverrideEffectFinished,
		1.0f,
		false
	);
}

void ABuildingSMActor::OnLODOverrideEffectFinished()
{

	bPlayingLODEffect = false;
	UE_LOG(LogNet, Verbose, TEXT("ABuildingSMActor: OnLODOverrideEffectFinished"));
}

void ABuildingSMActor::UpdateDynamicShrinkAndDestroyEffect()
{
	// Note: Original has typo "Effectt"

	if (bPlayingDestroyEffect)
	{
		return;
	}

	bPlayingDestroyEffect = true;

	// Start destroy effect timer
	GetWorldTimerManager().SetTimer(
		DestroyEffectTimerHandle,
		this,
		&ABuildingSMActor::OnDynamicShrinkAndDestroyEffectFinished,
		2.0f,
		false
	);

	// Play shrink animation/VFX
	UE_LOG(LogNet, Verbose, TEXT("ABuildingSMActor: UpdateDynamicShrinkAndDestroyEffect"));
}

void ABuildingSMActor::OnDynamicShrinkAndDestroyEffectFinished()
{

	bPlayingDestroyEffect = false;

	if (Role == ROLE_Authority)
	{
		// Spawn resources before destroying
		AttemptSpawnResources();

		// Destroy the building actor
		Destroy();
	}
}

void ABuildingSMActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABuildingSMActor, AltMeshIdx);
	DOREPLIFETIME(ABuildingSMActor, LootTier);
}

//////////////////////////////////////////////////////////////////////////
// AStrategicBuildingActor

AStrategicBuildingActor::AStrategicBuildingActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, StrategicValue(100.0f)
	, bIsObjective(false)
{

	UE_LOG(LogNet, Log, TEXT("AStrategicBuildingActor: Initialized"));

	// Strategic buildings are always relevant
	bAlwaysRelevant = true;
}

void AStrategicBuildingActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStrategicBuildingActor, StrategicValue);
	DOREPLIFETIME(AStrategicBuildingActor, bIsObjective);
	DOREPLIFETIME(AStrategicBuildingActor, TeamStrategicBuildings);
}
