// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoxRoom.generated.h"

UCLASS()
class MULTIPLAYER_PROJECT_API ABoxRoom : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABoxRoom();

	virtual void OnConstruction(const FTransform& transform) override;

	UPROPERTY(EditAnywhere)
	class UInstancedStaticMeshComponent* Walls;

	UPROPERTY(EditAnywhere)
	class UInstancedStaticMeshComponent* Edges;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0))
	float GridSize;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 1, CLampMax = 25))
	uint32 RoomSize;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 1, CLampMax = 5))
	uint32 RoomHeight;

private:
	uint32 BuiltGridSize;
	uint32 BuiltRoomSize;


};
