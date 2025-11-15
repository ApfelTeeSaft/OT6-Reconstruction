// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/PackageMapClient.h"
#include "RebuiltPackageMapClient.generated.h"

/**
 * PackageMapClient - Handles network GUIDs for object replication
 */
UCLASS(transient)
class NETWORKINGREBUILD_API URebuiltPackageMapClient : public UPackageMapClient
{
	GENERATED_BODY()

public:
	URebuiltPackageMapClient(const FObjectInitializer& ObjectInitializer);

	//~ Begin UPackageMapClient Interface

	/**
	 * Serialize an object reference into a NetGUID
	 */
	virtual bool SerializeObject(FArchive& Ar, UClass* InClass, UObject*& Obj, FNetworkGUID* OutNetGUID = nullptr) override;

	/**
	 * Serialize a new actor being spawned
	 * Decompiled from: References to "SerializeNewActor" in PackageMapClient
	 */
	virtual bool SerializeNewActor(FArchive& Ar, class UActorChannel* Channel, class AActor*& Actor) override;

	/**
	 * Write/Read object reference
	 */
	virtual bool WriteObject(FArchive& Ar, UObject* ObjOuter, FNetworkGUID NetGUID, const FString& ObjName) override;

	//~ End UPackageMapClient Interface

	/**
	 * Load an object from network GUID
	 */
	UObject* InternalLoadObject(UObject* ObjOuter, const FNetworkGUID& NetGUID, const FString& PathName);

	/**
	 * Assign a network GUID to an object
	 */
	FNetworkGUID AssignNewNetGUID(UObject* Object);

	/**
	 * Get or assign GUID for object
	 */
	FNetworkGUID GetOrAssignNetGUID(UObject* Object);

	/**
	 * Get object from existing GUID
	 */
	UObject* GetObjectFromNetGUID(const FNetworkGUID& NetGUID, const bool bIgnoreMustBeMapped);

	/**
	 * Register dormant actor
	 */
	void RegisterDormantActor(AActor* Actor, const FNetworkGUID& NetGUID);

	/**
	 * Unregister dormant actor
	 */
	void UnregisterDormantActor(AActor* Actor);

protected:
	/**
	 * Validate object can be loaded
	 * Decompiled from: InternalLoadObject error checks
	 */
	bool ValidateObjectForLoad(UObject* Object, const FNetworkGUID& NetGUID, const FString& PathName, UObject* ObjOuter);

	/**
	 * Handle pending kill objects
	 * From debug: "Received reference to pending kill object from client"
	 */
	bool HandlePendingKillObject(UObject* Object, const FString& PathName, UObject* ObjOuter);

	/**
	 * Validate package GUID
	 * From debug: "Default object package guid mismatch! GUID1: %u, GUID2: %u"
	 */
	bool ValidatePackageGUID(const FNetworkGUID& GUID1, const FNetworkGUID& GUID2);

	/**
	 * Resolve non-default GUID
	 * From debug: "Server could not resolve non default guid from client"
	 */
	UObject* ResolveNonDefaultGUID(const FNetworkGUID& NetGUID, UObject* ObjOuter);

private:
	/** Map from NetGUID to Object */
	TMap<FNetworkGUID, TWeakObjectPtr<UObject>> GuidCache;

	/** Map from Object to NetGUID */
	TMap<TWeakObjectPtr<UObject>, FNetworkGUID> ObjectToGuidCache;

	/** List of dormant actors by GUID */
	TMap<FNetworkGUID, TWeakObjectPtr<AActor>> DormantActors;

	/** Next GUID to assign */
	uint32 NextNetGUID;

	/** Whether this is server or client */
	bool bIsServer;
};
