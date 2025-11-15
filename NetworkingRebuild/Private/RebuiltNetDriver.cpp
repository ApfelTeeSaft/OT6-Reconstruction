// Copyright Epic Games, Inc. All Rights Reserved.
// Implementation file for RebuiltNetDriver

#include "NetworkingRebuild.h"
#include "RebuiltNetDriver.h"
#include "RebuiltNetConnection.h"
#include "SocketSubsystem.h"
#include "Sockets.h"
#include "IPAddress.h"
#include "Engine/ActorChannel.h"
#include "Engine/PackageMapClient.h"
#include "Net/NetworkProfiler.h"
#include "Net/DataReplication.h"


URebuiltNetDriver::URebuiltNetDriver(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Initialize based on standard UE 4.12 NetDriver defaults
	NetConnectionClassName = TEXT("/Script/NetworkingRebuild.RebuiltNetConnection");
	MaxInternetClientRate = 10000;
	MaxClientRate = 15000;
	ServerTravelPause = 4.0f;
}

bool URebuiltNetDriver::InitListen(FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error)
{

	if (!InitBase(true, InNotify, ListenURL, bReuseAddressAndPort, Error))
	{
		return false;
	}

	// Create socket for listening
	ISocketSubsystem* SocketSubsystem = GetSocketSubsystem();
	if (!SocketSubsystem)
	{
		Error = TEXT("Failed to get socket subsystem");
		return false;
	}

	// Bind to port
	int32 BindPort = ListenURL.Port;
	UE_LOG(LogNet, Log, TEXT("RebuiltNetDriver::InitListen: Binding to port %d"), BindPort);

	// Initialize connection listening
	InitConnectionClass();

	// Set up packet receive buffer
	const int32 RecvBufSize = 0x20000; // 128KB - standard UE4 size

	return true;
}

bool URebuiltNetDriver::InitConnect(FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error)
{

	if (!InitBase(false, InNotify, ConnectURL, false, Error))
	{
		return false;
	}

	ISocketSubsystem* SocketSubsystem = GetSocketSubsystem();
	if (!SocketSubsystem)
	{
		Error = TEXT("Failed to get socket subsystem");
		return false;
	}

	// Create server connection
	URebuiltNetConnection* Connection = NewObject<URebuiltNetConnection>(GetTransientPackage(), NetConnectionClass);
	if (!Connection)
	{
		Error = TEXT("Failed to create connection");
		return false;
	}

	// Resolve server address
	TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
	bool bIsValid = false;

	// Convert URL to address
	Addr->SetIp(*ConnectURL.Host, bIsValid);
	Addr->SetPort(ConnectURL.Port);

	if (!bIsValid)
	{
		Error = FString::Printf(TEXT("Invalid address: %s"), *ConnectURL.Host);
		return false;
	}

	UE_LOG(LogNet, Log, TEXT("RebuiltNetDriver::InitConnect: Connecting to %s:%d"), *ConnectURL.Host, ConnectURL.Port);

	// Initialize the connection
	// Note: Custom handshake may be required based on game protocol
	ServerConnection = Connection;

	return true;
}

void URebuiltNetDriver::TickDispatch(float DeltaTime)
{

	Super::TickDispatch(DeltaTime);

	// Process incoming packets
	// This would normally read from socket and dispatch to connections

	ISocketSubsystem* SocketSubsystem = GetSocketSubsystem();
	if (!SocketSubsystem)
	{
		return;
	}

	// Receive packets and dispatch to appropriate connections
	for (int32 i = 0; i < ClientConnections.Num(); i++)
	{
		UNetConnection* Connection = ClientConnections[i];
		if (Connection && Connection->State != USOCK_Closed)
		{
			// Each connection processes its own packets in Tick()
		}
	}
}

void URebuiltNetDriver::TickFlush(float DeltaTime)
{

	Super::TickFlush(DeltaTime);

	// Flush outgoing packets for all connections
	for (int32 i = 0; i < ClientConnections.Num(); i++)
	{
		UNetConnection* Connection = ClientConnections[i];
		if (Connection)
		{
			Connection->FlushNet();
		}
	}

	if (ServerConnection)
	{
		ServerConnection->FlushNet();
	}
}

bool URebuiltNetDriver::ProcessRemoteFunction(AActor* Actor, UFunction* Function, void* Parameters, FOutParmRec* OutParms, FFrame* Stack, UObject* SubObject)
{
	// RPC patterns found in dumps:
	// - Server_*: 28 references (client -> server RPCs)
	// - Client_*: 59 references (server -> client RPCs)
	// - NetMulticast_*: 27 references (multicast RPCs)

	bool bProcessed = false;

	// Determine RPC type from function flags
	const bool bIsServer = (Function->FunctionFlags & FUNC_Net) && (Function->FunctionFlags & FUNC_NetServer);
	const bool bIsClient = (Function->FunctionFlags & FUNC_Net) && (Function->FunctionFlags & FUNC_NetClient);
	const bool bIsMulticast = (Function->FunctionFlags & FUNC_Net) && (Function->FunctionFlags & FUNC_NetMulticast);

	if (bIsServer)
	{
		// Server RPC - send from client to server
		bProcessed = Super::ProcessRemoteFunction(Actor, Function, Parameters, OutParms, Stack, SubObject);
	}
	else if (bIsClient)
	{
		// Client RPC - send from server to specific client
		bProcessed = Super::ProcessRemoteFunction(Actor, Function, Parameters, OutParms, Stack, SubObject);
	}
	else if (bIsMulticast)
	{
		// Multicast RPC - send from server to all clients
		bProcessed = Super::ProcessRemoteFunction(Actor, Function, Parameters, OutParms, Stack, SubObject);
	}

	return bProcessed;
}

void URebuiltNetDriver::Shutdown()
{

	UE_LOG(LogNet, Log, TEXT("RebuiltNetDriver::Shutdown"));

	// Close all client connections
	for (int32 i = ClientConnections.Num() - 1; i >= 0; i--)
	{
		if (ClientConnections[i])
		{
			ClientConnections[i]->Close();
		}
	}

	// Close server connection
	if (ServerConnection)
	{
		ServerConnection->Close();
		ServerConnection = nullptr;
	}

	Super::Shutdown();
}

void URebuiltNetDriver::LowLevelSend(FString Address, void* Data, int32 CountBits)
{

	// This would normally send raw packet data to the network

	Super::LowLevelSend(Address, Data, CountBits);
}

ISocketSubsystem* URebuiltNetDriver::GetSocketSubsystem()
{

	return ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
}

bool URebuiltNetDriver::ValidatePacket(uint8* PacketData, int32 PacketSize)
{

	if (!PacketData || PacketSize <= 0)
	{
		return false;
	}

	// Basic packet validation
	// Game-specific validation may include:
	// - Magic number check
	// - Protocol version check
	// - Checksum validation
	// - Sequence number validation

	return true;
}

void URebuiltNetDriver::ProcessHandshake(UNetConnection* Connection, uint8* Data, int32 Size)
{
	// Custom handshake logic would go here

	if (!Connection || !Data)
	{
		return;
	}

	// Process connection handshake
	// This may include:
	// - Protocol version exchange
	// - Encryption key exchange
	// - Authentication tokens
	// - Game-specific initialization
}


//-----------------------------------------------------------------------------
// URebuiltIpNetDriver implementation
//-----------------------------------------------------------------------------

URebuiltIpNetDriver::URebuiltIpNetDriver(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Socket(nullptr)
	, MaxPacketSize(1024)
{

	NetConnectionClassName = TEXT("/Script/NetworkingRebuild.RebuiltIpConnection");
}

bool URebuiltIpNetDriver::InitListen(FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error)
{
	// IP-specific listen initialization
	if (!Super::InitListen(InNotify, ListenURL, bReuseAddressAndPort, Error))
	{
		return false;
	}

	ISocketSubsystem* SocketSubsystem = GetSocketSubsystem();
	if (!SocketSubsystem)
	{
		Error = TEXT("Failed to get socket subsystem");
		return false;
	}

	// Create UDP socket for listening
	Socket = SocketSubsystem->CreateSocket(NAME_DGram, TEXT("Unreal"), false);
	if (!Socket)
	{
		Error = TEXT("Failed to create socket");
		return false;
	}

	// Bind to address
	TSharedRef<FInternetAddr> BindAddr = SocketSubsystem->GetLocalBindAddr(*GLog);
	BindAddr->SetPort(ListenURL.Port);

	if (!Socket->Bind(*BindAddr))
	{
		Error = FString::Printf(TEXT("Failed to bind socket to port %d"), ListenURL.Port);
		return false;
	}

	// Set socket options
	int32 RecvBufSize = 0x100000; // 1MB
	int32 SendBufSize = 0x100000;
	Socket->SetReceiveBufferSize(RecvBufSize, RecvBufSize);
	Socket->SetSendBufferSize(SendBufSize, SendBufSize);

	Socket->SetNonBlocking(true);
	Socket->SetReuseAddr(bReuseAddressAndPort);

	UE_LOG(LogNet, Log, TEXT("IpNetDriver listening on port %d"), ListenURL.Port);

	return true;
}

bool URebuiltIpNetDriver::InitConnect(FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error)
{
	// IP-specific connect initialization
	if (!Super::InitConnect(InNotify, ConnectURL, Error))
	{
		return false;
	}

	ISocketSubsystem* SocketSubsystem = GetSocketSubsystem();
	if (!SocketSubsystem)
	{
		Error = TEXT("Failed to get socket subsystem");
		return false;
	}

	// Create UDP socket for client
	Socket = SocketSubsystem->CreateSocket(NAME_DGram, TEXT("Unreal"), false);
	if (!Socket)
	{
		Error = TEXT("Failed to create socket");
		return false;
	}

	// Bind to local address
	TSharedRef<FInternetAddr> LocalAddr = SocketSubsystem->GetLocalBindAddr(*GLog);
	LocalAddr->SetPort(0); // Use any available port

	if (!Socket->Bind(*LocalAddr))
	{
		Error = TEXT("Failed to bind socket");
		return false;
	}

	Socket->SetNonBlocking(true);

	UE_LOG(LogNet, Log, TEXT("IpNetDriver connecting to %s:%d"), *ConnectURL.Host, ConnectURL.Port);

	return true;
}

void URebuiltIpNetDriver::TickDispatch(float DeltaTime)
{
	// IP-specific packet dispatch
	Super::TickDispatch(DeltaTime);

	if (!Socket)
	{
		return;
	}

	// Receive packets from socket
	uint32 PendingDataSize = 0;
	while (Socket->HasPendingData(PendingDataSize) && PendingDataSize > 0)
	{
		RecvBuffer.SetNumUninitialized(FMath::Min(PendingDataSize, 65536u));

		TSharedRef<FInternetAddr> FromAddr = GetSocketSubsystem()->CreateInternetAddr();
		int32 BytesRead = 0;

		if (Socket->RecvFrom(RecvBuffer.GetData(), RecvBuffer.Num(), BytesRead, *FromAddr))
		{
			if (BytesRead > 0)
			{
				// Find or create connection for this address
				// Dispatch packet to appropriate connection
				// This would call Connection->ReceivedPacket()
			}
		}
	}
}

void URebuiltIpNetDriver::LowLevelSend(FString Address, void* Data, int32 CountBits)
{
	// IP-specific low-level send
	if (!Socket || !Data)
	{
		return;
	}

	// Convert address string to FInternetAddr
	TSharedRef<FInternetAddr> RemoteAddr = GetSocketSubsystem()->CreateInternetAddr();
	bool bIsValid = false;
	RemoteAddr->SetIp(*Address, bIsValid);

	if (bIsValid)
	{
		int32 CountBytes = FMath::DivideAndRoundUp(CountBits, 8);
		int32 BytesSent = 0;
		Socket->SendTo((uint8*)Data, CountBytes, BytesSent, *RemoteAddr);
	}
}
