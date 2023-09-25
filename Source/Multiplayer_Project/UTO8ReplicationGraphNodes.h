// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ReplicationGraph.h"
#include "Engine/DataTable.h"
#include "Engine/LevelStreamingDynamic.h"
#include <array>
#include "UTO8ReplicationGraphNodes.generated.h"

struct GridInfo
{
	GridInfo() : PosX(0.0), PosY(0.0), PosZ(0.0), Column(0), Row(0), IsInUse(false) {}
	GridInfo(float X, float Y, float Z) : PosX(0.0), PosY(0.0), PosZ(0.0), Column(0), Row(0), IsInUse(false) {}
	~GridInfo() { ; }

	bool operator==(const GridInfo& Other) const
	{
		return (Column == Other.Column && Row == Other.Row);
	}
	bool operator!=(const GridInfo& Other) const
	{
		return !(*this == Other);
	}

	float PosX = 0.0;
	float PosY = 0.0;
	float PosZ = 0.0;
	int8 Column = 0;
	int8 Row = 0;
	bool IsInUse = false;
};

class InstanceSpaceMaker
{
public:
	InstanceSpaceMaker();
	~InstanceSpaceMaker();

	GridInfo GetAvailableGrid();

	void Initialize_Space();
	void Initialize_Space(float OriginX, float OriginY, float OriginZ, float GridSize);
	void ReturnGrid(GridInfo Position);

private:
	float mPosX = 0;
	float mPosY = 0;
	float mPosZ = 6000;
	float mGridSize = 6000.0f; // 10,000 target
	int mGridAmount = 50; // 50*50=2500
	std::array<GridInfo, 2500> mGridArray;
};

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

	void TearDownGraphNode();

	void AddPlayer(const FNewReplicatedActorInfo& ActorInfo);
	int32 RemovePlayer(const FNewReplicatedActorInfo& ActorInfo);

	void AddActor(const FNewReplicatedActorInfo& ActorInfo);
	void RemoveActor(const FNewReplicatedActorInfo& ActorInfo);

	inline int GetPlayerSize() { return InstancePlayerList.Num(); }
	inline FVector GetMapPosition() { return { GridInfo.PosX,GridInfo.PosY,GridInfo.PosZ }; }
	inline const FString& GetMapName() { return MapName; }
	inline ULevelStreamingDynamic* GetMapPtr() { return InstancedMap; }
	inline GridInfo GetGridInfo() { return GridInfo; }
	inline const FString& GetNodeName() { return NodeName; }
	inline void SetNodeName(const FString& inName) { NodeName = inName; }
	inline void SetMapInfo(ULevelStreamingDynamic* MapPtr, GridInfo Grid, FString Name)
	{
		InstancedMap = MapPtr;
		GridInfo = Grid;
		MapName = Name;
	}
private:
	FActorRepListRefView InstancePlayerList;
	FActorRepListRefView InstanceActorList;

	ULevelStreamingDynamic* InstancedMap;
	GridInfo GridInfo;
	FString MapName;
	FString NodeName;
	double CullDistance;
};
