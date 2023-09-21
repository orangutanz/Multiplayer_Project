// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ReplicationGraph.h"
#include "UTO8ReplicationGraphNodes.generated.h"


class AGameplayDebuggerCategoryReplicator;

/**	custon nodes
 */
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

UCLASS()
class MULTIPLAYER_PROJECT_API UUTO8ReplicationGraphNode_Instance: public UReplicationGraphNode_ActorList
{
	GENERATED_BODY()

public:
    virtual void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params) override;

	void AddPlayer(const FNewReplicatedActorInfo& ActorInfo);
	int32 RemovePlayer(const FNewReplicatedActorInfo& ActorInfo);
	void AddActor(const FNewReplicatedActorInfo& ActorInfo);
	int32 RemoveActor(const FNewReplicatedActorInfo& ActorInfo);

	inline void SetInstanceNumber(int newInstanceNumber) { instanceNumber = newInstanceNumber; }
	inline int GetInstanceNumber() { return instanceNumber; }
private:
    /** Actors that could potentially become visible or hidden. */
    FActorRepListRefView InstancePlayerList;
	FActorRepListRefView InstanceActorList;

	int32 instanceNumber = 0;
};
