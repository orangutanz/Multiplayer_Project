// Copyright Epic Games, Inc. All Rights Reserved.

#include "Multiplayer_ProjectGameMode.h"
#include "Multiplayer_ProjectPlayerController.h"
#include "Multiplayer_ProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMultiplayer_ProjectGameMode::AMultiplayer_ProjectGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AMultiplayer_ProjectPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Character/Blueprints/BP_TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set default controller to our Blueprinted controller
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/Character/Blueprints/BP_TopDownController"));
	if(PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}

}
