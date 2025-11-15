// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite AI Pawn and Controller - AI Character Implementation
// Source: D:\BuildFarm\buildmachine_++Fortnite+Release-Live\FortniteGame\Source\FortniteGame\Private\AI\FortAIPawn.cpp

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "FortEnums.h"
#include "FortAIPawn.generated.h"

// Forward declarations
class AFortAIController;
class UFortAISpawnGroup;
class ABuildingActor;

/**
 * AFortAIPawn - Base AI pawn for Fortnite enemies
 */
UCLASS()
class FORTNITEGAME_API AFortAIPawn : public ACharacter
{
	GENERATED_BODY()

public:
	AFortAIPawn(const FObjectInitializer& ObjectInitializer);

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	//~ End AActor Interface

	//~ Begin APawn Interface
	virtual void PossessedBy(AController* NewController) override;
	//~ End APawn Interface

	/**
	 * Handle sleeping AI's floor building actor death
	 */
	UFUNCTION()
	void OnSleepingAIsFloorBuildingActorDied(AActor* DamagedActor, float Damage, AController* InstigatedBy, AActor* DamageCauser);

	/**
	 * Set AI to sleep (inactive) state
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	void SetAISleeping(bool bShouldSleep);

	/**
	 * Wake up sleeping AI
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	void WakeUpAI();

	/** AI spawn group this pawn belongs to */
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	UFortAISpawnGroup* SpawnGroup;

	/** Enemy index within spawn group */
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	int32 EnemyIndexInSpawnGroup;

	/** Is AI currently sleeping (inactive)? */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "AI")
	bool bIsSleeping;

	/** Floor building actor for sleeping AI */
	UPROPERTY()
	ABuildingActor* SleepingFloorBuildingActor;

	/** AI difficulty modifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float DifficultyModifier;

	/** AI health multiplier based on difficulty */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float HealthMultiplier;

	/** AI damage multiplier based on difficulty */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float DamageMultiplier;

	//~ Begin Replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End Replication

protected:
	/** Handle death */
	virtual void OnDeath(float Damage, const FDamageEvent& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser);

	/** Apply difficulty modifiers */
	void ApplyDifficultyModifiers();
};

/**
 * AFortAIController - AI controller for Fortnite enemies
 */
UCLASS()
class FORTNITEGAME_API AFortAIController : public AAIController
{
	GENERATED_BODY()

public:
	AFortAIController(const FObjectInitializer& ObjectInitializer);

	//~ Begin AController Interface
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	//~ End AController Interface

	/**
	 * Receive goal query result
	 */
	UFUNCTION()
	void OnReceiveGoalQueryResult(AActor* GoalActor, bool bSuccess);

	/**
	 * Set current goal actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	void SetGoalActor(AActor* NewGoalActor);

	/**
	 * Get current goal actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|AI")
	AActor* GetGoalActor() const { return CurrentGoalActor; }

	/** Current goal actor (e.g., building to attack, player to chase) */
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	AActor* CurrentGoalActor;

	/** Behavior tree asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UBehaviorTree* BehaviorTreeAsset;

	/** Blackboard asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UBlackboardData* BlackboardAsset;

protected:
	/** Run behavior tree */
	void RunBehaviorTree();

	/** Update goal */
	void UpdateGoal();
};
