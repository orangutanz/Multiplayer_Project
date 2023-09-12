// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ReplicationGraph.h"
#include "UTO8ReplicationGraph.generated.h"

enum class EClassRepPolicy : uint8
{
	NotRouted,
	RelevantAllConnections,

	// ------------------------------------
	// Spatialized routers into the grid node
	Spatialize_Static,		// use for actors that don't need frequent updates
	Spatialize_Dynamic,		// actors that need frequent updates
	Spatialize_Dormancy,	// actors that can either be Static or Dynamic, determined by their AActor::NetDormancy state
	
	// -------------------------------------

};

class UReplicationGraphNode_ActorList;
class UReplicationGraphNode_GridSpatialization2D;
class UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection;
class UUTO8ReplicationGraphNode_Instance;
class AGameplayDebuggerCategoryReplicator;


UCLASS(Transient, config=Engine)
class MULTIPLAYER_PROJECT_API UUTO8ReplicationGraph : public UReplicationGraph
{
	GENERATED_BODY()
public:
	// ~ begin UReplicationGraph implementation
	virtual void InitGlobalActorClassSettings() override;
	virtual void InitGlobalGraphNodes() override;
	virtual void InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager) override;
	virtual void ResetGameWorldState() override;
	virtual void RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo) override;
	virtual void RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo) override;
	// ~ end	

public:
	/**  Maps the actors that needs to be always relevant across streaming levels */
	TMap<FName, FActorRepListRefView> AlwaysRelevantStreamingLevelActors;

	TMap<int, FActorRepListRefView> InstancedActorList;

private:
	/** Sets class replication rule for a class */
	void InitClassReplicationInfo(FClassReplicationInfo& info, UClass* InClass, bool bSpatilize, float ServerMaxTickRate);

	UPROPERTY()
	TArray<UClass*> SpatializedClasses;

	UPROPERTY()
	TArray<UClass*> NonSpatializedClasses;

	UPROPERTY()
	TArray<UClass*> AlwaysRelevantClasses;

	UPROPERTY()
	TArray<UClass*> InstanceClasses;

	//-------------------------------------
	// ReplicationGraph Nodes

	/** Carves the map into grids and determines if an actor should send network updates
		to a connection depending on the different pre-dfine grids	*/
	UPROPERTY()
	UReplicationGraphNode_GridSpatialization2D* GridNode;

	/** Actors that are always replicated, to everyone.	*/
	UPROPERTY()
	UReplicationGraphNode_ActorList* AlwaysRelevantNode;

	/** Actors that are always replicated, for specific connections. */
	UPROPERTY()
	TMap<UNetConnection*, UReplicationGraphNode_AlwaysRelevant_ForConnection*> AlwaysRelevantForConnectionNodes;

	/** A node that contains actors that are in the instances */
	UPROPERTY()
	TMap<int, UUTO8ReplicationGraphNode_Instance*> InstanceNodes;



protected:
	/** Gets the connection always relevant node from a player controller */
	UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection* GetAlwaysRelevantNode(APlayerController* PlayerController);

#if WITH_GAMEPLAY_DEBUGGER
	void OnGameplayDebuggerOwnerChange(AGameplayDebuggerCategoryReplicator* Debugger, APlayerController* OldOwner);
#endif

	/*	Bind events
	*	Call these events on the server to reroute connection into nodes
	*/
	void OnCharacterNewEquipment(class AMultiplayer_ProjectCharacter* Pawn,
		class AEquipment* NewEquipment, class AEquipment* OldEquipment);

	void OnPlayerAddToInstance(class AMultiplayer_ProjectCharacter* Pawn,
		int32 InstanceNumber);

	void OnPlayerRemoveFromInstance(class AMultiplayer_ProjectCharacter* Pawn,
		int32 InstanceNumber);

	FORCEINLINE bool IsSpatialized(EClassRepPolicy Mapping) 
	{ 
		return Mapping >= EClassRepPolicy::Spatialize_Static;
	}

	/** Gets the mapping to used for the given class	*/
	EClassRepPolicy GetMappingPolicy(UClass* InClass);

	/** Mapes a class to a mapping policy */
	TClassMap<EClassRepPolicy> ClassRepPolicies;

	float GridCellSize = 100000.f;			// the size of one grid
	float SpatialBiasX = -60000.f;			// Min X for replication
	float SpatialBiasY = -60000.f;			// Min Y for replication
	bool bDisableSpatialRebuilding = true;

};
