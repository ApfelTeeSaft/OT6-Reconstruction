// Copyright Epic Games, Inc. All Rights Reserved.
// Fortnite Game Mode Implementation

#include "FortGameMode.h"
#include "FortGameState.h"
#include "FortPlayerState.h"
#include "FortPlayerController.h"
#include "FortPawn.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

AFortGameMode::AFortGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bMatchHasStarted(false)
	, MatchStartTime(0.0f)
	, MatchDuration(3600.0f)  // 60 minutes default
{
	// Set default classes
	PlayerControllerClass = AFortPlayerController::StaticClass();
	PlayerStateClass = AFortPlayerState::StaticClass();
	GameStateClass = AFortGameState::StaticClass();
	DefaultPawnClass = AFortPawn::StaticClass();

	bReplicates = true;
	bAlwaysRelevant = true;
}

void AFortGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UE_LOG(LogNet, Log, TEXT("FortGameMode: InitGame - Map: %s, Options: %s"), *MapName, *Options);
}

void AFortGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	UE_LOG(LogNet, Log, TEXT("FortGameMode: PreLogin - Address: %s, UniqueId: %s"), *Address, *UniqueId.ToString());
}

APlayerController* AFortGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	APlayerController* PC = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);

	if (PC)
	{
		UE_LOG(LogNet, Log, TEXT("FortGameMode: Player logged in - Controller: %s"), *PC->GetName());
		ConnectedPlayers.Add(PC);
	}

	return PC;
}

void AFortGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer)
	{
		UE_LOG(LogNet, Log, TEXT("FortGameMode: PostLogin - Player: %s"), *NewPlayer->GetName());

		// Initialize player state
		AFortPlayerState* FortPS = Cast<AFortPlayerState>(NewPlayer->PlayerState);
		if (FortPS)
		{
			UE_LOG(LogNet, Log, TEXT("Loading FortGameMode default pawn class for player: %s ..."), *NewPlayer->GetName());
			UE_LOG(LogNet, Log, TEXT("    ... FortGameMode default pawn class loaded!"));
		}
	}
}

void AFortGameMode::Logout(AController* Exiting)
{
	if (Exiting)
	{
		APlayerController* PC = Cast<APlayerController>(Exiting);
		if (PC)
		{
			ConnectedPlayers.Remove(PC);

			AFortPlayerState* FortPS = Cast<AFortPlayerState>(PC->PlayerState);
			if (FortPS)
			{
				DoLogoutAnalytics(PC, FortPS->AccountId);
			}
		}

		UE_LOG(LogNet, Log, TEXT("FortGameMode: Logout - Controller: %s"), *Exiting->GetName());
	}

	Super::Logout(Exiting);
}

void AFortGameMode::StartPlay()
{
	Super::StartPlay();

	bMatchHasStarted = true;
	MatchStartTime = GetWorld()->GetTimeSeconds();

	UE_LOG(LogNet, Log, TEXT("FortGameMode: Match started at time: %.2f"), MatchStartTime);
}

bool AFortGameMode::ReadyToStartMatch_Implementation()
{
	// Wait for minimum players if needed
	return Super::ReadyToStartMatch_Implementation();
}

void AFortGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (NewPlayer)
	{
		UE_LOG(LogNet, Log, TEXT("FortGameMode: HandleStartingNewPlayer - %s"), *NewPlayer->GetName());
	}
}

void AFortGameMode::ReturnToMainMenuHost()
{
	UE_LOG(LogNet, Log, TEXT("AFortGameMode::ReturnToMainMenuHost()"));

	// Disconnect all players and return to main menu
	for (APlayerController* PC : ConnectedPlayers)
	{
		if (PC)
		{
			PC->ClientReturnToMainMenu(TEXT("Host returned to main menu"));
		}
	}

	// Cleanup match state
	bMatchHasStarted = false;
}

void AFortGameMode::OnSaveWorldRecordCompleteForMainMenu(int32 SaveResult)
{
	UE_LOG(LogNet, Log, TEXT("AFortGameMode::OnSaveWorldRecordCompleteForMainMenu() %d"), SaveResult);

	if (SaveResult == 0)
	{
		// Save successful, proceed to main menu
		ReturnToMainMenuHost();
	}
	else
	{
		// Save failed
		UE_LOG(LogNet, Warning, TEXT("FortGameMode: Failed to save world record before returning to main menu. Result: %d"), SaveResult);
	}
}

void AFortGameMode::DoLogoutAnalytics(APlayerController* Player, const FString& AccountID)
{
	if (AccountID.IsEmpty())
	{
		UE_LOG(LogNet, Warning, TEXT("Empty AFortGameMode::DoLogoutAnalytics called, account ID: %s"), *AccountID);
		return;
	}

	UE_LOG(LogNet, Log, TEXT("FortGameMode: DoLogoutAnalytics - Account ID: %s"), *AccountID);

	// Record player session data
	if (Player && PlayerSessionData.Contains(AccountID))
	{
		float SessionDuration = GetWorld()->GetTimeSeconds() - PlayerSessionData[AccountID];
		UE_LOG(LogNet, Log, TEXT("FortGameMode: Player %s session duration: %.2f seconds"), *AccountID, SessionDuration);

		// Submit analytics here
		PlayerSessionData.Remove(AccountID);
	}
}

void AFortGameMode::HandleScoreToXPUpdate()
{
	UE_LOG(LogNet, Log, TEXT("FortGameMode: HandleScoreToXPUpdate"));

	// Convert player scores to XP
	for (APlayerController* PC : ConnectedPlayers)
	{
		if (PC)
		{
			AFortPlayerState* FortPS = Cast<AFortPlayerState>(PC->PlayerState);
			if (FortPS)
			{
				// Calculate XP based on various stats
				int32 TotalXP = 0;
				TotalXP += FortPS->MonsterKills * 10;
				TotalXP += FortPS->BuildingsBuilt * 5;
				TotalXP += FortPS->Assists * 5;

				UE_LOG(LogNet, Verbose, TEXT("FortGameMode: Player %s earned %d XP"), *FortPS->GetPlayerName(), TotalXP);
			}
		}
	}
}

void AFortGameMode::Say(const FString& Message)
{
	UE_LOG(LogNet, Log, TEXT("FortGameMode: Say - Message: %s"), *Message);

	// Broadcast to all players
	for (APlayerController* PC : ConnectedPlayers)
	{
		if (PC)
		{
			PC->ClientMessage(Message, NAME_None);
		}
	}
}

void AFortGameMode::TeamSay(const FString& Message)
{
	UE_LOG(LogNet, Log, TEXT("FortGameMode: TeamSay - Message: %s"), *Message);

	// Broadcast to team members only
	// Implementation would filter by team
}

void AFortGameMode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFortGameMode, bMatchHasStarted);
	DOREPLIFETIME(AFortGameMode, MatchStartTime);
	DOREPLIFETIME(AFortGameMode, MatchDuration);
}

//////////////////////////////////////////////////////////////////////////
// AFortGameModeEmptyDedicated

AFortGameModeEmptyDedicated::AFortGameModeEmptyDedicated(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	UE_LOG(LogNet, Log, TEXT("AFortGameModeEmptyDedicated: Initializing empty dedicated server"));
}

void AFortGameModeEmptyDedicated::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UE_LOG(LogNet, Log, TEXT("AFortGameModeEmptyDedicated: InitGame - Server ready for connections"));
}

//////////////////////////////////////////////////////////////////////////
// AFortGameModeFrontEnd

AFortGameModeFrontEnd::AFortGameModeFrontEnd(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameStateClass = AFortGameStateFrontEnd::StaticClass();
	PlayerStateClass = AFortPlayerStateFrontEnd::StaticClass();

	UE_LOG(LogNet, Log, TEXT("AFortGameModeFrontEnd: Initializing frontend game mode"));
}

void AFortGameModeFrontEnd::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UE_LOG(LogNet, Log, TEXT("AFortGameModeFrontEnd: InitGame - Frontend ready"));
}

void AFortGameModeFrontEnd::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AFortGameStateFrontEnd* FrontEndGS = Cast<AFortGameStateFrontEnd>(GameState);
	if (FrontEndGS)
	{
		FrontEndGS->NumPlayersInLobby = ConnectedPlayers.Num();
	}
}
