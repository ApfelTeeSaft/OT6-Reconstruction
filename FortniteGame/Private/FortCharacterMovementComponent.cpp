// Copyright Epic Games, Inc. All Rights Reserved.
// Reconstructed from Fortnite UE 4.12 IDA
// Client-side movement prediction and physics implementation

#include "FortCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "AI/Navigation/NavigationSystem.h"
#include "AI/Navigation/RecastNavMesh.h"
#include "FortNavigationTypes.h"
#include "FortNavigationData.h"

// Constants from decompiled code
static const float MOVE_INPUT_SCALE = 0.0078125f;  // 1/128 - from decompilation
static const float MAX_DELTA_TIME = 0.25f;          // Maximum time step to prevent exploits
static const float NETWORK_CORRECTION_THRESHOLD = 4.0f;  // Position error threshold in cm
static const int32 MAX_POSITION_ERROR_SQUARED = 16;     // 4^2 for fast check

//==============================================================================
// FSavedMove_Character_Fort Implementation
//==============================================================================

FSavedMove_Character_Fort::FSavedMove_Character_Fort()
	: Super()
	, TimeStamp(0.0f)
	, DeltaTime(0.0f)
	, SavedAcceleration(FVector::ZeroVector)
	, SavedLocation(FVector::ZeroVector)
	, SavedRotation(FRotator::ZeroRotator)
	, SavedControlRotation(FRotator::ZeroRotator)
	, SavedRootMotion(FTransform::Identity)
	, bHadAnimRootMotion(false)
{
	FMemory::Memzero(&CompressedFlags, sizeof(FCompressedMoveFlags));
}

void FSavedMove_Character_Fort::Clear()
{
	Super::Clear();

	TimeStamp = 0.0f;
	DeltaTime = 0.0f;
	SavedAcceleration = FVector::ZeroVector;
	SavedLocation = FVector::ZeroVector;
	SavedRotation = FRotator::ZeroRotator;
	SavedControlRotation = FRotator::ZeroRotator;
	FMemory::Memzero(&CompressedFlags, sizeof(FCompressedMoveFlags));
	SavedMovementBase = nullptr;
	SavedMovementBaseBoneName = NAME_None;
	SavedRootMotion = FTransform::Identity;
	bHadAnimRootMotion = false;
}

void FSavedMove_Character_Fort::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UFortCharacterMovementComponent* FortMovement = Cast<UFortCharacterMovementComponent>(Character->GetCharacterMovement());
	if (FortMovement)
	{
		// Store timestamp and delta
		TimeStamp = FortMovement->GetCurrentTimeStamp();
		DeltaTime = InDeltaTime;

		// Store acceleration
		SavedAcceleration = NewAccel;

		// Store location and rotation
		SavedLocation = Character->GetActorLocation();
		SavedRotation = Character->GetActorRotation();

		// Store control rotation
		if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
		{
			SavedControlRotation = PC->GetControlRotation();
		}

		// Store movement base
		SavedMovementBase = Character->GetMovementBase();
		SavedMovementBaseBoneName = Character->GetBasedMovement().BoneName;

		// Compress flags
		CompressedFlags.bPressedJump = Character->bPressedJump;
		CompressedFlags.bWantsToCrouch = FortMovement->bWantsToCrouch;
		CompressedFlags.bForceMaxAccel = FortMovement->bForceMaxAccel;

		// Save root motion state
		bHadAnimRootMotion = FortMovement->bHasAnimRootMotion;
		if (bHadAnimRootMotion)
		{
			SavedRootMotion = FortMovement->AnimRootMotionTransform;

			UE_LOG(LogNetPlayerMovement, VeryVerbose, TEXT("SavedMove SavedRootMotion: Translation=%s, Rotation=%s"),
				*SavedRootMotion.GetTranslation().ToString(),
				*SavedRootMotion.Rotator().ToString());
		}
	}
}

bool FSavedMove_Character_Fort::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	// Don't combine if timestamps are too far apart
	const FSavedMove_Character_Fort* NewFortMove = static_cast<const FSavedMove_Character_Fort*>(NewMove.Get());
	if (NewFortMove && FMath::Abs(NewFortMove->TimeStamp - TimeStamp) > MaxDelta)
	{
		return false;
	}

	// Don't combine if input changed significantly
	if (!SavedAcceleration.Equals(NewFortMove->SavedAcceleration, 0.1f))
	{
		return false;
	}

	// Don't combine if movement base changed
	if (SavedMovementBase != NewFortMove->SavedMovementBase)
	{
		return false;
	}

	// Don't combine if jump state changed
	if (CompressedFlags.bPressedJump != NewFortMove->CompressedFlags.bPressedJump)
	{
		return false;
	}

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void FSavedMove_Character_Fort::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	UFortCharacterMovementComponent* FortMovement = Cast<UFortCharacterMovementComponent>(Character->GetCharacterMovement());
	if (FortMovement)
	{
		// Restore input state
		FortMovement->Acceleration = SavedAcceleration;

		// Restore jump state
		Character->bPressedJump = CompressedFlags.bPressedJump;

		// Restore crouch state
		FortMovement->bWantsToCrouch = CompressedFlags.bWantsToCrouch;
	}
}

//==============================================================================
// FNetworkPredictionData_Client_Fort Implementation
//==============================================================================

FNetworkPredictionData_Client_Fort::FNetworkPredictionData_Client_Fort(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
	, CurrentTimeStamp(0.0f)
	, LastAckedTimeStamp(0.0f)
{
	MaxSavedMoveCount = MaxSavedMoves;
	MaxFreeMoveCount = MaxSavedMoves;
}

FSavedMovePtr FNetworkPredictionData_Client_Fort::AllocateNewMove()
{
	// Allocate Fort-specific saved move
	FSavedMove_Character_Fort* NewMove = new FSavedMove_Character_Fort();
	return FSavedMovePtr(NewMove);
}

//==============================================================================
// UFortCharacterMovementComponent Implementation
//==============================================================================

UFortCharacterMovementComponent::UFortCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentTimeStamp(0.0f)
	, LastAckedTimeStamp(0.0f)
	, LastServerMoveTimeStamp(0.0f)
	, MaxMoveDeltaTime(MAX_DELTA_TIME)
	, NetworkSimulatedSmoothLocationTime(0.100f)
	, NetworkSimulatedSmoothRotationTime(0.033f)
	, NetworkMaxSmoothUpdateDistance(256.0f)
	, NetworkNoSmoothUpdateDistance(4.0f)
	, TeleportDistanceThreshold(512.0f)
	, TeleportRotationThreshold(180.0f)
	, NetworkSmoothingMode(EFortNetworkSmoothingMode::Exponential)
	, bEnableMeshOffsetSmoothing(true)
	, CurrentMovementStyle(EFortMovementStyle::Running)
	, SprintSpeedMultiplier(1.5f)
	, bHasAnimRootMotion(false)
	, AnimRootMotionTransform(FTransform::Identity)
	, AnimRootMotionTranslationScale(1.0f)
	, bEnableNavWalking(true)
	, NavWalkingSearchHeightScale(0.5f)
	, PendingMoveTimeStamp(0.0f)
	, PendingAcceleration(FVector::ZeroVector)
	, bHasPendingMove(false)
{
	// Client prediction enabled
	bUseClientPrediction = true;

	// Network update settings optimized for Fortnite gameplay
	NetworkMaxSmoothUpdateDistance = 256.0f;
	NetworkNoSmoothUpdateDistance = 4.0f;  // Below 4cm: instant snap
	TeleportDistanceThreshold = 512.0f;     // Above 512cm: teleport

	// Enable NavWalking for AI pathfinding
	bRunPhysicsWithNoController = true;  // Allow AI movement
}

void UFortCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update timestamp for autonomous proxy
	if (CharacterOwner && CharacterOwner->Role == ROLE_AutonomousProxy)
	{
		CurrentTimeStamp += DeltaTime;
	}
}

FNetworkPredictionData_Client* UFortCharacterMovementComponent::GetPredictionData_Client() const
{
	if (!ClientPredictionData)
	{
		UFortCharacterMovementComponent* MutableThis = const_cast<UFortCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = MakeShareable(new FNetworkPredictionData_Client_Fort(*this));
	}

	return ClientPredictionData.Get();
}

float UFortCharacterMovementComponent::GetCurrentTimeStamp() const
{
	return CurrentTimeStamp;
}

void UFortCharacterMovementComponent::PerformMovement(float DeltaTime)
{
	// Clamp delta time to prevent exploits
	DeltaTime = FMath::Min(DeltaTime, MaxMoveDeltaTime);

	// Store position before movement for delta calculation
	const FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FRotator OldRotation = UpdatedComponent->GetComponentRotation();

	// Execute core movement logic
	PerformMovementImpl(DeltaTime);

	// Check if position changed
	const FVector NewLocation = UpdatedComponent->GetComponentLocation();
	const bool bLocationChanged = !OldLocation.Equals(NewLocation, KINDA_SMALL_NUMBER);

	// For autonomous proxy, save and send moves
	if (CharacterOwner && CharacterOwner->Role == ROLE_AutonomousProxy && bLocationChanged)
	{
		FNetworkPredictionData_Client_Fort* ClientData = static_cast<FNetworkPredictionData_Client_Fort*>(GetPredictionData_Client());
		if (ClientData)
		{
			// Allocate new saved move
			FSavedMovePtr NewMove = ClientData->CreateSavedMove();
			FSavedMove_Character_Fort* FortMove = static_cast<FSavedMove_Character_Fort*>(NewMove.Get());

			if (FortMove)
			{
				// Store move data
				FortMove->SetMoveFor(CharacterOwner, DeltaTime, Acceleration, *ClientData);

				// Add to saved moves
				ClientData->SavedMoves.Add(NewMove);

				// Trim old moves if buffer is full
				const int32 MaxSaves = FNetworkPredictionData_Client_Fort::MaxSavedMoves;
				if (ClientData->SavedMoves.Num() > MaxSaves)
				{
					UE_LOG(LogNetPlayerMovement, Warning, TEXT("SavedMove buffer overflow (%d moves) - possible high ping or packet loss"),
						ClientData->SavedMoves.Num());

					ClientData->SavedMoves.RemoveAt(0, ClientData->SavedMoves.Num() - MaxSaves);
				}

				// Send move to server
				const uint8 CompressedFlags = CompressInputFlags();
				const uint8 ClientRoll = FRotator::CompressAxisToByte(UpdatedComponent->GetComponentRotation().Roll);
				const uint32 View = PackYawAndPitchTo32(UpdatedComponent->GetComponentRotation().Yaw, UpdatedComponent->GetComponentRotation().Pitch);

				// Check if we can combine with pending move (dual move optimization)
				if (bHasPendingMove && (CurrentTimeStamp - PendingMoveTimeStamp) < 0.1f)
				{
					// Send dual move
					ServerMoveDual(
						PendingMoveTimeStamp,
						FVector_NetQuantize10(PendingAcceleration),
						CompressedFlags,
						View,
						CurrentTimeStamp,
						FVector_NetQuantize10(Acceleration),
						FVector_NetQuantize100(NewLocation),
						CompressedFlags,
						ClientRoll,
						View,
						Cast<UPrimitiveComponent>(CharacterOwner->GetMovementBase()),
						CharacterOwner->GetBasedMovement().BoneName,
						MovementMode
					);

					bHasPendingMove = false;
				}
				else
				{
					// Send single move
					ServerMove(
						CurrentTimeStamp,
						FVector_NetQuantize10(Acceleration),
						FVector_NetQuantize100(NewLocation),
						CompressedFlags,
						ClientRoll,
						View,
						Cast<UPrimitiveComponent>(CharacterOwner->GetMovementBase()),
						CharacterOwner->GetBasedMovement().BoneName,
						MovementMode
					);

					// Store as pending for potential dual move next frame
					PendingMoveTimeStamp = CurrentTimeStamp;
					PendingAcceleration = Acceleration;
					bHasPendingMove = true;
				}
			}
		}
	}

	// Detailed logging for debugging
	UE_LOG(LogNetPlayerMovement, VeryVerbose, TEXT("PerformMovement WorldSpaceRootMotion Translation: %s, Rotation: %s, Actor Facing: %s, Velocity: %s"),
		*NewLocation.ToString(),
		*UpdatedComponent->GetComponentRotation().ToString(),
		*CharacterOwner->GetActorForwardVector().ToString(),
		*Velocity.ToString()
	);
}

void UFortCharacterMovementComponent::PerformMovementImpl(float DeltaTime)
{
	if (!CharacterOwner || !UpdatedComponent)
	{
		return;
	}

	// Check if movement is allowed
	if (UpdatedComponent->IsSimulatingPhysics())
	{
		UE_LOG(LogNetPlayerMovement, Warning, TEXT("UFortCharacterMovementComponent::PerformMovementImpl: UpdateComponent (%s) is simulating physics - aborting."),
			*UpdatedComponent->GetName());
		return;
	}

	// Calculate velocity based on current movement mode
	CalcVelocityImpl(DeltaTime);

	// Execute physics for current movement mode
	switch (MovementMode)
	{
	case MOVE_Walking:
		PhysWalking(DeltaTime, 0);
		break;

	case MOVE_NavWalking:
		if (bEnableNavWalking)
		{
			PhysNavWalking(DeltaTime, 0);
		}
		else
		{
			// Fallback to walking if NavWalking disabled
			PhysWalking(DeltaTime, 0);
		}
		break;

	case MOVE_Falling:
		PhysFalling(DeltaTime, 0);
		break;

	case MOVE_Flying:
		PhysFlying(DeltaTime, 0);
		break;

	case MOVE_None:
	default:
		// No movement
		Velocity = FVector::ZeroVector;
		break;
	}

	// Update character rotation
	if (bOrientRotationToMovement && Velocity.SizeSquared() > KINDA_SMALL_NUMBER)
	{
		FRotator NewRotation = Velocity.Rotation();
		NewRotation.Pitch = 0.0f;
		NewRotation.Roll = 0.0f;

		if (RotationRate.Yaw > 0.0f)
		{
			// Smooth rotation
			NewRotation = FMath::RInterpConstantTo(UpdatedComponent->GetComponentRotation(), NewRotation, DeltaTime, RotationRate.Yaw);
		}

		MoveUpdatedComponent(FVector::ZeroVector, NewRotation, false);
	}
}

void UFortCharacterMovementComponent::CalcVelocityImpl(float DeltaTime)
{
	// Apply acceleration
	if (Acceleration.SizeSquared() > 0.0f)
	{
		// Clamp acceleration to max
		FVector AccelDir = Acceleration;
		const float AccelMag = AccelDir.Size();

		if (AccelMag > GetMaxAcceleration())
		{
			AccelDir = AccelDir / AccelMag * GetMaxAcceleration();
		}

		// Apply acceleration to velocity
		Velocity += AccelDir * DeltaTime;
	}

	// Apply friction based on movement mode
	if (MovementMode == MOVE_Walking)
	{
		// Ground friction
		const float ActualBrakingFriction = (bUseSeparateBrakingFriction ? BrakingFriction : GroundFriction);
		const float FrictionFactor = FMath::Max(0.0f, 1.0f - ActualBrakingFriction * DeltaTime);
		Velocity *= FrictionFactor;
	}
	else if (MovementMode == MOVE_Falling || MovementMode == MOVE_Flying)
	{
		// Air resistance
		Velocity *= FMath::Max(0.0f, 1.0f - (FallingLateralFriction * DeltaTime));
	}

	// Clamp to max speed (consider sprint multiplier)
	float MaxSpeed = GetMaxSpeed();

	// Apply sprint speed multiplier
	if (CurrentMovementStyle == EFortMovementStyle::Sprinting)
	{
		MaxSpeed *= SprintSpeedMultiplier;
	}

	if (Velocity.SizeSquared() > FMath::Square(MaxSpeed))
	{
		Velocity = Velocity.GetSafeNormal() * MaxSpeed;
	}

	// Apply gravity for falling
	if (MovementMode == MOVE_Falling)
	{
		const FVector Gravity(0.0f, 0.0f, GetGravityZ());
		Velocity += Gravity * DeltaTime;
	}
}

void UFortCharacterMovementComponent::PhysWalking(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!CharacterOwner || !UpdatedComponent)
	{
		return;
	}

	// Calculate movement delta
	FVector Delta = Velocity * DeltaTime;

	if (Delta.IsNearlyZero())
	{
		// Still need to check for floor
		return;
	}

	// Perform movement sweep
	FHitResult Hit(1.0f);
	SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

	if (Hit.IsValidBlockingHit())
	{
		// Hit something, slide along surface
		const FVector OldHitNormal = Hit.Normal;
		SlideAlongSurface(Delta, 1.0f - Hit.Time, Hit.Normal, Hit, true);

		// Check if still on walkable surface
		if (Hit.IsValidBlockingHit())
		{
			if (!IsWalkable(Hit))
			{
				// Not walkable, start falling
				SetMovementMode(MOVE_Falling);
			}
		}
	}
}

void UFortCharacterMovementComponent::PhysFalling(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!CharacterOwner || !UpdatedComponent)
	{
		return;
	}

	// Apply gravity
	Velocity.Z += GetGravityZ() * DeltaTime;

	// Calculate movement delta
	FVector Delta = Velocity * DeltaTime;

	// Perform movement sweep
	FHitResult Hit(1.0f);
	SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

	if (Hit.IsValidBlockingHit())
	{
		// Check if landed
		if (IsWalkable(Hit))
		{
			// Landed on walkable surface
			ProcessLanded(Hit, DeltaTime, 0);
			return;
		}
		else
		{
			// Hit wall/ceiling, slide along it
			SlideAlongSurface(Delta, 1.0f - Hit.Time, Hit.Normal, Hit, true);

			// Continue falling
			if (Hit.IsValidBlockingHit() && !IsWalkable(Hit))
			{
				// Bounce off if velocity is high enough
				if (Velocity.Z < -100.0f)
				{
					Velocity.Z *= -0.2f;  // Small bounce
				}
			}
		}
	}
}

void UFortCharacterMovementComponent::PhysFlying(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!CharacterOwner || !UpdatedComponent)
	{
		return;
	}

	// Calculate movement delta (no gravity)
	FVector Delta = Velocity * DeltaTime;

	if (Delta.IsNearlyZero())
	{
		Velocity = FVector::ZeroVector;
		return;
	}

	// Perform movement sweep
	FHitResult Hit(1.0f);
	SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

	if (Hit.IsValidBlockingHit())
	{
		// Hit something, slide along surface
		SlideAlongSurface(Delta, 1.0f - Hit.Time, Hit.Normal, Hit, true);
	}
}

void UFortCharacterMovementComponent::SimulatedTick(float DeltaSeconds)
{
	// Check if simulated proxy
	if (!CharacterOwner || CharacterOwner->Role != ROLE_SimulatedProxy)
	{
		return;
	}

	// Clear old moves if we're simulating
	FNetworkPredictionData_Client_Fort* ClientData = static_cast<FNetworkPredictionData_Client_Fort*>(GetPredictionData_Client());
	if (ClientData && ClientData->SavedMoves.Num() > 0)
	{
		UE_LOG(LogNetPlayerMovement, Log, TEXT("Clearing old moves in SimulatedTick (%d)"), ClientData->SavedMoves.Num());
		ClientData->SavedMoves.Empty();
	}

	// Update network smoothing for simulated proxies
	if (SmoothingState.bIsSmoothingActive)
	{
		SmoothClientPosition(DeltaSeconds);
	}

	// Update mesh offset smoothing if enabled
	if (bEnableMeshOffsetSmoothing)
	{
		UpdateMeshOffsetSmoothing(DeltaSeconds);
	}

	// Call base implementation for smooth replication
	Super::SimulatedTick(DeltaSeconds);
}

uint8 UFortCharacterMovementComponent::CompressInputFlags() const
{
	uint8 Result = 0;

	if (CharacterOwner && CharacterOwner->bPressedJump)
	{
		Result |= (1 << 0);
	}

	if (bWantsToCrouch)
	{
		Result |= (1 << 1);
	}

	if (bForceMaxAccel)
	{
		Result |= (1 << 2);
	}

	return Result;
}

void UFortCharacterMovementComponent::DecompressInputFlags(uint8 Flags)
{
	if (CharacterOwner)
	{
		CharacterOwner->bPressedJump = (Flags & (1 << 0)) != 0;
	}

	bWantsToCrouch = (Flags & (1 << 1)) != 0;
	bForceMaxAccel = (Flags & (1 << 2)) != 0;
}

int32 UFortCharacterMovementComponent::FindSavedMoveByTimeStamp(float TimeStamp) const
{
	const FNetworkPredictionData_Client_Fort* ClientData = static_cast<const FNetworkPredictionData_Client_Fort*>(GetPredictionData_Client());
	if (!ClientData)
	{
		return -1;
	}

	// Search saved moves for matching timestamp
	for (int32 i = 0; i < ClientData->SavedMoves.Num(); ++i)
	{
		const FSavedMove_Character_Fort* FortMove = static_cast<const FSavedMove_Character_Fort*>(ClientData->SavedMoves[i].Get());
		if (FortMove && FMath::IsNearlyEqual(FortMove->TimeStamp, TimeStamp, KINDA_SMALL_NUMBER))
		{
			return i;
		}
	}

	return -1;
}

//==============================================================================
// Network Smoothing Implementation
//==============================================================================

void UFortCharacterMovementComponent::SmoothClientPosition(float DeltaTime)
{
	if (!SmoothingState.bIsSmoothingActive || !UpdatedComponent)
	{
		return;
	}

	// Decrease remaining smoothing time
	SmoothingState.SmoothingTimeRemaining = FMath::Max(0.0f, SmoothingState.SmoothingTimeRemaining - DeltaTime);

	// Calculate interpolation alpha
	float Alpha = 0.0f;
	if (SmoothingState.TotalSmoothingTime > 0.0f)
	{
		const float ElapsedTime = SmoothingState.TotalSmoothingTime - SmoothingState.SmoothingTimeRemaining;

		switch (NetworkSmoothingMode)
		{
		case EFortNetworkSmoothingMode::Linear:
			// Linear interpolation
			Alpha = ElapsedTime / SmoothingState.TotalSmoothingTime;
			break;

		case EFortNetworkSmoothingMode::Exponential:
			// Exponential decay for smoother, more natural corrections
			// Alpha = 1 - e^(-k*t) where k controls smoothing speed
			{
				const float SmoothingSpeed = 8.0f;  // Higher = faster convergence
				Alpha = 1.0f - FMath::Exp(-SmoothingSpeed * ElapsedTime / SmoothingState.TotalSmoothingTime);
			}
			break;

		case EFortNetworkSmoothingMode::Disabled:
		default:
			// No smoothing
			Alpha = 1.0f;
			break;
		}

		Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
	}
	else
	{
		Alpha = 1.0f;  // Instant correction
	}

	// Get current position
	const FVector CurrentLocation = UpdatedComponent->GetComponentLocation();
	const FQuat CurrentRotation = UpdatedComponent->GetComponentQuat();

	// Interpolate toward target
	const FVector NewLocation = FMath::Lerp(CurrentLocation, SmoothingState.TargetLocation, Alpha);
	const FQuat NewRotation = FQuat::Slerp(CurrentRotation, SmoothingState.TargetRotation, Alpha);

	// Apply smoothed position
	UpdatedComponent->SetWorldLocationAndRotation(NewLocation, NewRotation, false);

	// Update velocity based on movement
	if (DeltaTime > KINDA_SMALL_NUMBER)
	{
		Velocity = (NewLocation - CurrentLocation) / DeltaTime;
	}

	// Check if smoothing is complete
	if (SmoothingState.SmoothingTimeRemaining <= 0.0f || Alpha >= 1.0f)
	{
		// Smoothing complete - snap to final position
		UpdatedComponent->SetWorldLocationAndRotation(SmoothingState.TargetLocation, SmoothingState.TargetRotation, false);
		SmoothingState.bIsSmoothingActive = false;

		UE_LOG(LogNetPlayerMovement, VeryVerbose, TEXT("SmoothClientPosition: Smoothing complete at %s"),
			*SmoothingState.TargetLocation.ToString());
	}
}

void UFortCharacterMovementComponent::ApplyNetworkCorrection(const FVector& NewLocation, const FRotator& NewRotation, const FVector& NewVelocity)
{
	if (!UpdatedComponent || !CharacterOwner)
	{
		return;
	}

	// Get current state
	const FVector CurrentLocation = UpdatedComponent->GetComponentLocation();
	const FRotator CurrentRotation = UpdatedComponent->GetComponentRotation();

	// Calculate errors
	const float LocationError = (NewLocation - CurrentLocation).Size();
	const float RotationError = FMath::Abs((NewRotation - CurrentRotation).GetManhattanDistance());

	UE_LOG(LogNetPlayerMovement, Verbose, TEXT("ApplyNetworkCorrection: LocationError=%.2f, RotationError=%.2f"),
		LocationError, RotationError);

	// Determine correction strategy based on error magnitude
	bool bShouldTeleport = false;
	bool bShouldSnap = false;
	bool bShouldSmooth = false;

	// Check teleport thresholds (large errors = instant teleport)
	if (LocationError > TeleportDistanceThreshold || RotationError > TeleportRotationThreshold)
	{
		bShouldTeleport = true;
		UE_LOG(LogNetPlayerMovement, Log, TEXT("ApplyNetworkCorrection: TELEPORT (error too large)"));
	}
	// Check snap threshold (tiny errors = instant snap, no smoothing overhead)
	else if (LocationError < NetworkNoSmoothUpdateDistance)
	{
		bShouldSnap = true;
		UE_LOG(LogNetPlayerMovement, VeryVerbose, TEXT("ApplyNetworkCorrection: SNAP (error too small to smooth)"));
	}
	// Mid-range errors get smoothed
	else if (LocationError <= NetworkMaxSmoothUpdateDistance)
	{
		bShouldSmooth = true;
		UE_LOG(LogNetPlayerMovement, Verbose, TEXT("ApplyNetworkCorrection: SMOOTH (error in smoothing range)"));
	}
	else
	{
		// Outside smoothing range but below teleport - instant correction
		bShouldSnap = true;
		UE_LOG(LogNetPlayerMovement, Log, TEXT("ApplyNetworkCorrection: SNAP (error beyond smooth range)"));
	}

	// Apply correction strategy
	if (bShouldTeleport || bShouldSnap || NetworkSmoothingMode == EFortNetworkSmoothingMode::Disabled)
	{
		// Instant correction (no smoothing)
		UpdatedComponent->SetWorldLocationAndRotation(NewLocation, NewRotation, false);
		Velocity = NewVelocity;
		SmoothingState.bIsSmoothingActive = false;
	}
	else if (bShouldSmooth)
	{
		// Smooth correction over time
		SmoothingState.TargetLocation = NewLocation;
		SmoothingState.TargetRotation = NewRotation.Quaternion();
		SmoothingState.TotalSmoothingTime = NetworkSimulatedSmoothLocationTime;
		SmoothingState.SmoothingTimeRemaining = NetworkSimulatedSmoothLocationTime;
		SmoothingState.bIsSmoothingActive = true;

		// If mesh offset smoothing is enabled, initialize mesh offset
		if (bEnableMeshOffsetSmoothing && CharacterOwner->GetMesh())
		{
			// Calculate initial mesh offset (current capsule position - target position)
			SmoothingState.MeshRelativeOffset = CurrentLocation - NewLocation;
			SmoothingState.MeshRotationOffset = (CurrentRotation.Quaternion() * NewRotation.Quaternion().Inverse());
		}

		// Start smoothing from current position toward target
		// The actual interpolation happens in SmoothClientPosition each frame
	}
}

void UFortCharacterMovementComponent::UpdateMeshOffsetSmoothing(float DeltaTime)
{
	if (!CharacterOwner || !CharacterOwner->GetMesh())
	{
		return;
	}

	USkeletalMeshComponent* Mesh = CharacterOwner->GetMesh();

	// If we have an active mesh offset, smooth it back to zero
	if (!SmoothingState.MeshRelativeOffset.IsNearlyZero() || !SmoothingState.MeshRotationOffset.Equals(FQuat::Identity, KINDA_SMALL_NUMBER))
	{
		// Smoothly return mesh to capsule position
		const float MeshSmoothSpeed = 10.0f;  // Speed of mesh offset reduction
		const float Alpha = FMath::Clamp(DeltaTime * MeshSmoothSpeed, 0.0f, 1.0f);

		// Interpolate offset toward zero
		SmoothingState.MeshRelativeOffset = FMath::Lerp(SmoothingState.MeshRelativeOffset, FVector::ZeroVector, Alpha);
		SmoothingState.MeshRotationOffset = FQuat::Slerp(SmoothingState.MeshRotationOffset, FQuat::Identity, Alpha);

		// Apply mesh offset (visual only, doesn't affect physics)
		Mesh->SetRelativeLocation(SmoothingState.MeshRelativeOffset);
		Mesh->SetRelativeRotation(SmoothingState.MeshRotationOffset);

		UE_LOG(LogNetPlayerMovement, VeryVerbose, TEXT("UpdateMeshOffsetSmoothing: Offset=%s"),
			*SmoothingState.MeshRelativeOffset.ToString());
	}
	else
	{
		// Ensure mesh is at zero offset
		if (!Mesh->GetRelativeLocation().IsNearlyZero() || !Mesh->GetRelativeRotation().IsNearlyZero())
		{
			Mesh->SetRelativeLocation(FVector::ZeroVector);
			Mesh->SetRelativeRotation(FRotator::ZeroRotator);
		}
	}
}

void UFortCharacterMovementComponent::ReplaySavedMoves(int32 StartMoveIndex)
{
	FNetworkPredictionData_Client_Fort* ClientData = static_cast<FNetworkPredictionData_Client_Fort*>(GetPredictionData_Client());
	if (!ClientData)
	{
		return;
	}

	// Replay moves from start index to end
	for (int32 i = StartMoveIndex; i < ClientData->SavedMoves.Num(); ++i)
	{
		FSavedMove_Character_Fort* FortMove = static_cast<FSavedMove_Character_Fort*>(ClientData->SavedMoves[i].Get());
		if (FortMove)
		{
			// Prepare move (restore input state)
			FortMove->PrepMoveFor(CharacterOwner);

			// Restore root motion if present
			if (FortMove->bHadAnimRootMotion)
			{
				bHasAnimRootMotion = true;
				AnimRootMotionTransform = FortMove->SavedRootMotion;
			}

			// Re-execute movement
			PerformMovementImpl(FortMove->DeltaTime);
		}
	}

	UE_LOG(LogNetPlayerMovement, Log, TEXT("Replayed %d saved moves"), ClientData->SavedMoves.Num() - StartMoveIndex);
}

//==============================================================================
// Root Motion & Custom Movement Modes
//==============================================================================

void UFortCharacterMovementComponent::ApplyRootMotionToVelocity(const FTransform& RootMotionTransform, float DeltaTime)
{
	if (DeltaTime < KINDA_SMALL_NUMBER)
	{
		return;
	}

	// Store root motion transform for this frame
	AnimRootMotionTransform = RootMotionTransform;
	bHasAnimRootMotion = true;

	// Extract translation and apply scale
	FVector RootMotionTranslation = RootMotionTransform.GetTranslation() * AnimRootMotionTranslationScale;

	// Convert root motion to velocity
	FVector RootMotionVelocity = RootMotionTranslation / DeltaTime;

	// Add to current velocity (root motion is additive)
	Velocity += RootMotionVelocity;

	UE_LOG(LogNetPlayerMovement, VeryVerbose,
		TEXT("ApplyRootMotionToVelocity: Translation=%s, Velocity=%s"),
		*RootMotionTranslation.ToString(), *Velocity.ToString());
}

void UFortCharacterMovementComponent::SetFortMovementStyle(EFortMovementStyle NewStyle)
{
	if (CurrentMovementStyle == NewStyle)
	{
		return;
	}

	const EFortMovementStyle OldStyle = CurrentMovementStyle;
	CurrentMovementStyle = NewStyle;

	// Apply style-specific speed modifiers
	switch (NewStyle)
	{
	case EFortMovementStyle::Walking:
		MaxWalkSpeed = 300.0f;
		break;

	case EFortMovementStyle::Running:
		MaxWalkSpeed = 600.0f;
		break;

	case EFortMovementStyle::Sprinting:
		MaxWalkSpeed = 600.0f;  // Base speed, multiplier applied in CalcVelocity
		break;

	case EFortMovementStyle::Flying:
		SetMovementMode(MOVE_Flying);
		MaxFlySpeed = 600.0f;
		break;

	default:
		break;
	}

	UE_LOG(LogNetPlayerMovement, Log, TEXT("SetFortMovementStyle: %d -> %d"), (int32)OldStyle, (int32)NewStyle);
}

void UFortCharacterMovementComponent::PhysNavWalking(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!CharacterOwner || !UpdatedComponent)
	{
		return;
	}

	// NavWalking constrains movement to navigation mesh surface
	// This is critical for AI pathfinding to ensure agents stay on valid paths

	// Calculate desired velocity
	CalcVelocityImpl(DeltaTime);

	// Calculate movement delta
	FVector Delta = Velocity * DeltaTime;

	if (Delta.IsNearlyZero())
	{
		return;
	}

	// Get current position
	const FVector CurrentLocation = UpdatedComponent->GetComponentLocation();

	// Calculate desired new position
	const FVector DesiredLocation = CurrentLocation + Delta;

	// Project desired location onto NavMesh
	FNavLocation ProjectedNavLocation;
	if (ProjectPointToNavMesh(DesiredLocation, ProjectedNavLocation))
	{
		// Successfully projected onto NavMesh - constrain movement to surface
		const FVector ConstrainedDelta = ProjectedNavLocation.Location - CurrentLocation;

		// Perform constrained movement
		FHitResult Hit(1.0f);
		SafeMoveUpdatedComponent(ConstrainedDelta, UpdatedComponent->GetComponentQuat(), true, Hit);

		if (Hit.IsValidBlockingHit())
		{
			// Hit obstacle, slide along surface
			const FVector OldHitNormal = Hit.Normal;
			SlideAlongSurface(ConstrainedDelta, 1.0f - Hit.Time, Hit.Normal, Hit, true);

			// Check if still on walkable surface
			if (Hit.IsValidBlockingHit() && !IsWalkable(Hit))
			{
				// Not walkable, start falling
				SetMovementMode(MOVE_Falling);
				return;
			}
		}

		UE_LOG(LogNetPlayerMovement, VeryVerbose,
			TEXT("PhysNavWalking: Constrained movement from %s to %s (NavMesh)"),
			*CurrentLocation.ToString(), *ProjectedNavLocation.Location.ToString());
	}
	else
	{
		// Failed to project onto NavMesh - fall back to walking or falling
		UE_LOG(LogNetPlayerMovement, Verbose,
			TEXT("PhysNavWalking: Cannot project to NavMesh at %s, leaving NavWalking mode"),
			*DesiredLocation.ToString());

		SetMovementMode(MOVE_Walking);
	}

	// Verify we should still be in NavWalking mode
	if (!CanStartNavWalking())
	{
		UE_LOG(LogNetPlayerMovement, Verbose,
			TEXT("PhysNavWalking: No longer on valid NavMesh - exiting NavWalking mode"));
		SetMovementMode(MOVE_Walking);
	}
}

bool UFortCharacterMovementComponent::CanStartNavWalking() const
{
	if (!bEnableNavWalking)
	{
		return false;
	}

	if (!CharacterOwner)
	{
		return false;
	}

	// NavWalking requires:
	// 1. Valid NavMesh in world
	// 2. Character is on or near NavMesh surface
	// 3. Character has controller (AI or player for navigation)

	// Check if NavMesh exists
	ARecastNavMesh* NavMesh = GetNavMesh();
	if (!NavMesh)
	{
		UE_LOG(LogNetPlayerMovement, VeryVerbose,
			TEXT("CanStartNavWalking: No NavMesh in world"));
		return false;
	}

	// Check if character location is on NavMesh
	const FVector CharacterLocation = CharacterOwner->GetActorLocation();
	FNavLocation NavLocation;

	// Project current location onto NavMesh with reasonable search extent
	const FVector SearchExtent(50.0f, 50.0f, NavWalkingSearchHeightScale * 200.0f);
	if (!ProjectPointToNavMesh(CharacterLocation, NavLocation, SearchExtent))
	{
		UE_LOG(LogNetPlayerMovement, VeryVerbose,
			TEXT("CanStartNavWalking: Character not on NavMesh at %s"),
			*CharacterLocation.ToString());
		return false;
	}

	// Check distance to NavMesh (should be within reasonable range)
	const float DistanceToNavMesh = FMath::Abs(NavLocation.Location.Z - CharacterLocation.Z);
	if (DistanceToNavMesh > 100.0f)  // 1 meter tolerance
	{
		UE_LOG(LogNetPlayerMovement, VeryVerbose,
			TEXT("CanStartNavWalking: Too far from NavMesh (%.1f cm)"),
			DistanceToNavMesh);
		return false;
	}

	// NavWalking primarily for AI, but can be used for player navigation too
	const bool bHasController = (CharacterOwner->GetController() != nullptr);
	if (!bHasController)
	{
		UE_LOG(LogNetPlayerMovement, VeryVerbose,
			TEXT("CanStartNavWalking: No controller"));
		return false;
	}

	UE_LOG(LogNetPlayerMovement, VeryVerbose,
		TEXT("CanStartNavWalking: Can start - on NavMesh at %s (projected from %s)"),
		*NavLocation.Location.ToString(), *CharacterLocation.ToString());

	return true;
}

bool UFortCharacterMovementComponent::ProjectPointToNavMesh(const FVector& Point, FNavLocation& OutLocation, const FVector& Extent) const
{
	ARecastNavMesh* NavMesh = GetNavMesh();
	if (!NavMesh)
	{
		return false;
	}

	// Delegate to NavMesh's ProjectPoint method
	return NavMesh->ProjectPoint(Point, OutLocation, Extent);
}

ARecastNavMesh* UFortCharacterMovementComponent::GetNavMesh() const
{
	// Check cached NavMesh
	if (CachedNavMesh.IsValid())
	{
		return CachedNavMesh.Get();
	}

	// Find world's navigation system
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UNavigationSystem* NavSys = World->GetNavigationSystem();
	if (!NavSys)
	{
		UE_LOG(LogNetPlayerMovement, VeryVerbose,
			TEXT("GetNavMesh: No NavigationSystem in world"));
		return nullptr;
	}

	// Get main navigation data (should be RecastNavMesh)
	ANavigationData* MainNavData = NavSys->GetMainNavData(ENavigationDataResolution::Default);
	if (!MainNavData)
	{
		UE_LOG(LogNetPlayerMovement, VeryVerbose,
			TEXT("GetNavMesh: No main navigation data"));
		return nullptr;
	}

	// Cast to RecastNavMesh
	::ARecastNavMesh* RecastNav = Cast<::ARecastNavMesh>(MainNavData);
	if (RecastNav)
	{
		// Cache for future use
		CachedNavMesh = RecastNav;
		UE_LOG(LogNetPlayerMovement, VeryVerbose,
			TEXT("GetNavMesh: Found and cached RecastNavMesh"));
		return RecastNav;
	}

	UE_LOG(LogNetPlayerMovement, Warning,
		TEXT("GetNavMesh: Main navigation data is not RecastNavMesh (type: %s)"),
		*MainNavData->GetClass()->GetName());

	return nullptr;
}

//==============================================================================
// Server Move RPC Implementations
//==============================================================================

void UFortCharacterMovementComponent::ServerMove_Implementation(
	float TimeStamp,
	FVector_NetQuantize10 Acceleration,
	FVector_NetQuantize100 ClientLoc,
	uint8 CompressedMoveFlags,
	uint8 ClientRoll,
	uint32 View,
	UPrimitiveComponent* ClientMovementBase,
	FName ClientBaseBoneName,
	uint8 ClientMovementMode)
{
	if (!CharacterOwner)
	{
		return;
	}

	// Validate timestamp
	if (TimeStamp <= LastServerMoveTimeStamp)
	{
		UE_LOG(LogNetPlayerMovement, Warning, TEXT("ServerMove: TimeStamp %f is older than last move %f"),
			TimeStamp, LastServerMoveTimeStamp);
		return;
	}

	// Calculate delta time
	float DeltaTime = TimeStamp - LastServerMoveTimeStamp;
	DeltaTime = FMath::Clamp(DeltaTime, 0.0f, MaxMoveDeltaTime);

	// Update last timestamp
	LastServerMoveTimeStamp = TimeStamp;

	// Decompress input
	this->Acceleration = FVector(Acceleration);
	DecompressInputFlags(CompressedMoveFlags);

	UE_LOG(LogNetPlayerMovement, VeryVerbose, TEXT("ServerMove Time %f Acceleration %s Position %s DeltaTime %f"),
		TimeStamp, *Acceleration.ToString(), *ClientLoc.ToString(), DeltaTime);

	// Store position before server movement
	const FVector ServerLocation = UpdatedComponent->GetComponentLocation();

	// Execute movement on server
	PerformMovementImpl(DeltaTime);

	// Get position after server movement
	const FVector NewServerLocation = UpdatedComponent->GetComponentLocation();

	// Check for position mismatch
	const float PositionErrorSquared = (NewServerLocation - FVector(ClientLoc)).SizeSquared();

	if (PositionErrorSquared > MAX_POSITION_ERROR_SQUARED)
	{
		// Position mismatch - send correction
		UE_LOG(LogNetPlayerMovement, Log, TEXT("ServerMove correction: Client at %s, Server at %s (error: %.2f)"),
			*ClientLoc.ToString(), *NewServerLocation.ToString(), FMath::Sqrt(PositionErrorSquared));

		ClientAdjustPosition(
			TimeStamp,
			NewServerLocation,
			Velocity,
			GetMovementBase(),
			GetMovementBaseOwner(GetMovementBase()) ? GetMovementBaseOwner(GetMovementBase())->GetBasedMovement().BoneName : NAME_None,
			GetMovementBase() != nullptr,
			false,  // absolute position
			MovementMode
		);
	}
	else
	{
		// Position correct - acknowledge good move
		ClientAckGoodMove(TimeStamp, NewServerLocation, Velocity, MovementMode);
	}
}

bool UFortCharacterMovementComponent::ServerMove_Validate(
	float TimeStamp,
	FVector_NetQuantize10 Acceleration,
	FVector_NetQuantize100 ClientLoc,
	uint8 CompressedMoveFlags,
	uint8 ClientRoll,
	uint32 View,
	UPrimitiveComponent* ClientMovementBase,
	FName ClientBaseBoneName,
	uint8 ClientMovementMode)
{
	// Basic validation
	if (TimeStamp < 0.0f || TimeStamp > 10000.0f)
	{
		return false;
	}

	// Validate acceleration magnitude
	if (Acceleration.SizeSquared() > FMath::Square(GetMaxAcceleration() * 2.0f))
	{
		return false;
	}

	return true;
}

void UFortCharacterMovementComponent::ServerMoveDual_Implementation(
	float TimeStamp0,
	FVector_NetQuantize10 Acceleration0,
	uint8 PendingFlags,
	uint32 View0,
	float TimeStamp,
	FVector_NetQuantize10 Acceleration,
	FVector_NetQuantize100 ClientLoc,
	uint8 NewFlags,
	uint8 ClientRoll,
	uint32 View,
	UPrimitiveComponent* ClientMovementBase,
	FName ClientBaseBoneName,
	uint8 ClientMovementMode)
{
	// Execute first move
	ServerMove_Implementation(
		TimeStamp0,
		Acceleration0,
		ClientLoc,  // Use final location for both (optimization)
		PendingFlags,
		ClientRoll,
		View0,
		ClientMovementBase,
		ClientBaseBoneName,
		ClientMovementMode
	);

	// Execute second move
	ServerMove_Implementation(
		TimeStamp,
		Acceleration,
		ClientLoc,
		NewFlags,
		ClientRoll,
		View,
		ClientMovementBase,
		ClientBaseBoneName,
		ClientMovementMode
	);
}

bool UFortCharacterMovementComponent::ServerMoveDual_Validate(
	float TimeStamp0,
	FVector_NetQuantize10 Acceleration0,
	uint8 PendingFlags,
	uint32 View0,
	float TimeStamp,
	FVector_NetQuantize10 Acceleration,
	FVector_NetQuantize100 ClientLoc,
	uint8 NewFlags,
	uint8 ClientRoll,
	uint32 View,
	UPrimitiveComponent* ClientMovementBase,
	FName ClientBaseBoneName,
	uint8 ClientMovementMode)
{
	// Validate both timestamps
	return ServerMove_Validate(TimeStamp0, Acceleration0, ClientLoc, PendingFlags, ClientRoll, View0, ClientMovementBase, ClientBaseBoneName, ClientMovementMode)
		&& ServerMove_Validate(TimeStamp, Acceleration, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
}

void UFortCharacterMovementComponent::ServerMoveOld_Implementation(
	float OldTimeStamp,
	FVector_NetQuantize10 OldAccel,
	uint8 OldMoveFlags)
{
	UE_LOG(LogNetPlayerMovement, Verbose, TEXT("ServerMoveOld: Timestamp %f"), OldTimeStamp);

	// Process old move (for high ping scenarios)
	// replay from this point
}

bool UFortCharacterMovementComponent::ServerMoveOld_Validate(
	float OldTimeStamp,
	FVector_NetQuantize10 OldAccel,
	uint8 OldMoveFlags)
{
	return OldTimeStamp >= 0.0f;
}

//==============================================================================
// Client Correction RPC Implementations
//==============================================================================

void UFortCharacterMovementComponent::ClientAdjustPosition_Implementation(
	float TimeStamp,
	FVector NewLoc,
	FVector NewVel,
	UPrimitiveComponent* NewBase,
	FName NewBaseBoneName,
	bool bHasBase,
	bool bBaseRelativePosition,
	uint8 ServerMovementMode)
{
	if (!CharacterOwner)
	{
		return;
	}

	// Check if this is for an autonomous proxy (local player) or simulated proxy (other players)
	const bool bIsAutonomousProxy = (CharacterOwner->Role == ROLE_AutonomousProxy);

	if (bIsAutonomousProxy)
	{
		// Autonomous proxy: Find saved move and replay
		const int32 MoveIndex = FindSavedMoveByTimeStamp(TimeStamp);

		if (MoveIndex == -1)
		{
			UE_LOG(LogNetPlayerMovement, Warning,
				TEXT("ClientAdjustPosition_Implementation could not find Move for TimeStamp: %f, LastAckedTimeStamp: %f, CurrentTimeStamp: %f"),
				TimeStamp, LastAckedTimeStamp, CurrentTimeStamp);

			// Apply correction anyway (better than nothing)
			UpdatedComponent->SetWorldLocation(NewLoc, false);
			Velocity = NewVel;
			SetMovementMode(EMovementMode(ServerMovementMode));
			return;
		}

		UE_LOG(LogNetPlayerMovement, Log, TEXT("ClientAdjustPosition: Correcting autonomous proxy to %s at TimeStamp %f (found at move %d)"),
			*NewLoc.ToString(), TimeStamp, MoveIndex);

		// Apply corrected state (instant for autonomous proxy)
		UpdatedComponent->SetWorldLocation(NewLoc, false);
		Velocity = NewVel;
		SetMovementMode(EMovementMode(ServerMovementMode));

		// Set movement base
		if (bHasBase && NewBase)
		{
			CharacterOwner->SetBase(NewBase, NewBaseBoneName);
		}

		// Clear moves before the corrected one
		FNetworkPredictionData_Client_Fort* ClientData = static_cast<FNetworkPredictionData_Client_Fort*>(GetPredictionData_Client());
		if (ClientData)
		{
			ClientData->SavedMoves.RemoveAt(0, MoveIndex);
			LastAckedTimeStamp = TimeStamp;
		}

		// Replay moves after correction
		ReplaySavedMoves(0);  // Start from 0 since we removed old moves
	}
	else
	{
		// Simulated proxy: Apply smooth correction
		UE_LOG(LogNetPlayerMovement, Verbose, TEXT("ClientAdjustPosition: Smoothing simulated proxy to %s"),
			*NewLoc.ToString());

		// Set movement mode
		SetMovementMode(EMovementMode(ServerMovementMode));

		// Set movement base
		if (bHasBase && NewBase)
		{
			CharacterOwner->SetBase(NewBase, NewBaseBoneName);
		}

		// Apply correction with smoothing (for visual quality)
		const FRotator NewRot = UpdatedComponent->GetComponentRotation();  // Keep current rotation
		ApplyNetworkCorrection(NewLoc, NewRot, NewVel);
	}
}

void UFortCharacterMovementComponent::ClientAckGoodMove_Implementation(
	float TimeStamp,
	FVector NewLoc,
	FVector NewVel,
	uint8 ServerMovementMode)
{
	// Find acknowledged move
	const int32 MoveIndex = FindSavedMoveByTimeStamp(TimeStamp);

	if (MoveIndex != -1)
	{
		// Free acknowledged moves
		FNetworkPredictionData_Client_Fort* ClientData = static_cast<FNetworkPredictionData_Client_Fort*>(GetPredictionData_Client());
		if (ClientData)
		{
			ClientData->SavedMoves.RemoveAt(0, MoveIndex + 1);
			LastAckedTimeStamp = TimeStamp;

			UE_LOG(LogNetPlayerMovement, VeryVerbose, TEXT("ClientAckGoodMove: Timestamp %f acknowledged, freed %d moves"),
				TimeStamp, MoveIndex + 1);
		}
	}
	else
	{
		UE_LOG(LogNetPlayerMovement, Warning,
			TEXT("ClientAckGoodMove_Implementation could not find Move for TimeStamp: %f, LastAckedTimeStamp: %f, CurrentTimeStamp: %f"),
			TimeStamp, LastAckedTimeStamp, CurrentTimeStamp);
	}
}
