// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite PlayerController Implementation

#include "FortPlayerController.h"
#include "FortPawn.h"
#include "Net/UnrealNetwork.h"

AFortPlayerController::AFortPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentAbilityIndex(-1)
	, bInCombat(false)
{
	bReplicates = true;
}

void AFortPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogNet, Log, TEXT("FortPlayerController: BeginPlay for %s"), *GetName());
}

void AFortPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFortPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

void AFortPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	UE_LOG(LogNet, Log, TEXT("FortPlayerController: Possessed %s"), InPawn ? *InPawn->GetName() : TEXT("NULL"));
}

void AFortPlayerController::OnUnPossess()
{
	Super::OnUnPossess();

	UE_LOG(LogNet, Log, TEXT("FortPlayerController: Unpossessed"));
}

void AFortPlayerController::GetPlayerStats(float& Health, float& Shield, float& MaxHealth, float& MaxShield)
{
	AFortPawn* FortPawn = Cast<AFortPawn>(GetPawn());
	if (FortPawn)
	{
		FortPawn->GetPawnStats(Health, Shield, MaxHealth, MaxShield);
	}
	else
	{
		Health = 0.0f;
		Shield = 0.0f;
		MaxHealth = 0.0f;
		MaxShield = 0.0f;
	}
}

void AFortPlayerController::ServerActivateAbility_Implementation(int32 AbilityIndex)
{
	// Server-side ability activation
	CurrentAbilityIndex = AbilityIndex;

	UE_LOG(LogNet, Log, TEXT("FortPlayerController: ServerActivateAbility %d"), AbilityIndex);

	// Notify all clients
	ClientNotifyAbilityActivated(AbilityIndex);
}

bool AFortPlayerController::ServerActivateAbility_Validate(int32 AbilityIndex)
{
	// Validate ability index
	return AbilityIndex >= 0 && AbilityIndex < 10; // Max 10 abilities
}

void AFortPlayerController::ClientNotifyAbilityActivated_Implementation(int32 AbilityIndex)
{
	// Client-side notification
	UE_LOG(LogNet, Log, TEXT("FortPlayerController: ClientNotifyAbilityActivated %d"), AbilityIndex);
}

void AFortPlayerController::HandlePlayerDeath()
{
	UE_LOG(LogNet, Log, TEXT("FortPlayerController: HandlePlayerDeath"));

	// Disable input
	DisableInput(this);
}

void AFortPlayerController::RequestRespawn()
{
	// Request respawn from server
	UE_LOG(LogNet, Log, TEXT("FortPlayerController: RequestRespawn"));
}

void AFortPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFortPlayerController, CurrentAbilityIndex);
	DOREPLIFETIME(AFortPlayerController, PawnUniqueID);
	DOREPLIFETIME(AFortPlayerController, bInCombat);
}
