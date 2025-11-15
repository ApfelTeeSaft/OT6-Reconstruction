// Copyright Epic Games, Inc. All Rights Reserved.
// Implementation file for RebuiltNetConnection

#include "NetworkingRebuild.h"
#include "RebuiltNetConnection.h"
#include "RebuiltChannels.h"
#include "Engine/ActorChannel.h"
#include "Engine/PackageMapClient.h"
#include "Net/DataReplication.h"
#include "Sockets.h"
#include "SocketSubsystem.h"

URebuiltNetConnection::URebuiltNetConnection(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bIsCustomHandshake(true)
	, LastTickTime(0.0)
	, LastReceiveTime(0.0)
	, TimeoutThreshold(30.0f)
{
}

void URebuiltNetConnection::InitRemoteConnection(UNetDriver* InDriver, FSocket* InSocket, const FURL& InURL, const FInternetAddr& InRemoteAddr, EConnectionState InState, int32 InMaxPacket, int32 InPacketOverhead)
{

	Super::InitRemoteConnection(InDriver, InSocket, InURL, InRemoteAddr, InState, InMaxPacket, InPacketOverhead);

	// Custom handshake initialization
	if (bIsCustomHandshake)
	{
		// Game-specific handshake logic
		UE_LOG(LogNet, Log, TEXT("InitRemoteConnection: Initializing custom handshake for %s"), *RemoteAddressToString());
	}

	LastReceiveTime = FPlatformTime::Seconds();
	LastTickTime = LastReceiveTime;
}

void URebuiltNetConnection::InitLocalConnection(UNetDriver* InDriver, FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket, int32 InPacketOverhead)
{

	Super::InitLocalConnection(InDriver, InSocket, InURL, InState, InMaxPacket, InPacketOverhead);

	// Custom handshake for local connection
	if (bIsCustomHandshake)
	{
		SendHandshake();
	}

	LastReceiveTime = FPlatformTime::Seconds();
	LastTickTime = LastReceiveTime;
}

FString URebuiltNetConnection::RemoteAddressToString()
{

	return Super::RemoteAddressToString();
}

void URebuiltNetConnection::SendRawBunch(FOutBunch& Bunch, bool InAllowMerge)
{

	// Validate bunch before sending
	if (Bunch.IsError())
	{
		UE_LOG(LogNetTraffic, Warning, TEXT("SendRawBunch: Bunch has error, ChIndex: %d"), Bunch.ChIndex);
		return;
	}

	// Log the send operation with details matching decompiled debug string
	UE_LOG(LogNetTraffic, VeryVerbose, TEXT("UNetConnection::SendRawBunch. ChIndex: %d. Bits: %d. PacketId: %d"),
		Bunch.ChIndex,
		Bunch.GetNumBits(),
		OutPacketId);

	// Validate send buffer
	if (!ValidateSendBuffer())
	{
		UE_LOG(LogNetTraffic, Error, TEXT("SendRawBunch: Send buffer validation failed"));
		return;
	}

	// Call parent implementation
	Super::SendRawBunch(Bunch, InAllowMerge);
}

void URebuiltNetConnection::Tick(float DeltaSeconds)
{

	const double CurrentTime = FPlatformTime::Seconds();
	const double RealDeltaTime = CurrentTime - LastTickTime;

	// Check for very long time between ticks
	// From debug string: "UNetConnection::Tick: Very long time between ticks. DeltaTime: %2.2f, Realtime: %2.2f %s"
	if (RealDeltaTime > 1.0)
	{
		UE_LOG(LogNet, Warning, TEXT("UNetConnection::Tick: Very long time between ticks. DeltaTime: %2.2f, Realtime: %2.2f %s"),
			DeltaSeconds,
			RealDeltaTime,
			*Describe());
	}

	LastTickTime = CurrentTime;

	// Connection timeout check
	// From debug string: "UNetConnection::Tick: Connection TIMED OUT. Closing connection. Elapsed: %2.2f, Real: %2.2f, Good: %2.2f, DriverTime: %2.2f, Threshold: %2.2f, %s"
	const double TimeSinceLastReceive = CurrentTime - LastReceiveTime;

	if (TimeSinceLastReceive > TimeoutThreshold)
	{
		const double DriverTime = Driver ? Driver->Time : 0.0;

		UE_LOG(LogNet, Warning, TEXT("UNetConnection::Tick: Connection TIMED OUT. Closing connection. Elapsed: %2.2f, Real: %2.2f, Good: %2.2f, DriverTime: %2.2f, Threshold: %2.2f, %s"),
			TimeSinceLastReceive,
			RealDeltaTime,
			LastReceiveRealtime,
			DriverTime,
			TimeoutThreshold,
			*Describe());

		Close();
		return;
	}

	Super::Tick(DeltaSeconds);
}

void URebuiltNetConnection::LowLevelSend(void* Data, int32 CountBytes, int32 CountBits)
{

	if (!Data || CountBytes <= 0)
	{
		return;
	}

	// Validate buffer before send
	if (!ValidateSendBuffer())
	{
		UE_LOG(LogNetTraffic, Error, TEXT("LowLevelSend: Buffer validation failed"));
		return;
	}

	Super::LowLevelSend(Data, CountBytes, CountBits);
}

void URebuiltNetConnection::ReceivedRawBunch(FInBunch& Bunch, bool& bOutHasPacketInfoToTrack)
{

	// Update last receive time
	LastReceiveTime = FPlatformTime::Seconds();

	// Validate bunch
	if (Bunch.IsError())
	{
		UE_LOG(LogNetTraffic, Error, TEXT("ReceivedRawBunch: Received bunch with error on channel %d"), Bunch.ChIndex);
		return;
	}

	// Dispatch to channel
	DispatchBunchToChannel(Bunch);

	Super::ReceivedRawBunch(Bunch, bOutHasPacketInfoToTrack);
}

void URebuiltNetConnection::ReceivedPacket(FBitReader& Reader)
{

	// Update last receive time
	LastReceiveTime = FPlatformTime::Seconds();

	// Process packet header
	if (Reader.IsError())
	{
		UE_LOG(LogNetTraffic, Error, TEXT("ReceivedPacket: Packet reader has error"));
		return;
	}

	// Log receive for debugging
	UE_LOG(LogNetTraffic, VeryVerbose, TEXT("%6.3f: Received %i"), Driver ? Driver->Time : 0.0f, Reader.GetNumBits());

	Super::ReceivedPacket(Reader);
}

void URebuiltNetConnection::FlushNet(bool bIgnoreSimulation)
{

	// Validate before flush
	if (!ValidateSendBuffer())
	{
		UE_LOG(LogNetTraffic, Warning, TEXT("FlushNet: Buffer validation failed, skipping flush"));
		return;
	}

	Super::FlushNet(bIgnoreSimulation);
}

bool URebuiltNetConnection::ValidateSendBuffer()
{

	// Check if send buffer has error
	if (SendBuffer.IsError())
	{
		const int32 NumBits = SendBuffer.GetNumBits();
		const int32 NumBytes = SendBuffer.GetNumBytes();
		const int32 MaxBits = SendBuffer.GetMaxBits();

		UE_LOG(LogNetTraffic, Error, TEXT("UNetConnection::ValidateSendBuffer: Out.IsError() == true. NumBits: %i, NumBytes: %i, MaxBits: %i"),
			NumBits,
			NumBytes,
			MaxBits);

		return false;
	}

	return true;
}

void URebuiltNetConnection::HandlePacketLoss(int32 Sequence)
{

	UE_LOG(LogNetTraffic, Verbose, TEXT("HandlePacketLoss: Detected packet loss for sequence %d"), Sequence);

	// Packet loss handling logic
	// - Resend reliable bunches
	// - Update statistics
	// - Trigger congestion control
}

void URebuiltNetConnection::SendHandshake()
{

	if (!bIsCustomHandshake)
	{
		return;
	}

	// Send custom handshake packet
	// This would include:
	// - Protocol version
	// - Game version
	// - Authentication tokens
	// - Client/server capabilities

	UE_LOG(LogNet, Log, TEXT("SendHandshake: Sending custom handshake"));
}

void URebuiltNetConnection::DispatchBunchToChannel(FInBunch& Bunch)
{

	if (Bunch.ChIndex >= Channels.Num() || Bunch.ChIndex < 0)
	{
		UE_LOG(LogNetTraffic, Error, TEXT("DispatchBunchToChannel: Invalid channel index %d"), Bunch.ChIndex);
		return;
	}

	UChannel* Channel = Channels[Bunch.ChIndex];
	if (!Channel)
	{
		// Create channel if needed
		if (Bunch.bOpen)
		{
			// Channel creation logic
			UE_LOG(LogNetTraffic, Log, TEXT("DispatchBunchToChannel: Creating new channel %d"), Bunch.ChIndex);
		}
		else
		{
			UE_LOG(LogNetTraffic, Warning, TEXT("DispatchBunchToChannel: No channel for index %d"), Bunch.ChIndex);
		}
		return;
	}

	// Dispatch bunch to channel
	Channel->ReceivedBunch(Bunch);
}


//-----------------------------------------------------------------------------
// URebuiltIpConnection implementation
//-----------------------------------------------------------------------------

URebuiltIpConnection::URebuiltIpConnection(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Socket(nullptr)
{
}

void URebuiltIpConnection::LowLevelSend(void* Data, int32 CountBytes, int32 CountBits)
{
	// IP-specific low-level send

	if (!Socket || !RemoteAddr.IsValid() || !Data || CountBytes <= 0)
	{
		return;
	}

	int32 BytesSent = 0;
	if (!Socket->SendTo((uint8*)Data, CountBytes, BytesSent, *RemoteAddr))
	{
		UE_LOG(LogNetTraffic, Warning, TEXT("IpConnection: Failed to send %d bytes"), CountBytes);
	}
	else
	{
		UE_LOG(LogNetTraffic, VeryVerbose, TEXT("IpConnection: Sent %d bytes to %s"), BytesSent, *RemoteAddressToString());
	}
}

FString URebuiltIpConnection::RemoteAddressToString()
{
	if (RemoteAddr.IsValid())
	{
		return RemoteAddr->ToString(true);
	}

	return TEXT("Invalid");
}
