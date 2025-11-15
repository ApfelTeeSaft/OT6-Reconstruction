// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/NetConnection.h"
#include "RebuiltNetConnection.generated.h"

/**
 * Net connection class reconstructed
 */
UCLASS(transient, config=Engine)
class NETWORKINGREBUILD_API URebuiltNetConnection : public UNetConnection
{
	GENERATED_BODY()

public:
	URebuiltNetConnection(const FObjectInitializer& ObjectInitializer);

	//~ Begin UNetConnection Interface

	/**
	 * Initialize connection on the remote (server) side
	 */
	virtual void InitRemoteConnection(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, const class FInternetAddr& InRemoteAddr, EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;

	/**
	 * Initialize connection on the local (client) side
	 */
	virtual void InitLocalConnection(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;

	/**
	 * Get remote address as string
	 */
	virtual FString RemoteAddressToString() override;

	/**
	 * Send a raw bunch (unreliable message bundle)
	 * Debug string: "UNetConnection::SendRawBunch. ChIndex: %d. Bits: %d. PacketId: %d"
	 */
	virtual void SendRawBunch(FOutBunch& Bunch, bool InAllowMerge);

	/**
	 * Connection tick - process queues and timeouts
	 * Debug strings:
	 * - "UNetConnection::Tick: Very long time between ticks. DeltaTime: %2.2f, Realtime: %2.2f %s"
	 * - "UNetConnection::Tick: Connection TIMED OUT. Closing connection. Elapsed: %2.2f, Real: %2.2f, Good: %2.2f, DriverTime: %2.2f, Threshold: %2.2f, %s"
	 */
	virtual void Tick(float DeltaSeconds) override;

	/**
	 * Low-level send function
	 */
	virtual void LowLevelSend(void* Data, int32 CountBytes, int32 CountBits) override;

	/**
	 * Receive and process a raw bunch
	 */
	virtual void ReceivedRawBunch(FInBunch& Bunch, bool& bOutHasPacketInfoToTrack) override;

	/**
	 * Receive a packet from network
	 */
	virtual void ReceivedPacket(FBitReader& Reader) override;

	/**
	 * Flush pending outgoing data
	 */
	virtual void FlushNet(bool bIgnoreSimulation = false) override;

	//~ End UNetConnection Interface

	/**
	 * Validate send buffer before transmission
	 * Debug string: "UNetConnection::ValidateSendBuffer: Out.IsError() == true. NumBits: %i, NumBytes: %i, MaxBits: %i"
	 */
	bool ValidateSendBuffer();

protected:
	/**
	 * Handle packet loss detection and recovery
	 */
	void HandlePacketLoss(int32 Sequence);

	/**
	 * Send initial handshake data
	 */
	void SendHandshake();

	/**
	 * Process received bunch on a specific channel
	 */
	void DispatchBunchToChannel(FInBunch& Bunch);

private:
	/** Custom handshake flag - specific to this game */
	bool bIsCustomHandshake;

	/** Last tick time for delta calculation */
	double LastTickTime;

	/** Time of last successful receive */
	double LastReceiveTime;

	/** Connection timeout threshold */
	float TimeoutThreshold;
};


/**
 * IP-based NetConnection implementation
 */
UCLASS(transient, config=Engine)
class NETWORKINGREBUILD_API URebuiltIpConnection : public URebuiltNetConnection
{
	GENERATED_BODY()

public:
	URebuiltIpConnection(const FObjectInitializer& ObjectInitializer);

	//~ Begin UNetConnection Interface
	virtual void LowLevelSend(void* Data, int32 CountBytes, int32 CountBits) override;
	virtual FString RemoteAddressToString() override;
	//~ End UNetConnection Interface

protected:
	/** Socket for this connection */
	class FSocket* Socket;

	/** Remote internet address */
	TSharedPtr<class FInternetAddr> RemoteAddr;
};
