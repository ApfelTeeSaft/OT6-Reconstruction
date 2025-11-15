// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/NetDriver.h"
#include "RebuiltNetDriver.generated.h"

/**
 * Base NetDriver class reconstructed from IDA pseudo-C dumps
 */
UCLASS(transient, config=Engine)
class NETWORKINGREBUILD_API URebuiltNetDriver : public UNetDriver
{
	GENERATED_BODY()

public:
	URebuiltNetDriver(const FObjectInitializer& ObjectInitializer);

	//~ Begin UNetDriver Interface

	/**
	 * Initialize the network driver for listening (server mode)
	 */
	virtual bool InitListen(FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error) override;

	/**
	 * Initialize the network driver for connecting (client mode)
	 */
	virtual bool InitConnect(FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error) override;

	/**
	 * Process incoming packets and dispatch to connections
	 */
	virtual void TickDispatch(float DeltaTime) override;

	/**
	 * Flush outgoing packet buffers to network
	 */
	virtual void TickFlush(float DeltaTime) override;

	/**
	 * Process remote function calls (RPCs)
	 */
	virtual bool ProcessRemoteFunction(class AActor* Actor, UFunction* Function, void* Parameters, FOutParmRec* OutParms, FFrame* Stack, class UObject* SubObject = nullptr) override;

	/**
	 * Shut down the network driver
	 */
	virtual void Shutdown() override;

	/**
	 * Low-level packet send function
	 */
	virtual void LowLevelSend(FString Address, void* Data, int32 CountBits) override;

	//~ End UNetDriver Interface

protected:
	/**
	 * Get the socket subsystem for this driver
	 */
	class ISocketSubsystem* GetSocketSubsystem();

private:
	/**
	 * Custom packet validation
	 */
	bool ValidatePacket(uint8* PacketData, int32 PacketSize);

	/**
	 * Handle connection handshake
	 */
	void ProcessHandshake(class UNetConnection* Connection, uint8* Data, int32 Size);
};


/**
 * IP-based NetDriver implementation
 */
UCLASS(transient, config=Engine)
class NETWORKINGREBUILD_API URebuiltIpNetDriver : public URebuiltNetDriver
{
	GENERATED_BODY()

public:
	URebuiltIpNetDriver(const FObjectInitializer& ObjectInitializer);

	//~ Begin UNetDriver Interface
	virtual bool InitListen(FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error) override;
	virtual bool InitConnect(FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error) override;
	virtual void TickDispatch(float DeltaTime) override;
	virtual void LowLevelSend(FString Address, void* Data, int32 CountBits) override;
	//~ End UNetDriver Interface

protected:
	/** The socket used for network communication */
	class FSocket* Socket;

	/** Receive buffer for incoming packets */
	TArray<uint8> RecvBuffer;

	/** Maximum packet size */
	int32 MaxPacketSize;
};
