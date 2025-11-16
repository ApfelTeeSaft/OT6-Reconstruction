// Copyright Epic Games, Inc. All Rights Reserved.
// FortAbilityTask_DirectedMovement Implementation

#include "FortAbilityTask_DirectedMovement.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SceneComponent.h"
#include "TimerManager.h"

//////////////////////////////////////////////////////////////////////////
// UFortAbilityTask_DirectedMovement

UFortAbilityTask_DirectedMovement::UFortAbilityTask_DirectedMovement()
	: OwningAbility(nullptr)
	, TargetActor(nullptr)
	, TargetComponent(nullptr)
	, AcceptanceRadius(50.0f)
	, bStopOnOverlap(true)
	, bUseTargetActor(false)
	, bUseTargetComponent(false)
	, bUseTargetLocation(false)
{
}

UFortAbilityTask_DirectedMovement* UFortAbilityTask_DirectedMovement::DirectedMovementToActor(
	UFortGameplayAbility* OwningAbility,
	FName TaskInstanceName,
	AActor* TargetActor,
	float AcceptanceRadius,
	bool bStopOnOverlap)
{
	if (!TargetActor)
	{
		UE_LOG(LogTemp, Error, TEXT("UFortAbilityTask_DirectedMovement::DirectedMovementToActor called from %s with invalid TargetActor"),
			OwningAbility ? *OwningAbility->GetName() : TEXT("unknown"));
		return nullptr;
	}

	UFortAbilityTask_DirectedMovement* Task = NewObject<UFortAbilityTask_DirectedMovement>();
	if (Task)
	{
		Task->OwningAbility = OwningAbility;
		Task->TaskInstanceName = TaskInstanceName;
		Task->TargetActor = TargetActor;
		Task->AcceptanceRadius = AcceptanceRadius;
		Task->bStopOnOverlap = bStopOnOverlap;
		Task->bUseTargetActor = true;
	}

	return Task;
}

UFortAbilityTask_DirectedMovement* UFortAbilityTask_DirectedMovement::DirectedMovementToComponent(
	UFortGameplayAbility* OwningAbility,
	FName TaskInstanceName,
	USceneComponent* TargetComponent,
	float AcceptanceRadius,
	bool bStopOnOverlap)
{
	if (!TargetComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("UFortAbilityTask_DirectedMovement::DirectedMovementToActor called from %s with invalid TargetComponent"),
			OwningAbility ? *OwningAbility->GetName() : TEXT("unknown"));
		return nullptr;
	}

	UFortAbilityTask_DirectedMovement* Task = NewObject<UFortAbilityTask_DirectedMovement>();
	if (Task)
	{
		Task->OwningAbility = OwningAbility;
		Task->TaskInstanceName = TaskInstanceName;
		Task->TargetComponent = TargetComponent;
		Task->AcceptanceRadius = AcceptanceRadius;
		Task->bStopOnOverlap = bStopOnOverlap;
		Task->bUseTargetComponent = true;
	}

	return Task;
}

UFortAbilityTask_DirectedMovement* UFortAbilityTask_DirectedMovement::DirectedMovementToLocation(
	UFortGameplayAbility* OwningAbility,
	FName TaskInstanceName,
	FVector TargetLocation,
	float AcceptanceRadius)
{
	UFortAbilityTask_DirectedMovement* Task = NewObject<UFortAbilityTask_DirectedMovement>();
	if (Task)
	{
		Task->OwningAbility = OwningAbility;
		Task->TaskInstanceName = TaskInstanceName;
		Task->TargetLocation = TargetLocation;
		Task->AcceptanceRadius = AcceptanceRadius;
		Task->bUseTargetLocation = true;
	}

	return Task;
}

void UFortAbilityTask_DirectedMovement::Activate()
{
	if (!OwningAbility)
	{
		UE_LOG(LogTemp, Warning, TEXT("UFortAbilityTask_DirectedMovement: Cannot activate - missing ability"));
		OnFailed.Broadcast();
		return;
	}

	AActor* AvatarActor = OwningAbility->GetAvatarActor();
	if (!AvatarActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("UFortAbilityTask_DirectedMovement: Cannot activate - no avatar actor"));
		OnFailed.Broadcast();
		return;
	}

	// Get AI controller
	ACharacter* Character = Cast<ACharacter>(AvatarActor);
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("UFortAbilityTask_DirectedMovement: Cannot activate - avatar is not a character"));
		OnFailed.Broadcast();
		return;
	}

	AAIController* AIController = Cast<AAIController>(Character->GetController());
	if (!AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("UFortAbilityTask_DirectedMovement: Cannot activate - no AI controller"));
		OnFailed.Broadcast();
		return;
	}

	// Determine target location
	FVector FinalTargetLocation;
	if (bUseTargetActor && TargetActor)
	{
		FinalTargetLocation = TargetActor->GetActorLocation();
	}
	else if (bUseTargetComponent && TargetComponent)
	{
		FinalTargetLocation = TargetComponent->GetComponentLocation();
	}
	else if (bUseTargetLocation)
	{
		FinalTargetLocation = TargetLocation;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UFortAbilityTask_DirectedMovement: Cannot activate - no valid target"));
		OnFailed.Broadcast();
		return;
	}

	// Move to location
	if (bUseTargetActor && TargetActor)
	{
		AIController->MoveToActor(TargetActor, AcceptanceRadius, bStopOnOverlap);
	}
	else
	{
		AIController->MoveToLocation(FinalTargetLocation, AcceptanceRadius, bStopOnOverlap);
	}

	UE_LOG(LogTemp, Log, TEXT("UFortAbilityTask_DirectedMovement: Started movement to %s"), *FinalTargetLocation.ToString());

	// Start tick timer to check for completion
	UWorld* World = OwningAbility->GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			MovementTickHandle,
			this,
			&UFortAbilityTask_DirectedMovement::TickMovement,
			0.1f,
			true
		);
	}
}

void UFortAbilityTask_DirectedMovement::TickMovement(float DeltaTime)
{
	if (!OwningAbility)
	{
		OnFailed.Broadcast();
		return;
	}

	AActor* AvatarActor = OwningAbility->GetAvatarActor();
	if (!AvatarActor)
	{
		OnFailed.Broadcast();
		return;
	}

	// Get current target location
	FVector CurrentTargetLocation;
	if (bUseTargetActor && TargetActor)
	{
		CurrentTargetLocation = TargetActor->GetActorLocation();
	}
	else if (bUseTargetComponent && TargetComponent)
	{
		CurrentTargetLocation = TargetComponent->GetComponentLocation();
	}
	else if (bUseTargetLocation)
	{
		CurrentTargetLocation = TargetLocation;
	}
	else
	{
		OnFailed.Broadcast();
		return;
	}

	// Check if we've reached the target
	float Distance = FVector::Dist(AvatarActor->GetActorLocation(), CurrentTargetLocation);
	if (Distance <= AcceptanceRadius)
	{
		OnTargetReached();
	}
}

void UFortAbilityTask_DirectedMovement::OnTargetReached()
{
	UE_LOG(LogTemp, Log, TEXT("UFortAbilityTask_DirectedMovement: Target reached"));

	// Clear timer
	if (OwningAbility)
	{
		UWorld* World = OwningAbility->GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(MovementTickHandle);
		}
	}

	OnCompleted.Broadcast();
}
