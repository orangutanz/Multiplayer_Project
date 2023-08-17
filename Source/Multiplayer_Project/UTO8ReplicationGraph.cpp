// Fill out your copyright notice in the Description page of Project Settings.

#include "UTO8ReplicationGraph.h"
#include "Engine/LevelScriptActor.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebuggerCategoryReplicator.h"
#endif

#include "Multiplayer_ProjectCharacter.h"
#include "Equipment.h"
#include "Projectile.h"

void UUTO8ReplicationGraph::ResetGameWorldState()
{
	Super::ResetGameWorldState();
	AlwaysRelevantStreamingLevelActors.Empty();

	for (auto& ConnectionList : { Connections,PendingConnections })
	{
		for (UNetReplicationGraphConnection* Connection : ConnectionList )
		{
			for (UReplicationGraphNode* ConnectionNode : Connection->GetConnectionGraphNodes())
			{
				UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection* Node = Cast<UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection>(ConnectionNode);
				if (Node != nullptr)
				{
					Node->ResetGameWorldState();
				}
			}
		}
	}
}

void UUTO8ReplicationGraph::InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager)
{
	UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection* Node = CreateNewNode<UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection>();
	ConnectionManager->OnClientVisibleLevelNameAdd.AddUObject(Node, &UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection::OnClientLevelVisitbilityAdd);
	ConnectionManager->OnClientVisibleLevelNameRemove.AddUObject(Node,
		&UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection::OnClientLevelVisitbilityRemove);
	AddConnectionGraphNode(Node, ConnectionManager);
}

void UUTO8ReplicationGraph::InitGlobalActorClassSettings()
{
	Super::InitGlobalActorClassSettings();

	// ------------------------------------------
	// Assign mapping to classes
	auto SetRule = [&](UClass* InClass, EClassRepPolicy Mapping) { ClassRepPolicies.Set(InClass, Mapping); };
	// System classes
	SetRule(AReplicationGraphDebugActor::StaticClass(),		 EClassRepPolicy::NotRouted);
	SetRule(ALevelScriptActor::StaticClass(),				 EClassRepPolicy::NotRouted);
	SetRule(AInfo::StaticClass(),							 EClassRepPolicy::RelevantAllConnections);
	
	// Game classes
	SetRule(AProjectile::StaticClass(),						 EClassRepPolicy::Spatialize_Dynamic);
	SetRule(AMultiplayer_ProjectCharacter::StaticClass(),	 EClassRepPolicy::Spatialize_Dynamic);

#if WITH_GAMEPLAY_DEBUGGER
	SetRule(AGameplayDebuggerCategoryReplicator::StaticClass(), EClassRepPolicy::NotRouted);
#endif

	// Inter through all the UClasses including blueprints to determind their replication routes
	TArray<UClass*> ReplicatedClasses;
	for (TObjectIterator<UClass> Itr; Itr; ++Itr)
	{
		UClass* Class = *Itr;
		AActor* ActorCDO = Cast<AActor>(Class->GetDefaultObject());

		// Do not add the actor if it does not replicate
		if (!ActorCDO || !ActorCDO->GetIsReplicated())
		{
			continue;
		}

		// Do not add SKEL and REINST classes
		FString ClassName = Class->GetName();
		if (ClassName.StartsWith("SKEL_") || ClassName.StartsWith("REINST_"))
		{
			continue;
		}

		ReplicatedClasses.Add(Class);

		// if we already have mapped it to the policy, don't do it again
		if (ClassRepPolicies.Contains(Class, false))
		{
			continue;
		}

		auto ShouldSpatialize = [](const AActor* Actor)
		{
			return Actor->GetIsReplicated() && 
				(!(Actor->bAlwaysRelevant || Actor->bOnlyRelevantToOwner || Actor->bNetUseOwnerRelevancy));
		};

		UClass* SuperClass = Class->GetSuperClass();
		if (AActor* SuperCDO = Cast<AActor>(SuperClass->GetDefaultObject()))
		{
			if	(SuperCDO->GetIsReplicated() == ActorCDO->GetIsReplicated()
				&& (SuperCDO->bAlwaysRelevant == ActorCDO->bAlwaysRelevant)
				&& (SuperCDO->bOnlyRelevantToOwner == ActorCDO->bOnlyRelevantToOwner)
				&& (SuperCDO->bNetUseOwnerRelevancy == ActorCDO->bNetUseOwnerRelevancy))
			{
				continue;
			}

			if (ShouldSpatialize(ActorCDO) == false && ShouldSpatialize(SuperCDO) == true)
			{
				NonSpatializedClasses.Add(Class);
			}
		}

		if (ShouldSpatialize(ActorCDO) == true)
		{
			SetRule(Class, EClassRepPolicy::Spatialize_Dynamic);
		}
		else if (ActorCDO->bAlwaysRelevant && !ActorCDO->bOnlyRelevantToOwner)
		{
			SetRule(Class, EClassRepPolicy::RelevantAllConnections);
		}
	}


	// -----------------------------------------
	// Explitialy set replication info for classes

	TArray<UClass*> ExplicitlySetClasses;

	auto SetClassInfo = [&](UClass* InClass, FClassReplicationInfo& RepInfo)
	{
		GlobalActorReplicationInfoMap.SetClassInfo(InClass, RepInfo);
		ExplicitlySetClasses.Add(InClass);
	};

	FClassReplicationInfo PawnClassInfo;
	PawnClassInfo.SetCullDistanceSquared(300000.f * 300000.f);
	SetClassInfo(APawn::StaticClass(), PawnClassInfo);

	for (UClass* ReplicatedClass : ReplicatedClasses)
	{
		if (ExplicitlySetClasses.FindByPredicate([&](const UClass* InClass) { return ReplicatedClass->IsChildOf(InClass);}) != nullptr)
		{
			continue;
		}
		bool bSpatialize = IsSpatialized(ClassRepPolicies.GetChecked(ReplicatedClass));

		FClassReplicationInfo ClassInfo;
		InitClassReplicationInfo(ClassInfo, ReplicatedClass, bSpatialize, NetDriver->NetServerMaxTickRate);
		GlobalActorReplicationInfoMap.SetClassInfo(ReplicatedClass, ClassInfo);
	}

	// ----------------------------------------------
	// Bind evnets here
	AMultiplayer_ProjectCharacter::OnNewEquipment.AddUObject(this, &UUTO8ReplicationGraph::OnCharacterNewEquipment);

#if WITH_GAMEPLAY_DEBUGGER
	AGameplayDebuggerCategoryReplicator::NotifyDebuggerOwnerChange.AddUObject(this, &UUTO8ReplicationGraph::OnGameplayDebuggerOwnerChange);
	
#endif
}	

void UUTO8ReplicationGraph::InitGlobalGraphNodes()
{

	// -----------------------------
	// Create grid node
	GridNode = CreateNewNode<UReplicationGraphNode_GridSpatialization2D>();
	GridNode->CellSize = GridCellSize;
	GridNode->SpatialBias = FVector2D(SpatialBiasX,SpatialBiasY);

	if (bDisableSpatialRebuilding == true)
	{
		GridNode->AddToClassRebuildDenyList(AActor::StaticClass());
	}

	AddGlobalGraphNode(GridNode);

	// ----------------------------------
	// Create always relevant node
	AlwaysRelevantNode = CreateNewNode<UReplicationGraphNode_ActorList>();
	AddGlobalGraphNode(AlwaysRelevantNode);
}

void UUTO8ReplicationGraph::RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo)
{
	EClassRepPolicy MappingPolicy = GetMappingPolicy(ActorInfo.Class);
	switch (MappingPolicy)
	{
	case EClassRepPolicy::RelevantAllConnections:
		if (ActorInfo.StreamingLevelName == NAME_None)
		{
			AlwaysRelevantNode->NotifyAddNetworkActor(ActorInfo);
		}
		else
		{
			FActorRepListRefView& RepList = AlwaysRelevantStreamingLevelActors.FindOrAdd(ActorInfo.StreamingLevelName);
			RepList.ConditionalAdd(ActorInfo.Actor);
		}
		break;

	case EClassRepPolicy::Spatialize_Static:
		GridNode->AddActor_Static(ActorInfo, GlobalInfo);
		break;

	case EClassRepPolicy::Spatialize_Dynamic:
		GridNode->AddActor_Dynamic(ActorInfo, GlobalInfo);
		break;

	case EClassRepPolicy::Spatialize_Dormancy:
		GridNode->AddActor_Dormancy(ActorInfo, GlobalInfo);
		break;

	default:
		break;
	}
}

void UUTO8ReplicationGraph::RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo)
{
	EClassRepPolicy MappingPolicy = GetMappingPolicy(ActorInfo.Class);
	switch (MappingPolicy)
	{
	case EClassRepPolicy::RelevantAllConnections:
		if (ActorInfo.StreamingLevelName == NAME_None)
		{
			AlwaysRelevantNode->NotifyRemoveNetworkActor(ActorInfo);
		}
		else
		{
			FActorRepListRefView& RepList = AlwaysRelevantStreamingLevelActors.FindOrAdd(ActorInfo.StreamingLevelName);
			RepList.RemoveFast(ActorInfo.Actor);
		}
		break;

	case EClassRepPolicy::Spatialize_Static:
		GridNode->RemoveActor_Static(ActorInfo);
		break;

	case EClassRepPolicy::Spatialize_Dynamic:
		GridNode->RemoveActor_Dynamic(ActorInfo);
		break;

	case EClassRepPolicy::Spatialize_Dormancy:
		GridNode->RemoveActor_Dormancy(ActorInfo);
		break;

	default:
		break;
	}

}

void UUTO8ReplicationGraph::InitClassReplicationInfo(FClassReplicationInfo& info, UClass* InClass, bool bSpatilize, float ServerMaxTickRate)
{
	if (AActor* CDO = Cast<AActor>(InClass->GetDefaultObject()))
	{
		if (bSpatilize == true)
		{
			info.SetCullDistanceSquared(CDO->NetCullDistanceSquared);
		}

		info.ReplicationPeriodFrame = FMath::Max<uint32>((uint32)FMath::RoundToFloat(ServerMaxTickRate / CDO->NetUpdateFrequency), 1);
	}
}

UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection* UUTO8ReplicationGraph::GetAlwaysRelevantNode(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return nullptr;
	}
	if (UNetConnection* NetConnection = PlayerController->NetConnection)
	{
		if (UNetReplicationGraphConnection* GraphConnection = FindOrAddConnectionManager(NetConnection))
		{
			for (UReplicationGraphNode* ConnectionNode : GraphConnection->GetConnectionGraphNodes())
			{
				auto Node = Cast<UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection>(ConnectionNode);
				if (Node)
				{
					return Node;
				}
			}
		}
	}

	return nullptr;
}

#if WITH_GAMEPLAY_DEBUGGER
void UUTO8ReplicationGraph::OnGameplayDebuggerOwnerChange(AGameplayDebuggerCategoryReplicator* Debugger, APlayerController* OldOwner)
{
	if (UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection* Node = GetAlwaysRelevantNode(OldOwner))
	{
		Node->GameplayDebugger = nullptr;
	}

	if (UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection* Node = GetAlwaysRelevantNode(Debugger->GetReplicationOwner()))
	{
		Node->GameplayDebugger = Debugger;
	}
}
#endif

void UUTO8ReplicationGraph::OnCharacterNewEquipment(AMultiplayer_ProjectCharacter* Pawn, AEquipment* NewEquipment, AEquipment* OldEquipment)
{
	if (!Pawn || Pawn->GetWorld() != GetWorld())
	{
		return;
	}

	if (NewEquipment)
	{
		GlobalActorReplicationInfoMap.AddDependentActor(Pawn, NewEquipment);
	}
	if (OldEquipment)
	{
		GlobalActorReplicationInfoMap.RemoveDependentActor(Pawn, OldEquipment);
	}
}

void UUTO8ReplicationGraph::OnCharacterEnterLiveroom(AMultiplayer_ProjectCharacter* Pawn, int RoomNumber)
{
	if (!Pawn || Pawn->GetWorld() != GetWorld())
	{
		return;
	}
}

EClassRepPolicy UUTO8ReplicationGraph::GetMappingPolicy(UClass* InClass)
{	
	return ClassRepPolicies.Get(InClass) != NULL ? *ClassRepPolicies.Get(InClass) : EClassRepPolicy::NotRouted;
}

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
