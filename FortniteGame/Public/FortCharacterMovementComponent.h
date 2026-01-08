// Copyright Epic Games, Inc. All Rights Reserved.
// Reconstructed from Fortnite UE 4.12 IDA
// Client-side movement prediction and physics system

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "FortCharacterMovementComponent.generated.h"

// Forward declarations
class AFortPawn;
class FSavedMove_Character_Fort;
class ARecastNavMesh;
struct FNavLocation;

/**
 * Network smoothing mode for simulated proxies
 * Based on decompiled ENetworkSmoothingMode references
 */
UENUM(BlueprintType)
enum class EFortNetworkSmoothingMode : uint8
{
	/** No smoothing - instant position updates */
	Disabled,

	/** Linear interpolation over time */
	Linear,

	/** Exponential decay interpolation (smoother, more natural) */
	Exponential
};

/**
 * Custom Fortnite movement styles
 * Extends base movement modes with game-specific behaviors
 * Reference: EFortMovementStyle from decompiled code
 */
UENUM(BlueprintType)
enum class EFortMovementStyle : uint8
{
	/** Normal running movement */
	Running,

	/** Walking (slower, quieter) */
	Walking,

	/** Charging (sprinting with special effects) */
	Charging,

	/** Sprinting (faster than running) */
	Sprinting,

	/** Personal vehicle movement */
	PersonalVehicle,

	/** Flying movement */
	Flying,

	/** Tethered movement (ziplines, grind rails) */
	Tethered
};

/**
 * Network smoothing state for simulated proxy corrections
 * Tracks mesh offset for visual smoothing while capsule remains accurate
 */
struct FNetworkSmoothingState
{
	// Target location to smooth toward
	FVector TargetLocation;

	// Target rotation to smooth toward
	FQuat TargetRotation;

	// Current mesh offset from capsule (for visual smoothing)
	FVector MeshRelativeOffset;

	// Current mesh rotation offset from capsule
	FQuat MeshRotationOffset;

	// Time remaining for smoothing
	float SmoothingTimeRemaining;

	// Total time allocated for this correction
	float TotalSmoothingTime;

	// Whether we're currently smoothing
	bool bIsSmoothingActive;

	FNetworkSmoothingState()
		: TargetLocation(FVector::ZeroVector)
		, TargetRotation(FQuat::Identity)
		, MeshRelativeOffset(FVector::ZeroVector)
		, MeshRotationOffset(FQuat::Identity)
		, SmoothingTimeRemaining(0.0f)
		, TotalSmoothingTime(0.0f)
		, bIsSmoothingActive(false)
	{
	}
};

/**
 * Compressed movement flags for network optimization
 * Based on decompiled ServerMove RPC parameters
 */
struct FCompressedMoveFlags
{
	uint32 bPressedJump : 1;
	uint32 bWantsToCrouch : 1;
	uint32 bForceMaxAccel : 1;
	uint32 bIgnoreBaseRotation : 1;
	uint32 Reserved : 28;
};

/**
 * Saved move structure for client prediction
 * Stores input and state for replaying moves after server corrections
 */
class FSavedMove_Character_Fort : public FSavedMove_Character
{
public:
	typedef FSavedMove_Character Super;

	// Timestamp for this move (client time)
	float TimeStamp;

	// Delta time for this move
	float DeltaTime;

	// Input acceleration for this move
	FVector SavedAcceleration;

	// Location at the time of this move
	FVector SavedLocation;

	// Rotation at the time of this move
	FRotator SavedRotation;

	// Control rotation at the time of this move
	FRotator SavedControlRotation;

	// Compressed movement flags
	FCompressedMoveFlags CompressedFlags;

	// Movement base (for moving platforms)
	TWeakObjectPtr<UPrimitiveComponent> SavedMovementBase;
	FName SavedMovementBaseBoneName;

	// Root motion data for animation-driven movement
	FTransform SavedRootMotion;
	bool bHadAnimRootMotion;

	// Constructor
	FSavedMove_Character_Fort();

	// FSavedMove_Character interface
	virtual void Clear() override;
	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;
	virtual void PrepMoveFor(ACharacter* Character) override;
};

/**
 * Client prediction data structure
 * Manages buffering and replay of saved moves
 */
class FNetworkPredictionData_Client_Fort : public FNetworkPredictionData_Client_Character
{
public:
	typedef FNetworkPredictionData_Client_Character Super;

	// Constructor
	FNetworkPredictionData_Client_Fort(const UCharacterMovementComponent& ClientMovement);

	// Current timestamp
	float CurrentTimeStamp;

	// Last acknowledged timestamp from server
	float LastAckedTimeStamp;

	// Maximum number of saved moves to buffer
	static const int32 MaxSavedMoves = 96;

	// Create new saved move
	virtual FSavedMovePtr AllocateNewMove() override;
};

/**
 * Fortnite Character Movement Component
 * Implements client-side prediction, server validation, and physics simulation
 *
 * Based on reverse-engineered CharacterMovementComponent from UE 4.12 build
 * Reference: CharacterMovementComponent.cpp debug strings and pseudocode analysis
 */
UCLASS()
class FORTNITEGAME_API UFortCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UFortCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

	//~ Begin UActorComponent Interface
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent Interface

	//~ Begin UCharacterMovementComponent Interface
	virtual void PerformMovement(float DeltaTime) override;
	virtual void SimulatedTick(float DeltaSeconds) override;
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	//~ End UCharacterMovementComponent Interface

	/**
	 * Core movement execution - applies physics and updates position
	 * Called for both predicted (client) and authoritative (server) movement
	 *
	 * Implementation based on decompiled function at 0157DB40
	 * @param DeltaTime Time step for this movement update
	 */
	void PerformMovementImpl(float DeltaTime);

	/**
	 * Physics mode: Walking on ground
	 * Handles acceleration, friction, and ground detection
	 *
	 * @param DeltaTime Time step for physics update
	 * @param Iterations Number of sub-steps for collision resolution
	 */
	void PhysWalking(float DeltaTime, int32 Iterations);

	/**
	 * Physics mode: Falling through air
	 * Handles gravity, air control, and landing detection
	 *
	 * @param DeltaTime Time step for physics update
	 * @param Iterations Number of sub-steps for collision resolution
	 */
	void PhysFalling(float DeltaTime, int32 Iterations);

	/**
	 * Physics mode: Flying (no gravity)
	 * Handles free 3D movement with acceleration/deceleration
	 *
	 * @param DeltaTime Time step for physics update
	 * @param Iterations Number of sub-steps for collision resolution
	 */
	void PhysFlying(float DeltaTime, int32 Iterations);

	/**
	 * Build and send movement data to server
	 * Called on autonomous proxy clients after performing predicted movement
	 *
	 * Implementation based on ServerMove RPC pattern at 015874A0
	 * @param TimeStamp Client timestamp for this move
	 * @param Acceleration Input acceleration for this move
	 * @param ClientLoc Client's predicted location after move
	 * @param CompressedFlags Compressed input flags (jump, crouch, etc)
	 * @param ClientRoll Client's roll rotation
	 * @param View Client's control rotation
	 * @param ClientMovementBase Movement base actor (for moving platforms)
	 * @param ClientBaseBoneName Bone name if base is skeletal mesh
	 * @param ClientMovementMode Movement mode (walking, falling, etc)
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerMove(
		float TimeStamp,
		FVector_NetQuantize10 Acceleration,
		FVector_NetQuantize100 ClientLoc,
		uint8 CompressedMoveFlags,
		uint8 ClientRoll,
		uint32 View,
		UPrimitiveComponent* ClientMovementBase,
		FName ClientBaseBoneName,
		uint8 ClientMovementMode
	);

	/**
	 * Dual move optimization - sends two moves in one RPC
	 * Used when moves can be combined to reduce bandwidth
	 *
	 * @param TimeStamp0 Timestamp for older move
	 * @param Acceleration0 Acceleration for older move
	 * @param PendingFlags Flags for older move
	 * @param View0 View for older move
	 * @param TimeStamp Timestamp for newer move
	 * @param Acceleration Acceleration for newer move
	 * @param ClientLoc Location after newer move
	 * @param NewFlags Flags for newer move
	 * @param ClientRoll Roll for newer move
	 * @param View View for newer move
	 * @param ClientMovementBase Movement base
	 * @param ClientBaseBoneName Base bone name
	 * @param ClientMovementMode Movement mode
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerMoveDual(
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
		uint8 ClientMovementMode
	);

	/**
	 * Old move RPC - for moves that are very old (high ping scenarios)
	 *
	 * @param OldTimeStamp Old timestamp
	 * @param OldAccel Old acceleration
	 * @param OldMoveFlags Old move flags
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerMoveOld(
		float OldTimeStamp,
		FVector_NetQuantize10 OldAccel,
		uint8 OldMoveFlags
	);

	/**
	 * Server correction: Client position differs from server simulation
	 * Client must rewind to this position and replay subsequent moves
	 *
	 * Implementation based on ClientAdjustPosition at 0156D520
	 * @param TimeStamp Server timestamp for correction
	 * @param NewLoc Corrected location
	 * @param NewVel Corrected velocity
	 * @param NewBase Corrected movement base
	 * @param NewBaseBoneName Corrected base bone name
	 * @param bHasBase Whether a base exists
	 * @param bBaseRelativePosition Whether position is relative to base
	 * @param ServerMovementMode Server movement mode
	 */
	UFUNCTION(Client, Reliable)
	void ClientAdjustPosition(
		float TimeStamp,
		FVector NewLoc,
		FVector NewVel,
		UPrimitiveComponent* NewBase,
		FName NewBaseBoneName,
		bool bHasBase,
		bool bBaseRelativePosition,
		uint8 ServerMovementMode
	);

	/**
	 * Server acknowledgement: Client move was correct, no correction needed
	 * Allows client to free old saved moves from buffer
	 *
	 * @param TimeStamp Acknowledged timestamp
	 * @param NewLoc Confirmed location
	 * @param NewVel Confirmed velocity
	 * @param ServerMovementMode Confirmed movement mode
	 */
	UFUNCTION(Client, Reliable)
	void ClientAckGoodMove(
		float TimeStamp,
		FVector NewLoc,
		FVector NewVel,
		uint8 ServerMovementMode
	);

	/**
	 * Get current timestamp for client moves
	 * Increments with each movement update
	 */
	float GetCurrentTimeStamp() const;

	/**
	 * Find saved move by timestamp
	 * Used for replaying moves after server correction
	 *
	 * @param TimeStamp Timestamp to search for
	 * @return Index of move in saved move array, or -1 if not found
	 */
	int32 FindSavedMoveByTimeStamp(float TimeStamp) const;

	/**
	 * Replay saved moves after server correction
	 * Reconstructs predicted state from corrected position
	 *
	 * @param StartMoveIndex Index of first move to replay
	 */
	void ReplaySavedMoves(int32 StartMoveIndex);

	/**
	 * Compress input flags for network transmission
	 * Packs multiple boolean flags into single byte
	 */
	uint8 CompressInputFlags() const;

	/**
	 * Decompress input flags received from network
	 * Unpacks boolean flags from byte
	 */
	void DecompressInputFlags(uint8 Flags);

	/**
	 * Calculate velocity for current movement mode
	 * Applies acceleration, friction, and constraints
	 *
	 * @param DeltaTime Time step for calculation
	 */
	void CalcVelocityImpl(float DeltaTime);

	/**
	 * Smooth client position correction for simulated proxies
	 * Uses mesh offset technique to avoid visual "pop"
	 *
	 * Implementation based on decompiled smoothing logic
	 * @param DeltaTime Time step for smoothing interpolation
	 */
	void SmoothClientPosition(float DeltaTime);

	/**
	 * Apply network correction with distance-based smoothing
	 * Decides whether to snap, smooth, or teleport based on error magnitude
	 *
	 * @param NewLocation Server-corrected location
	 * @param NewRotation Server-corrected rotation
	 * @param NewVelocity Server-corrected velocity
	 */
	void ApplyNetworkCorrection(const FVector& NewLocation, const FRotator& NewRotation, const FVector& NewVelocity);

	/**
	 * Update mesh relative transform for visual smoothing
	 * Allows capsule to be at correct physics position while mesh smoothly transitions
	 */
	void UpdateMeshOffsetSmoothing(float DeltaTime);

	/**
	 * Physics mode: NavWalking on navigation mesh
	 * Handles AI pathfinding movement on navmesh surfaces
	 *
	 * @param DeltaTime Time step for physics update
	 * @param Iterations Number of sub-steps for collision resolution
	 */
	void PhysNavWalking(float DeltaTime, int32 Iterations);

	/**
	 * Check if character can enter NavWalking mode
	 * Requires valid navmesh under character
	 */
	bool CanStartNavWalking() const;

	/**
	 * Project character location onto NavMesh
	 * @param Point World space point to project
	 * @param OutLocation Output projected location
	 * @return true if projection succeeded
	 */
	bool ProjectPointToNavMesh(const FVector& Point, struct FNavLocation& OutLocation) const;

	/**
	 * Get cached NavMesh for this world
	 * @return RecastNavMesh actor, or nullptr if not available
	 */
	class ARecastNavMesh* GetNavMesh() const;

	/**
	 * Apply root motion transform to movement
	 * Integrates animation-driven movement with physics
	 *
	 * @param RootMotionTransform Transform from root motion this frame
	 * @param DeltaTime Time step for integration
	 */
	void ApplyRootMotionToVelocity(const FTransform& RootMotionTransform, float DeltaTime);

	/**
	 * Get current Fortnite movement style
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Movement")
	EFortMovementStyle GetFortMovementStyle() const { return CurrentMovementStyle; }

	/**
	 * Set Fortnite movement style
	 */
	UFUNCTION(BlueprintCallable, Category = "Fort|Movement")
	void SetFortMovementStyle(EFortMovementStyle NewStyle);

protected:
	/** Current client timestamp - increments each movement update */
	UPROPERTY()
	float CurrentTimeStamp;

	/** Last timestamp acknowledged by server */
	UPROPERTY()
	float LastAckedTimeStamp;

	/** Last server move timestamp - tracks most recent move processed */
	UPROPERTY()
	float LastServerMoveTimeStamp;

	/** Maximum time delta allowed for a single move (prevents exploits) */
	UPROPERTY()
	float MaxMoveDeltaTime;

	/** Network smoothing for position corrections */
	UPROPERTY()
	float NetworkSimulatedSmoothLocationTime;

	/** Network smoothing for rotation corrections */
	UPROPERTY()
	float NetworkSimulatedSmoothRotationTime;

	/** Position error threshold before applying correction */
	UPROPERTY()
	float NetworkMaxSmoothUpdateDistance;

	/** Distance below which we snap instead of smooth (instant correction) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|NetworkSmoothing")
	float NetworkNoSmoothUpdateDistance;

	/** Distance above which we teleport instead of smooth */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|NetworkSmoothing")
	float TeleportDistanceThreshold;

	/** Rotation error threshold above which we teleport */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|NetworkSmoothing")
	float TeleportRotationThreshold;

	/** Network smoothing mode for simulated proxies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|NetworkSmoothing")
	EFortNetworkSmoothingMode NetworkSmoothingMode;

	/** Whether to enable mesh offset smoothing for visual corrections */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|NetworkSmoothing")
	bool bEnableMeshOffsetSmoothing;

	/** Current network smoothing state */
	FNetworkSmoothingState SmoothingState;

	/** Current Fortnite-specific movement style */
	UPROPERTY(BlueprintReadOnly, Category = "Fort|Movement")
	EFortMovementStyle CurrentMovementStyle;

	/** Sprint speed multiplier when in Sprinting style */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|Movement")
	float SprintSpeedMultiplier;

	/** Whether root motion is currently being applied */
	UPROPERTY()
	bool bHasAnimRootMotion;

	/** Accumulated root motion transform for current frame */
	FTransform AnimRootMotionTransform;

	/** Root motion translation scale (for tuning) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|RootMotion")
	float AnimRootMotionTranslationScale;

	/** Whether to enable NavWalking mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|NavWalking")
	bool bEnableNavWalking;

	/** Nav walking search height scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fort|NavWalking")
	float NavWalkingSearchHeightScale;

	/** Timestamp before current one (for dual move optimization) */
	float PendingMoveTimeStamp;

	/** Cached acceleration from previous frame */
	FVector PendingAcceleration;

	/** Whether we have a pending move to combine */
	bool bHasPendingMove;

	/** Client prediction data */
	mutable TSharedPtr<FNetworkPredictionData_Client_Fort> ClientPredictionData;

	/** Cached NavMesh reference for queries */
	mutable TWeakObjectPtr<ARecastNavMesh> CachedNavMesh;

private:
	/**
	 * Execute movement on server authority
	 * Validates client input and checks for position mismatch
	 *
	 * @param TimeStamp Client timestamp
	 * @param DeltaTime Time delta for move
	 * @param Acceleration Client input acceleration
	 * @param ClientLocation Client's predicted location
	 */
	void ServerMove_Implementation(
		float TimeStamp,
		FVector_NetQuantize10 Acceleration,
		FVector_NetQuantize100 ClientLoc,
		uint8 CompressedMoveFlags,
		uint8 ClientRoll,
		uint32 View,
		UPrimitiveComponent* ClientMovementBase,
		FName ClientBaseBoneName,
		uint8 ClientMovementMode
	);

	/**
	 * Validate server move request
	 * Prevents cheating and exploits
	 */
	bool ServerMove_Validate(
		float TimeStamp,
		FVector_NetQuantize10 Acceleration,
		FVector_NetQuantize100 ClientLoc,
		uint8 CompressedMoveFlags,
		uint8 ClientRoll,
		uint32 View,
		UPrimitiveComponent* ClientMovementBase,
		FName ClientBaseBoneName,
		uint8 ClientMovementMode
	);

	void ServerMoveDual_Implementation(
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
		uint8 ClientMovementMode
	);

	bool ServerMoveDual_Validate(
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
		uint8 ClientMovementMode
	);

	void ServerMoveOld_Implementation(
		float OldTimeStamp,
		FVector_NetQuantize10 OldAccel,
		uint8 OldMoveFlags
	);

	bool ServerMoveOld_Validate(
		float OldTimeStamp,
		FVector_NetQuantize10 OldAccel,
		uint8 OldMoveFlags
	);

	/**
	 * Handle client position correction from server
	 * Rewinds state and replays moves
	 */
	void ClientAdjustPosition_Implementation(
		float TimeStamp,
		FVector NewLoc,
		FVector NewVel,
		UPrimitiveComponent* NewBase,
		FName NewBaseBoneName,
		bool bHasBase,
		bool bBaseRelativePosition,
		uint8 ServerMovementMode
	);

	void ClientAckGoodMove_Implementation(
		float TimeStamp,
		FVector NewLoc,
		FVector NewVel,
		uint8 ServerMovementMode
	);
};
