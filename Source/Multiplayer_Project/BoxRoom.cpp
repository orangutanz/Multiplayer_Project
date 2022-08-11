// Fill out your copyright notice in the Description page of Project Settings.


#include "BoxRoom.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Multiplayer_Project.h"

// Sets default values
ABoxRoom::ABoxRoom()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GridSize = 1000.0f;
	RoomSize = 3;

	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	Walls = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Walls"));
	Walls->SetupAttachment(SceneComponent);

	Edges = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Edges"));
	Edges->SetupAttachment(SceneComponent);
}

//Add instance also rotate accordingly
static FORCEINLINE void AddInstance(UInstancedStaticMeshComponent* Component, 
	const FRotator& Rotation, const FVector& Translation)
{
	Component->AddInstance(FTransform(Rotation, Rotation.RotateVector(Translation)));
}

static const FRotator PositiveX(0.0f, 0.0f, 0.0f);
static const FRotator NegativeX(0.0f, 180.0f, 0.0f);
static const FRotator PositiveY(0.0f, 90.0f, 0.0f);
static const FRotator NegativeY(0.0f, 270.0f, 0.0f);
static const FRotator PositiveZ(90.0f, 0.0f, 0.0f);
static const FRotator NegativeZ(-90.0f, 0.0f, 0.0f);

void ABoxRoom::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	//Only rebuild when size is changed
	if (BuiltGridSize == GridSize && BuiltRoomSize == RoomSize)
		return;

	UE_LOG(LogMultiplayer_Project,Log,
		TEXT("ABoxRoom::OnConstruction Building Room Size %d (Built=%d this=%s)"),
		RoomSize, BuiltRoomSize, this);

	BuiltGridSize = GridSize;
	BuiltRoomSize = RoomSize;

	Walls->ClearInstances();
	Edges->ClearInstances();

	//build from the center
	int32 HalfSize = RoomSize * 0.5;
	int32 WallOffset = (HalfSize + 1) * GridSize;
	FVector Translation(WallOffset, 0.0f, 0.0f);

	for (int32 a = -HalfSize; a <= HalfSize; a++)
	{
		Translation.Y = GridSize * a;
		for (int32 b = -HalfSize; b <= HalfSize; b++)
		{
			Translation.Z = GridSize * b;
			AddInstance(Walls, PositiveX, Translation);
			AddInstance(Walls, PositiveY, Translation);
			//AddInstance(Walls, PositiveZ, Translation);
			AddInstance(Walls, NegativeX, Translation);
			AddInstance(Walls, NegativeY, Translation);
			AddInstance(Walls, NegativeZ, Translation);
		}

		Translation.Z = -WallOffset;
		AddInstance(Edges, PositiveX, Translation);
		AddInstance(Edges, PositiveY, Translation);
		AddInstance(Edges, NegativeX, Translation);
		AddInstance(Edges, NegativeY, Translation);
	}



}


