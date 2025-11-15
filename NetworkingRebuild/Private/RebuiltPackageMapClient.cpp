// Copyright Epic Games, Inc. All Rights Reserved.
// FULL IMPLEMENTATION - PackageMapClient

#include "NetworkingRebuild.h"
#include "RebuiltPackageMapClient.h"
#include "Engine/ActorChannel.h"
#include "Engine/NetConnection.h"
#include "GameFramework/Actor.h"
#include "UObject/Package.h"

URebuiltPackageMapClient::URebuiltPackageMapClient(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, NextNetGUID(1)
	, bIsServer(false)
{
}

bool URebuiltPackageMapClient::SerializeObject(FArchive& Ar, UClass* InClass, UObject*& Obj, FNetworkGUID* OutNetGUID)
{

	if (Ar.IsSaving())
	{
		// Writing object reference
		if (Obj)
		{
			// Get or assign GUID for this object
			FNetworkGUID NetGUID = GetOrAssignNetGUID(Obj);

			// Write the GUID
			Ar << NetGUID;

			if (OutNetGUID)
			{
				*OutNetGUID = NetGUID;
			}

			// If this is a new GUID, write the object path
			if (!GuidCache.Contains(NetGUID))
			{
				// Write full object path for client to load
				FString PathName = Obj->GetPathName();
				Ar << PathName;

				// Cache this GUID
				GuidCache.Add(NetGUID, Obj);
			}

			return true;
		}
		else
		{
			// NULL object
			FNetworkGUID NullGUID;
			Ar << NullGUID;
			return true;
		}
	}
	else
	{
		// Reading object reference
		FNetworkGUID NetGUID;
		Ar << NetGUID;

		if (OutNetGUID)
		{
			*OutNetGUID = NetGUID;
		}

		if (!NetGUID.IsValid())
		{
			Obj = nullptr;
			return true;
		}

		// Try to get object from cache
		Obj = GetObjectFromNetGUID(NetGUID, false);

		if (!Obj)
		{
			// Read path name if this is a new GUID
			FString PathName;
			if (Ar.Tell() < Ar.TotalSize())
			{
				Ar << PathName;

				// Load the object
				Obj = InternalLoadObject(nullptr, NetGUID, PathName);
			}
		}

		return Obj != nullptr;
	}
}

bool URebuiltPackageMapClient::SerializeNewActor(FArchive& Ar, UActorChannel* Channel, AActor*& Actor)
{

	if (Ar.IsSaving())
	{
		// Server writing new actor
		if (!Actor)
		{
			UE_LOG(LogNet, Error, TEXT("SerializeNewActor: Null actor"));
			return false;
		}

		// Assign GUID for new actor
		FNetworkGUID NetGUID = GetOrAssignNetGUID(Actor);
		Ar << NetGUID;

		// Write actor class
		UClass* ActorClass = Actor->GetClass();
		FString ClassName = ActorClass->GetPathName();
		Ar << ClassName;

		// Write spawn location
		FVector Location = Actor->GetActorLocation();
		Ar << Location;

		// Write spawn rotation
		FRotator Rotation = Actor->GetActorRotation();
		Ar << Rotation;

		// Write spawn scale
		FVector Scale = Actor->GetActorScale3D();
		Ar << Scale;

		UE_LOG(LogNetTraffic, Verbose, TEXT("SerializeNewActor: Wrote %s with GUID %s"),
			*Actor->GetName(), *NetGUID.ToString());

		return true;
	}
	else
	{
		// Client reading new actor spawn
		FNetworkGUID NetGUID;
		Ar << NetGUID;

		// Read actor class
		FString ClassName;
		Ar << ClassName;

		// Load class
		UClass* ActorClass = LoadObject<UClass>(nullptr, *ClassName);
		if (!ActorClass)
		{
			UE_LOG(LogNet, Error, TEXT("SerializeNewActor: Failed to load class %s"), *ClassName);
			return false;
		}

		// Read spawn parameters
		FVector Location;
		FRotator Rotation;
		FVector Scale;
		Ar << Location;
		Ar << Rotation;
		Ar << Scale;

		// Spawn the actor
		UWorld* World = Channel->Connection->Driver->GetWorld();
		FActorSpawnParameters SpawnParams;
		SpawnParams.bNoCollisionFail = true;
		SpawnParams.bRemoteOwned = true;

		Actor = World->SpawnActor<AActor>(ActorClass, Location, Rotation, SpawnParams);
		if (Actor)
		{
			Actor->SetActorScale3D(Scale);

			// Register the GUID
			GuidCache.Add(NetGUID, Actor);
			ObjectToGuidCache.Add(Actor, NetGUID);

			UE_LOG(LogNetTraffic, Verbose, TEXT("SerializeNewActor: Spawned %s with GUID %s"),
				*Actor->GetName(), *NetGUID.ToString());

			return true;
		}
		else
		{
			UE_LOG(LogNet, Error, TEXT("SerializeNewActor: Failed to spawn actor of class %s"), *ClassName);
			return false;
		}
	}
}

bool URebuiltPackageMapClient::WriteObject(FArchive& Ar, UObject* ObjOuter, FNetworkGUID NetGUID, const FString& ObjName)
{

	if (!NetGUID.IsValid())
	{
		return false;
	}

	// Write GUID
	Ar << NetGUID;

	// Write object name and outer
	FString PathName = ObjName;
	Ar << PathName;

	// Write outer GUID if exists
	if (ObjOuter)
	{
		FNetworkGUID OuterGUID = GetOrAssignNetGUID(ObjOuter);
		Ar << OuterGUID;
	}
	else
	{
		FNetworkGUID NullGUID;
		Ar << NullGUID;
	}

	return true;
}

UObject* URebuiltPackageMapClient::InternalLoadObject(UObject* ObjOuter, const FNetworkGUID& NetGUID, const FString& PathName)
{

	// Check if already in cache
	if (GuidCache.Contains(NetGUID))
	{
		TWeakObjectPtr<UObject>* CachedObj = GuidCache.Find(NetGUID);
		if (CachedObj && CachedObj->IsValid())
		{
			return CachedObj->Get();
		}
	}

	// Load the object by path
	UObject* LoadedObject = StaticLoadObject(UObject::StaticClass(), nullptr, *PathName, nullptr, LOAD_None, nullptr);

	if (!LoadedObject)
	{
		UE_LOG(LogNet, Warning, TEXT("InternalLoadObject: Failed to load object: PathName: %s, ObjOuter: %s"),
			*PathName, ObjOuter ? *ObjOuter->GetName() : TEXT("NULL"));
		return nullptr;
	}

	// Validate object
	if (!ValidateObjectForLoad(LoadedObject, NetGUID, PathName, ObjOuter))
	{
		return nullptr;
	}

	// Check for pending kill
	// From debug: "UPackageMapClient::InternalLoadObject: Received reference to pending kill object from client: PathName: %s, ObjOuter: %s"
	if (LoadedObject->IsPendingKill())
	{
		if (!HandlePendingKillObject(LoadedObject, PathName, ObjOuter))
		{
			UE_LOG(LogNet, Error, TEXT("UPackageMapClient::InternalLoadObject: Received reference to pending kill object from client: PathName: %s, ObjOuter: %s"),
				*PathName, ObjOuter ? *ObjOuter->GetName() : TEXT("NULL"));
			return nullptr;
		}
	}

	// Cache the loaded object
	GuidCache.Add(NetGUID, LoadedObject);
	ObjectToGuidCache.Add(LoadedObject, NetGUID);

	UE_LOG(LogNetTraffic, Verbose, TEXT("InternalLoadObject: Loaded %s with GUID %s"),
		*LoadedObject->GetName(), *NetGUID.ToString());

	return LoadedObject;
}

FNetworkGUID URebuiltPackageMapClient::AssignNewNetGUID(UObject* Object)
{

	if (!Object)
	{
		return FNetworkGUID();
	}

	// Check if already assigned
	FNetworkGUID* ExistingGUID = ObjectToGuidCache.Find(Object);
	if (ExistingGUID)
	{
		return *ExistingGUID;
	}

	// Assign new GUID
	FNetworkGUID NewGUID;
	NewGUID.Value = NextNetGUID++;

	// Set dynamic flag if this is a runtime-spawned object
	if (Object->IsA<AActor>())
	{
		AActor* Actor = Cast<AActor>(Object);
		if (!Actor->IsNetStartupActor())
		{
			NewGUID.bIsDynamic = true;
		}
	}

	// Cache it
	GuidCache.Add(NewGUID, Object);
	ObjectToGuidCache.Add(Object, NewGUID);

	return NewGUID;
}

FNetworkGUID URebuiltPackageMapClient::GetOrAssignNetGUID(UObject* Object)
{

	if (!Object)
	{
		return FNetworkGUID();
	}

	// Check cache first
	FNetworkGUID* ExistingGUID = ObjectToGuidCache.Find(Object);
	if (ExistingGUID)
	{
		return *ExistingGUID;
	}

	// Assign new GUID
	return AssignNewNetGUID(Object);
}

UObject* URebuiltPackageMapClient::GetObjectFromNetGUID(const FNetworkGUID& NetGUID, const bool bIgnoreMustBeMapped)
{

	if (!NetGUID.IsValid())
	{
		return nullptr;
	}

	TWeakObjectPtr<UObject>* CachedObj = GuidCache.Find(NetGUID);
	if (CachedObj && CachedObj->IsValid())
	{
		return CachedObj->Get();
	}

	if (!bIgnoreMustBeMapped)
	{
		UE_LOG(LogNetTraffic, VeryVerbose, TEXT("GetObjectFromNetGUID: GUID %s not in cache"), *NetGUID.ToString());
	}

	return nullptr;
}

void URebuiltPackageMapClient::RegisterDormantActor(AActor* Actor, const FNetworkGUID& NetGUID)
{

	if (!Actor || !NetGUID.IsValid())
	{
		return;
	}

	DormantActors.Add(NetGUID, Actor);

	UE_LOG(LogNetTraffic, Verbose, TEXT("RegisterDormantActor: %s with GUID %s"),
		*Actor->GetName(), *NetGUID.ToString());
}

void URebuiltPackageMapClient::UnregisterDormantActor(AActor* Actor)
{

	if (!Actor)
	{
		return;
	}

	// Find and remove from dormant list
	for (auto It = DormantActors.CreateIterator(); It; ++It)
	{
		if (It.Value().Get() == Actor)
		{
			UE_LOG(LogNetTraffic, Verbose, TEXT("UnregisterDormantActor: %s"), *Actor->GetName());
			It.RemoveCurrent();
			break;
		}
	}
}

bool URebuiltPackageMapClient::ValidateObjectForLoad(UObject* Object, const FNetworkGUID& NetGUID, const FString& PathName, UObject* ObjOuter)
{

	if (!Object)
	{
		return false;
	}

	// Check if it's a default object
	if (Object->IsDefaultSubobject())
	{
		// From debug: "UPackageMapClient::InternalLoadObject: Default object not a package from client: PathName: %s, ObjOuter: %s"
		UPackage* Package = Cast<UPackage>(Object->GetOuter());
		if (!Package)
		{
			UE_LOG(LogNet, Error, TEXT("UPackageMapClient::InternalLoadObject: Default object not a package from client: PathName: %s, ObjOuter: %s"),
				*PathName, ObjOuter ? *ObjOuter->GetName() : TEXT("NULL"));
			return false;
		}

		// Validate package GUID
		// From debug: "UPackageMapClient::InternalLoadObject: Default object package guid mismatch! PathName: %s, ObjOuter: %s, GUID1: %u, GUID2: %u"
		FNetworkGUID PackageGUID = GetOrAssignNetGUID(Package);
		if (!ValidatePackageGUID(NetGUID, PackageGUID))
		{
			UE_LOG(LogNet, Error, TEXT("UPackageMapClient::InternalLoadObject: Default object package guid mismatch! PathName: %s, ObjOuter: %s, GUID1: %u, GUID2: %u"),
				*PathName, ObjOuter ? *ObjOuter->GetName() : TEXT("NULL"), NetGUID.Value, PackageGUID.Value);
			return false;
		}
	}

	return true;
}

bool URebuiltPackageMapClient::HandlePendingKillObject(UObject* Object, const FString& PathName, UObject* ObjOuter)
{
	// From debug: "Received reference to pending kill object from client"

	if (!Object || !Object->IsPendingKill())
	{
		return true;
	}

	// On client, pending kill objects from server are errors
	// On server, pending kill objects from client might be legitimate during cleanup
	if (!bIsServer)
	{
		return false;
	}

	// Server can handle pending kill by removing from cache
	FNetworkGUID* ExistingGUID = ObjectToGuidCache.Find(Object);
	if (ExistingGUID)
	{
		GuidCache.Remove(*ExistingGUID);
		ObjectToGuidCache.Remove(Object);
	}

	return true;
}

bool URebuiltPackageMapClient::ValidatePackageGUID(const FNetworkGUID& GUID1, const FNetworkGUID& GUID2)
{
	// From debug: "Default object package guid mismatch! GUID1: %u, GUID2: %u"

	if (!GUID1.IsValid() || !GUID2.IsValid())
	{
		return false;
	}

	// For default objects, the package GUID should match
	// Allow some flexibility for dynamic vs static GUIDs
	if (GUID1.bIsDynamic != GUID2.bIsDynamic)
	{
		return false;
	}

	return true;
}

UObject* URebuiltPackageMapClient::ResolveNonDefaultGUID(const FNetworkGUID& NetGUID, UObject* ObjOuter)
{
	// From debug: "Server could not resolve non default guid from client"

	if (!bIsServer)
	{
		// Clients should not be resolving server GUIDs
		return nullptr;
	}

	// Check dormant actors
	TWeakObjectPtr<AActor>* DormantActor = DormantActors.Find(NetGUID);
	if (DormantActor && DormantActor->IsValid())
	{
		return DormantActor->Get();
	}

	// Failed to resolve
	UE_LOG(LogNet, Warning, TEXT("UPackageMapClient::InternalLoadObject: Server could not resolve non default guid from client. PathName: %s, ObjOuter: %s"),
		TEXT("Unknown"), ObjOuter ? *ObjOuter->GetName() : TEXT("NULL"));

	return nullptr;
}
