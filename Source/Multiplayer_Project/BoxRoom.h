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

	UPROPERTY(EditAnywhere, meta = (Clamp = 0))
	float GridSize;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 1, CLampMax = 25))
	uint32 RoomSize;

private:
	uint32 BuiltGridSize;
	uint32 BuiltRoomSize;


};
