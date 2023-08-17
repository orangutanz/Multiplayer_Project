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
};



class UReplicationGraphNode_ActorList;
class UReplicationGraphNode_GridSpatialization2D;
class UReplicationGraphNode_AlwaysRelevant_ForConnection;
class AGameplayDebuggerCategoryReplicator;

/**
 * 
 */
UCLASS(Transient, config=Engine)
class MULTIPLAYER_PROJECT_API UUTO8ReplicationGraph : public UReplicationGraph
{
	GENERATED_BODY()
public:
	/**  Maps the actors that needs to be always relevant across streaming levels */
	TMap<FName, FActorRepListRefView> AlwaysRelevantStreamingLevelActors;

private:
	// ~ begin UReplicationGraph implementation
	virtual void ResetGameWorldState() override;
	virtual void InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager) override;
	virtual void InitGlobalActorClassSettings() override;
	virtual void InitGlobalGraphNodes() override;
	virtual void RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo) override;
	virtual void RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo) override;
	// ~ end

	/** Sets class replication rule for a class */
	void InitClassReplicationInfo(FClassReplicationInfo& info, UClass* InClass, bool bSpatilize, float ServerMaxTickRate);

	UPROPERTY()
	TArray<UClass*> SpatializedClasses;

	UPROPERTY()
	TArray<UClass*> NonSpatializedClasses;

	UPROPERTY()
	TArray<UClass*> AlwaysRelevantClasses;



	//-------------------------------------
	// ReplicationGraph Nodes

	/**
	*  This is problebly the most important node in the replication graph 
	* 
	*  Carves the map into grids and determines if an actor should send network updates
	*  to a connection depending on the different pre-dfine grids
	*/
	UPROPERTY()
	UReplicationGraphNode_GridSpatialization2D* GridNode;

	UPROPERTY()
	UReplicationGraphNode_ActorList* AlwaysRelevantNode;


protected:
	/** Gets the connection always relevant node from a player controller */
	UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection* GetAlwaysRelevantNode(APlayerController* PlayerController);

#if WITH_GAMEPLAY_DEBUGGER
	void OnGameplayDebuggerOwnerChange(AGameplayDebuggerCategoryReplicator* Debugger, APlayerController* OldOwner);
#endif

	void OnCharacterNewEquipment(class AMultiplayer_ProjectCharacter* Pawn,
		class AEquipment* NewEquipment, class AEquipment* OldEquipment);

	void OnCharacterEnterLiveroom(class AMultiplayer_ProjectCharacter* Pawn,
		int RoomNumber);

	FORCEINLINE bool IsSpatialized(EClassRepPolicy Mapping) 
	{ 
		return Mapping >= EClassRepPolicy::Spatialize_Static;
	}

	/** Gets the mapping to used for the given class	*/
	EClassRepPolicy GetMappingPolicy(UClass* InClass);

	/** Mapes a class to a mapping policy */
	TClassMap<EClassRepPolicy> ClassRepPolicies;

	float GridCellSize = 50000.f;			// the size of one grid
	float SpatialBiasX = -150000.f;			// Min X for replication
	float SpatialBiasY = -200000.f;			// Min Y for replication
	bool bDisableSpatialRebuilding = true;

};

// custon grid node
UCLASS()
class UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection : public UReplicationGraphNode_AlwaysRelevant_ForConnection
{
public:
	GENERATED_BODY()

public:

	// ~ begin UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection implementation
	virtual void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params) override;
	// ~ end

	void OnClientLevelVisitbilityAdd(FName LevelName, UWorld* LevelWorld);
	void OnClientLevelVisitbilityRemove(FName LevelName);

	void ResetGameWorldState();

#if WITH_GAMEPLAY_DEBUGGER
	AGameplayDebuggerCategoryReplicator* GameplayDebugger = nullptr;
#endif

protected:

	/** Stores levelstreaming acotrs */
	TArray<FName, TInlineAllocator<64>> AlwaysRelevantStreamingLevels;
};