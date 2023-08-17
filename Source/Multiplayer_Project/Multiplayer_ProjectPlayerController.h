// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/PlayerController.h"
#include "Multiplayer_ProjectPlayerController.generated.h"

/** Forward declaration to improve compiling times */
class UNiagaraSystem;

UCLASS()
class AMultiplayer_ProjectPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMultiplayer_ProjectPlayerController();

	/** Call RoomList Update*/
	UFUNCTION(BlueprintCallable)
	void UpdateRoomList();


protected:

	float FollowTime; // For how long it has been pressed
};


