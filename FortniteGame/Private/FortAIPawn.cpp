// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite AI Pawn and Controller - AI Character Implementation
// Source: D:\BuildFarm\buildmachine_++Fortnite+Release-Live\FortniteGame\Source\FortniteGame\Private\AI\FortAIPawn.cpp

#include "FortAIPawn.h"
#include "FortAIDirector.h"
#include "BuildingActor.h"
#include "Net/UnrealNetwork.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "GameFramework/CharacterMovementComponent.h"

// ============================================================================
// AFortAIPawn
// ============================================================================

AFortAIPawn::AFortAIPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bReplicates = true;
	bIsSleeping = false;
	SleepingFloorBuildingActor = nullptr;
	EnemyIndexInSpawnGroup = -1;
	DifficultyModifier = 1.0f;
	HealthMultiplier = 1.0f;
	DamageMultiplier = 1.0f;

	// Configure character movement
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->MaxWalkSpeed = 300.0f;
		MovementComp->bUseRVOAvoidance = true;
	}
}

void AFortAIPawn::BeginPlay()
{
	Super::BeginPlay();

	// Apply difficulty modifiers
	ApplyDifficultyModifiers();
}

void AFortAIPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Additional AI logic here
}

float AFortAIPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// Check for death
	if (GetCharacterMovement() && GetCharacterMovement()->MovementMode != MOVE_None)
	{
		// Still alive
		if (bIsSleeping)
		{
			// Wake up when damaged
			WakeUpAI();
		}
	}
	else
	{
		// Handle death
		OnDeath(ActualDamage, DamageEvent, EventInstigator ? EventInstigator->GetPawn() : nullptr, DamageCauser);
	}

	return ActualDamage;
}

void AFortAIPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AFortAIController* AIController = Cast<AFortAIController>(NewController);
	if (AIController)
	{
		// AI controller successfully possessed this pawn
		UE_LOG(LogTemp, Log, TEXT("AI Pawn %s possessed by controller %s"), *GetName(), *AIController->GetName());
	}
}

void AFortAIPawn::OnSleepingAIsFloorBuildingActorDied(AActor* DamagedActor, float Damage, AController* InstigatedBy, AActor* DamageCauser)
{

	if (bIsSleeping && SleepingFloorBuildingActor == DamagedActor)
	{
		// Floor was destroyed, wake up and fall
		WakeUpAI();
		SleepingFloorBuildingActor = nullptr;

		UE_LOG(LogTemp, Log, TEXT("Sleeping AI %s woken up because floor building actor died"), *GetName());
	}
}

void AFortAIPawn::SetAISleeping(bool bShouldSleep)
{
	if (bIsSleeping == bShouldSleep)
	{
		return;
	}

	bIsSleeping = bShouldSleep;

	if (bIsSleeping)
	{
		// Disable tick and movement
		SetActorTickEnabled(false);
		if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
		{
			MovementComp->DisableMovement();
		}

		// Find floor building actor beneath us
		FHitResult HitResult;
		FVector Start = GetActorLocation();
		FVector End = Start - FVector(0, 0, 1000.0f);

		if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility))
		{
			SleepingFloorBuildingActor = Cast<ABuildingActor>(HitResult.Actor.Get());
			if (SleepingFloorBuildingActor)
			{
				// Bind to floor building actor's damage event
				SleepingFloorBuildingActor->OnTakeAnyDamage.AddDynamic(this, &AFortAIPawn::OnSleepingAIsFloorBuildingActorDied);
			}
		}
	}
	else
	{
		WakeUpAI();
	}
}

void AFortAIPawn::WakeUpAI()
{
	if (!bIsSleeping)
	{
		return;
	}

	bIsSleeping = false;

	// Re-enable tick and movement
	SetActorTickEnabled(true);
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->SetMovementMode(MOVE_Walking);
	}

	// Unbind from floor building actor
	if (SleepingFloorBuildingActor)
	{
		SleepingFloorBuildingActor->OnTakeAnyDamage.RemoveDynamic(this, &AFortAIPawn::OnSleepingAIsFloorBuildingActorDied);
		SleepingFloorBuildingActor = nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("AI %s woken up"), *GetName());
}

void AFortAIPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFortAIPawn, bIsSleeping);
}

void AFortAIPawn::OnDeath(float Damage, const FDamageEvent& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser)
{
	// Disable collision
	SetActorEnableCollision(false);

	// Disable movement
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->DisableMovement();
	}

	// Play death effects: death montage, particle effects, death sounds
	if (USkeletalMeshComponent* Mesh = GetMesh())
	{
		if (Mesh->AnimScriptInstance)
		{
			// Play death montage on the animation instance
			// In UE 4.12, death montages would typically be in a character's animation blueprint
			// For now, we trigger death state which animation blueprint can respond to
			UE_LOG(LogTemp, Log, TEXT("Triggering death animation for AI Pawn %s"), *GetName());

			// Stop all montages and transition to death state
			Mesh->AnimScriptInstance->Montage_Stop(0.2f);
		}
	}

	// Spawn death particle effects at pawn location
	if (UWorld* World = GetWorld())
	{
		// Spawn particle effect at death location
		// this would reference a specific particle system asset
		FVector DeathLocation = GetActorLocation();
		UE_LOG(LogTemp, Log, TEXT("Spawning death effects at location: %s"), *DeathLocation.ToString());

		// Example: UGameplayStatics::SpawnEmitterAtLocation(World, DeathParticleSystem, DeathLocation);
	}

	// Play death sound
	// this would play a specific death sound cue
	UE_LOG(LogTemp, Log, TEXT("AI Pawn %s died from %.1f damage - death effects triggered"), *GetName(), Damage);

	// Notify director
	// Find AI Director and notify of death for encounter tracking
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AFortAIDirector> It(World); It; ++It)
		{
			AFortAIDirector* Director = *It;
			if (Director)
			{
				// Remove from spawned list
				Director->SpawnedAIPawns.Remove(this);
				UE_LOG(LogTemp, Log, TEXT("Notified AI Director of death"));
				break;
			}
		}
	}

	// Destroy after delay
	SetLifeSpan(5.0f);
}

void AFortAIPawn::ApplyDifficultyModifiers()
{
	// Apply health multiplier
	if (HealthMultiplier != 1.0f)
	{
		// Modify max health based on difficulty
		float BaseHealth = 100.0f; // Default health
		float ModifiedHealth = BaseHealth * HealthMultiplier * DifficultyModifier;

		// Set health through UE 4.12 native ability system component
		UFortAbilitySystemComponent* ASC = FindComponentByClass<UFortAbilitySystemComponent>();
		if (ASC)
		{
			// Use native attribute system (UE 4.12 compatible)
			ASC->SetAttributeValue(FName("MaxHealth"), ModifiedHealth);
			ASC->SetAttributeValue(FName("Health"), ModifiedHealth);
			UE_LOG(LogTemp, Verbose, TEXT("Modified AI max health via attribute system: %.1f"), ModifiedHealth);
		}
		else
		{
			// Fallback: If no ability system component, log warning
			UE_LOG(LogTemp, Warning, TEXT("AI Pawn %s has no FortAbilitySystemComponent - cannot apply health modifier"), *GetName());
		}

		UE_LOG(LogTemp, Verbose, TEXT("Modified AI health: %.1f"), ModifiedHealth);
	}

	// Apply damage multiplier
	// Damage multiplier is applied when dealing damage, not here

	UE_LOG(LogTemp, Log, TEXT("Applied difficulty modifiers to %s: Health=%.1fx, Damage=%.1fx"),
		*GetName(), HealthMultiplier, DamageMultiplier);
}

// ============================================================================
// AFortAIController
// ============================================================================

AFortAIController::AFortAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CurrentGoalActor = nullptr;
	BehaviorTreeAsset = nullptr;
	BlackboardAsset = nullptr;
}

void AFortAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AFortAIPawn* AIPawn = Cast<AFortAIPawn>(InPawn);
	if (AIPawn)
	{
		// Run behavior tree
		RunBehaviorTree();

		UE_LOG(LogTemp, Log, TEXT("AI Controller %s possessed pawn %s"), *GetName(), *AIPawn->GetName());
	}
}

void AFortAIController::OnUnPossess()
{
	// Stop behavior tree
	if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent))
	{
		BTComp->StopTree();
	}

	Super::OnUnPossess();
}

void AFortAIController::OnReceiveGoalQueryResult(AActor* GoalActor, bool bSuccess)
{

	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Goal query failed for AI Controller %s"), *GetName());
		return;
	}

	SetGoalActor(GoalActor);
}

void AFortAIController::SetGoalActor(AActor* NewGoalActor)
{
	CurrentGoalActor = NewGoalActor;

	// Update blackboard
	if (Blackboard)
	{
		Blackboard->SetValueAsObject(FName("GoalActor"), CurrentGoalActor);
	}

	UE_LOG(LogTemp, Log, TEXT("AI Controller %s set goal to %s"),
		*GetName(), CurrentGoalActor ? *CurrentGoalActor->GetName() : TEXT("None"));
}

void AFortAIController::RunBehaviorTree()
{
	if (!BehaviorTreeAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("AI Controller %s has no behavior tree asset"), *GetName());
		return;
	}

	// Initialize blackboard
	if (BlackboardAsset && UseBlackboard(BlackboardAsset, Blackboard))
	{
		// Run behavior tree
		RunBehaviorTree(BehaviorTreeAsset);
		UE_LOG(LogTemp, Log, TEXT("AI Controller %s started behavior tree"), *GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AI Controller %s failed to initialize blackboard"), *GetName());
	}
}

void AFortAIController::UpdateGoal()
{
	// Query goal manager for new goal
	// Query AFortAIGoalManager for highest priority target
	// Goals can be: player fortifications, objectives, player characters
	// Priority based on: distance, threat level, strategic value

	if (UWorld* World = GetWorld())
	{
		// Find goal manager
		for (TActorIterator<AFortAIGoalManager> It(World); It; ++It)
		{
			AFortAIGoalManager* GoalManager = *It;
			if (GoalManager && GoalManager->ActiveGoals.Num() > 0)
			{
				// Get controlled pawn for distance calculations
				APawn* ControlledPawn = GetPawn();
				if (!ControlledPawn)
				{
					UE_LOG(LogTemp, Warning, TEXT("AI Controller %s has no pawn for goal calculation"), *GetName());
					return;
				}

				FVector PawnLocation = ControlledPawn->GetActorLocation();

				// Find highest priority goal based on multiple factors
				AActor* BestGoal = nullptr;
				float BestPriority = -1.0f;

				for (AActor* Goal : GoalManager->ActiveGoals)
				{
					if (!Goal)
					{
						continue;
					}

					float Priority = 0.0f;

					// Distance factor (closer = higher priority, inverted and normalized)
					float Distance = FVector::Dist(PawnLocation, Goal->GetActorLocation());
					float DistancePriority = FMath::Clamp(1.0f - (Distance / 5000.0f), 0.0f, 1.0f); // 50m max range
					Priority += DistancePriority * 10.0f; // Weight: 10

					// Threat level factor (check if it's a player or building)
					if (Goal->IsA(ACharacter::StaticClass()))
					{
						// Player characters are high priority threats
						Priority += 20.0f; // Weight: 20
					}
					else if (Goal->IsA(ABuildingActor::StaticClass()))
					{
						ABuildingActor* Building = Cast<ABuildingActor>(Goal);
						if (Building)
						{
							// Strategic buildings (objectives) are medium priority
							if (Building->BuildingType == EFortBuildingType::Objective)
							{
								Priority += 30.0f; // Weight: 30 (highest - objectives)
							}
							else if (Building->BuildingType == EFortBuildingType::Wall ||
							         Building->BuildingType == EFortBuildingType::Floor)
							{
								Priority += 5.0f; // Weight: 5 (lower - structural)
							}
						}
					}

					// Track best goal
					if (Priority > BestPriority)
					{
						BestPriority = Priority;
						BestGoal = Goal;
					}
				}

				// Set the best goal
				if (BestGoal)
				{
					SetGoalActor(BestGoal);
					UE_LOG(LogTemp, Log, TEXT("AI Controller %s selected goal: %s (priority: %.1f)"),
						*GetName(), *BestGoal->GetName(), BestPriority);
					return;
				}
			}
		}
	}

	UE_LOG(LogTemp, Verbose, TEXT("No goals available for AI Controller %s"), *GetName());
}
