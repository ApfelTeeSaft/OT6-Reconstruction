// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Channel.h"
#include "Engine/ActorChannel.h"
#include "Engine/ControlChannel.h"
#include "Engine/VoiceChannel.h"
#include "RebuiltChannels.generated.h"

UCLASS(transient)
class NETWORKINGREBUILD_API URebuiltChannel : public UChannel
{
	GENERATED_BODY()

public:
	URebuiltChannel(const FObjectInitializer& ObjectInitializer);

	//~ Begin UChannel Interface

	/**
	 * Send a bunch (message bundle) on this channel
	 * Confidence: MEDIUM - Standard bunch transmission
	 */
	virtual FPacketIdRange SendBunch(FOutBunch* Bunch, bool Merge) override;

	/**
	 * Process received bunch
	 * Confidence: HIGH - Core receive logic with extensive references
	 */
	virtual void ReceivedBunch(FInBunch& Bunch) override;

	/**
	 * Tick this channel
	 */
	virtual void Tick() override;

	//~ End UChannel Interface

protected:
	/**
	 * Validate bunch data before processing
	 */
	bool ValidateBunch(FInBunch& Bunch);
};


/**
 * Actor channel - handles actor replication
 */
UCLASS(transient)
class NETWORKINGREBUILD_API URebuiltActorChannel : public UActorChannel
{
	GENERATED_BODY()

public:
	URebuiltActorChannel(const FObjectInitializer& ObjectInitializer);

	//~ Begin UActorChannel Interface

	/**
	 * Replicate actor and its properties to remote connection
	 */
	virtual int64 ReplicateActor() override;

	/**
	 * Process received bunch for actor updates
	 */
	virtual void ReceivedBunch(FInBunch& Bunch) override;

	/**
	 * Clean up actor channel
	 */
	virtual void CleanUp() override;

	//~ End UActorChannel Interface

protected:
	/**
	 * Replicate individual properties
	 */
	bool ReplicateProperties(FOutBunch& Bunch);

	/**
	 * Compare properties for changes (dirty checking)
	 */
	bool CompareProperties(class FRepLayout* RepLayout, class FRepState* RepState);

	/**
	 * Process received property data
	 */
	void ReceiveProperties(FInBunch& Bunch);

private:
	/** Custom replication state - game-specific */
	TSharedPtr<class FRebuiltRepState> CustomRepState;
};


/**
 * Control channel - handles connection control messages
 */
UCLASS(transient)
class NETWORKINGREBUILD_API URebuiltControlChannel : public UControlChannel
{
	GENERATED_BODY()

public:
	URebuiltControlChannel(const FObjectInitializer& ObjectInitializer);

	//~ Begin UControlChannel Interface
	virtual void ReceivedBunch(FInBunch& Bunch) override;
	//~ End UControlChannel Interface

protected:
	/**
	 * Process control messages (join, welcome, etc.)
	 */
	void ProcessControlMessage(uint8 MessageType, FInBunch& Bunch);
};


/**
 * Voice channel - handles voice data transmission
 */
UCLASS(transient)
class NETWORKINGREBUILD_API URebuiltVoiceChannel : public UVoiceChannel
{
	GENERATED_BODY()

public:
	URebuiltVoiceChannel(const FObjectInitializer& ObjectInitializer);

	//~ Begin UVoiceChannel Interface
	virtual void ReceivedBunch(FInBunch& Bunch) override;
	virtual void Tick() override;
	//~ End UVoiceChannel Interface

protected:
	/**
	 * Process voice data packets
	 */
	void ProcessVoiceData(FInBunch& Bunch);

private:
	/** Voice data buffer */
	TArray<uint8> VoiceBuffer;
};


/**
 * Replication state for custom property tracking
 */
struct FRebuiltRepState
{
	/** History of sent properties */
	TArray<uint16> HistoryStart;

	/** History of received properties */
	TArray<uint16> HistoryEnd;

	/** Shared properties across clients */
	TSharedPtr<struct FRepChangedPropertyTracker> RepChangedPropertyTracker;

	/** Number of times this state has been replicated */
	int32 NumReplicationFrames;

	/** Custom game-specific replication flags */
	uint32 CustomFlags;
};
