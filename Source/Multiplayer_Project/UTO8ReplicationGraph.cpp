// Fill out your copyright notice in the Description page of Project Settings.

#include "UTO8ReplicationGraph.h"
#include "UTO8ReplicationGraphNodes.h"
#include "Engine/LevelScriptActor.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebuggerCategoryReplicator.h"
#endif

#include "Multiplayer_ProjectCharacter.h"
#include "Equipment.h"
#include "Projectile.h"

UUTO8ReplicationGraph::UUTO8ReplicationGraph()
{
	InstanceSpaceMaker_Island = new InstanceSpaceMaker();
	InstanceSpaceMaker_Island->Initialize_Space();
}

UUTO8ReplicationGraph::~UUTO8ReplicationGraph()
{
	delete InstanceSpaceMaker_Island;
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

		// helper lamda
		auto ShouldSpatialize = [](const AActor* Actor)
		{
			return Actor->GetIsReplicated() && 
				(!(Actor->bAlwaysRelevant || Actor->bOnlyRelevantToOwner || Actor->bNetUseOwnerRelevancy));
		};

		// check is replication policy different from parent
		UClass* SuperClass = Class->GetSuperClass();
		if (AActor* SuperCDO = Cast<AActor>(SuperClass->GetDefaultObject()))
		{
			if	(SuperCDO->GetIsReplicated() == ActorCDO->GetIsReplicated()
				&& (SuperCDO->bAlwaysRelevant == ActorCDO->bAlwaysRelevant)
				&& (SuperCDO->bOnlyRelevantToOwner == ActorCDO->bOnlyRelevantToOwner)
				&& (SuperCDO->bNetUseOwnerRelevancy == ActorCDO->bNetUseOwnerRelevancy))
			{
				continue; // using same repPolicy skip to next one
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
	PawnClassInfo.SetCullDistanceSquared(12000.f * 12000.f);
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
	AMultiplayer_ProjectCharacter::OnChangeInstance.BindUObject(this, &UUTO8ReplicationGraph::OnPlayerChangeInstance);
	AMultiplayer_ProjectCharacter::OnEnterInstance.BindUObject(this, &UUTO8ReplicationGraph::OnPlayerAddToInstance);
	AMultiplayer_ProjectCharacter::OnLeaveInstance.BindUObject(this, &UUTO8ReplicationGraph::OnPlayerRemoveFromInstance);

#if WITH_GAMEPLAY_DEBUGGER
	AGameplayDebuggerCategoryReplicator::NotifyDebuggerOwnerChange.AddUObject(this, &UUTO8ReplicationGraph::OnGameplayDebuggerOwnerChange);
	
#endif
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

void UUTO8ReplicationGraph::InitGlobalGraphNodes()
{
	Super::InitGlobalGraphNodes();

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

void UUTO8ReplicationGraph::InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager)
{
	Super::InitConnectionGraphNodes(ConnectionManager);

	// Setup node for actors that are always replicated, for the specified connection. support level streaming
	UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection* Node = CreateNewNode<UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection>();
	ConnectionManager->OnClientVisibleLevelNameAdd.AddUObject(Node, &UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection::OnClientLevelVisitbilityAdd);
	ConnectionManager->OnClientVisibleLevelNameRemove.AddUObject(Node,
		&UUTO8ReplicationGraphNode_AlwaysRelavent_ForConnection::OnClientLevelVisitbilityRemove);
	AddConnectionGraphNode(Node, ConnectionManager);
}

void UUTO8ReplicationGraph::ResetGameWorldState()
{
	Super::ResetGameWorldState();
	AlwaysRelevantStreamingLevelActors.Empty();

	for (auto& ConnectionList : { Connections,PendingConnections })
	{
		for (UNetReplicationGraphConnection* Connection : ConnectionList)
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

void UUTO8ReplicationGraph::OnPlayerAddToInstance(AMultiplayer_ProjectCharacter* Pawn, int32 InstanceNumber, const FString& mapName)
{
	if (!Pawn || Pawn->GetWorld() != GetWorld())
	{
		return;
	}
	UUTO8ReplicationGraphNode_Instance* nodeRef = InstanceNodes.FindRef(InstanceNumber);
	if (!nodeRef)
	{
		// instance Node doesn't exist, make a new one
		nodeRef = CreateNewNode<UUTO8ReplicationGraphNode_Instance>();
		if (!mapName.IsEmpty())
		{
			GridInfo Grid = InstanceSpaceMaker_Island->GetAvailableGrid();
			nodeRef->SetNodeName(nodeRef->GetFName().ToString());
			FTransform transform;
			transform.SetLocation({ Grid.PosX, Grid.PosY, Grid.PosZ });
			ULevelStreamingDynamic::FLoadLevelInstanceParams params(GetWorld(), mapName, transform);
			params.OptionalLevelNameOverride = &nodeRef->GetNodeName();
			bool Success = false;
			auto MapPtr = ULevelStreamingDynamic::LoadLevelInstance(params, Success);
			if (!Success)
			{
				InstanceSpaceMaker_Island->ReturnGrid(Grid);
				return;
			}
			nodeRef->SetMapInfo(MapPtr, Grid, mapName);
			MapPtr->OnLevelLoaded.Add(Pawn->OnLevelLoaded);
			Pawn->mapPosition = nodeRef->GetMapPosition();
			Pawn->CLIENT_LoadInstancedMap(nodeRef->GetMapName(), nodeRef->GetMapPosition(), nodeRef->GetNodeName());
		}
		InstanceNodes.Add(InstanceNumber, nodeRef);
	}
	else
	{
		// add to existing Node
		if (!mapName.IsEmpty())
		{
			Pawn->mapPosition = nodeRef->GetMapPosition();
			Pawn->CLIENT_LoadInstancedMap(nodeRef->GetMapName(), nodeRef->GetMapPosition(), nodeRef->GetNodeName());
			Pawn->InstanceTeleport();
		}
	}	
	auto actorInfo = FNewReplicatedActorInfo(Pawn);
	auto netConnection = Pawn->GetNetConnection();
	nodeRef->AddPlayer(actorInfo);
	GridNode->RemoveActor_Dynamic(actorInfo);
	AddConnectionGraphNode(nodeRef, netConnection);
	RemoveConnectionGraphNode(GridNode, netConnection);
}

void UUTO8ReplicationGraph::OnPlayerRemoveFromInstance(AMultiplayer_ProjectCharacter* Pawn, int32 InstanceNumber)
{
	// Get world is wrong for some reason
	if (!Pawn /* || Pawn->GetWorld() != GetWorld()*/)
	{
		return;
	}
	auto nodeRef = InstanceNodes.FindRef(InstanceNumber);
	if (nodeRef)
	{
		// remove from Node
		auto actorInfo = FNewReplicatedActorInfo(Pawn);
		auto netConnection = Pawn->GetNetConnection();
		int32 NodePlayerSize = nodeRef->RemovePlayer(actorInfo);
		if (NodePlayerSize == 0)
		{
			if (!nodeRef->GetMapPtr())
			{
				InstanceSpaceMaker_Island->ReturnGrid(nodeRef->GetGridInfo());
			}
			nodeRef->TearDownGraphNode();
			InstanceNodes.Remove(InstanceNumber);
		}
		if (!nodeRef->GetMapPtr())
		{
			Pawn->CLIENT_RemoveInstancedMap();
			Pawn->InstanceTeleportBack();
		}
		// dummy variables, only for GridNode->AddActor_Dynamic. The actual function only need ActorInfo
		auto dummyClassInfo = FClassReplicationInfo();
		auto dummy = FGlobalActorReplicationInfo(dummyClassInfo);
		// add to GridNode
		GridNode->AddActor_Dynamic(actorInfo, dummy);
		AddConnectionGraphNode(GridNode, netConnection);
		RemoveConnectionGraphNode(nodeRef, netConnection);
	}
}

void UUTO8ReplicationGraph::OnPlayerChangeInstance(AMultiplayer_ProjectCharacter* Pawn, int32 OldNumber, int32 NewNumber, const FString& mapName)
{
	if (!Pawn)
	{
		return;
	}
	UUTO8ReplicationGraphNode_Instance* oldNodeRef = InstanceNodes.FindRef(OldNumber);
	if (!oldNodeRef)
	{
		return;
	}
	UUTO8ReplicationGraphNode_Instance* newNodeRef = InstanceNodes.FindRef(NewNumber);
	if (!newNodeRef)
	{
		// instance Node doesn't exist, make a new one
		newNodeRef = CreateNewNode<UUTO8ReplicationGraphNode_Instance>();
		if (!mapName.IsEmpty())
		{
			GridInfo Grid = InstanceSpaceMaker_Island->GetAvailableGrid();
			newNodeRef->SetNodeName(newNodeRef->GetFName().ToString());
			FTransform transform;
			transform.SetLocation({ Grid.PosX, Grid.PosY, Grid.PosZ });
			ULevelStreamingDynamic::FLoadLevelInstanceParams params(GetWorld(), mapName, transform);
			params.OptionalLevelNameOverride = &newNodeRef->GetNodeName();
			bool Success = false;
			auto MapPtr = ULevelStreamingDynamic::LoadLevelInstance(params, Success);
			if (!Success)
			{
				InstanceSpaceMaker_Island->ReturnGrid(Grid);
				return;
			}
			newNodeRef->SetMapInfo(MapPtr, Grid, mapName);
			Pawn->mapPosition = newNodeRef->GetMapPosition();
			Pawn->CLIENT_LoadInstancedMap(newNodeRef->GetMapName(), newNodeRef->GetMapPosition(), newNodeRef->GetNodeName());
			MapPtr->OnLevelLoaded.Add(Pawn->OnLevelLoaded);
		}
		InstanceNodes.Add(NewNumber, newNodeRef);
	}
	else
	{
		if (!mapName.IsEmpty())
		{
			Pawn->mapPosition = newNodeRef->GetMapPosition();
			Pawn->CLIENT_LoadInstancedMap(newNodeRef->GetMapName(), newNodeRef->GetMapPosition(), newNodeRef->GetNodeName());
			Pawn->InstanceTeleport_Async();
		}
	}

	auto actorInfo = FNewReplicatedActorInfo(Pawn);
	auto netConnection = Pawn->GetNetConnection();
	// reroute nodes
	newNodeRef->AddPlayer(actorInfo);
	int32 NodePlayerSize = oldNodeRef->RemovePlayer(actorInfo);
	if (NodePlayerSize == 0)
	{
		if (!mapName.IsEmpty())
		{
			InstanceSpaceMaker_Island->ReturnGrid(oldNodeRef->GetGridInfo());
		}
		oldNodeRef->TearDownGraphNode();
		InstanceNodes.Remove(OldNumber);
	}
	AddConnectionGraphNode(newNodeRef, netConnection);
	RemoveConnectionGraphNode(oldNodeRef, netConnection);
}

EClassRepPolicy UUTO8ReplicationGraph::GetMappingPolicy(UClass* InClass)
{	
	return ClassRepPolicies.Get(InClass) != NULL ? *ClassRepPolicies.Get(InClass) : EClassRepPolicy::NotRouted;
}
