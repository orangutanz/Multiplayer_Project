// Fill out your copyright notice in the Description page of Project Settings.


#include "UTO8ReplicationGraphNodes.h"
#include "UTO8ReplicationGraph.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebuggerCategoryReplicator.h"
#endif


// ------------------------------------------------------------------------
// UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection

void UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
	Super::GatherActorListsForConnection(Params);

	UUTO8ReplicationGraph* RepGraph = Cast<UUTO8ReplicationGraph>(GetOuter());

	FPerConnectionActorInfoMap& ConnectionAcotrInfoMap = Params.ConnectionManager.ActorInfoMap;
	TMap<FName, FActorRepListRefView>& AlwaysRelevantStreamingLevelActors = RepGraph->AlwaysRelevantStreamingLevelActors;

	for (int32 Idx = AlwaysRelevantStreamingLevelActors.Num() - 1; Idx >= 0; --Idx)
	{
		FName StreamingLevel = AlwaysRelevantStreamingLevels[Idx];
		FActorRepListRefView* ListPtr = AlwaysRelevantStreamingLevelActors.Find(StreamingLevel);

		if (ListPtr == nullptr)
		{
			AlwaysRelevantStreamingLevels.RemoveAtSwap(Idx, 1, false);
			continue;
		}

		FActorRepListRefView& RepList = *ListPtr;
		if (RepList.Num() > 0)
		{
			bool bAllDormant = true;
			for (FActorRepListType Actor : RepList)
			{
				FConnectionReplicationActorInfo& ConnectionActorInfo = ConnectionAcotrInfoMap.FindOrAdd(Actor);
				if (ConnectionActorInfo.bDormantOnConnection == false)
				{
					bAllDormant = false;
					break;
				}
			}

			if (bAllDormant == true)
			{
				AlwaysRelevantStreamingLevels.RemoveAtSwap(Idx, 1, false);
			}
			else
			{
				Params.OutGatheredReplicationLists.AddReplicationActorList(RepList);
			}
		}
	}

#if WITH_GAMEPLAY_DEBUGGER
	if (GameplayDebugger)
	{
		ReplicationActorList.ConditionalAdd(GameplayDebugger);
	}
#endif
}

void UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection::OnClientLevelVisitbilityAdd(FName LevelName, UWorld* LevelWorld)
{
	AlwaysRelevantStreamingLevels.Add(LevelName);
}

void UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection::OnClientLevelVisitbilityRemove(FName LevelName)
{
	AlwaysRelevantStreamingLevels.Remove(LevelName);
}

void UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection::ResetGameWorldState()
{
	AlwaysRelevantStreamingLevels.Empty();
}

// ------------------------------------------------------------------------
// UUTO8ReplicationGraphNode_Instance
void UUTO8ReplicationGraphNode_Instance::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
	if (InstancePlayerList.Num() > 0)
	{
		Params.OutGatheredReplicationLists.AddReplicationActorList(InstancePlayerList);
	}	
	if (InstanceActorList.Num() > 0)
	{
		Params.OutGatheredReplicationLists.AddReplicationActorList(InstanceActorList);
	}
}

void UUTO8ReplicationGraphNode_Instance::AddPlayer(const FNewReplicatedActorInfo& ActorInfo, int32 InstanceNumber)
{
	InstancePlayerList.Add(ActorInfo.Actor);
}

int32 UUTO8ReplicationGraphNode_Instance::RemovePlayer(const FNewReplicatedActorInfo& ActorInfo)
{
	InstancePlayerList.RemoveFast(ActorInfo.Actor);
	return InstancePlayerList.Num();
}

void UUTO8ReplicationGraphNode_Instance::AddActor(const FNewReplicatedActorInfo& ActorInfo, int32 InstanceNumber)
{
	InstanceActorList.Add(ActorInfo.Actor);
}

int32 UUTO8ReplicationGraphNode_Instance::RemoveActor(const FNewReplicatedActorInfo& ActorInfo)
{
	InstanceActorList.RemoveFast(ActorInfo.Actor);
	return InstanceActorList.Num();
}
