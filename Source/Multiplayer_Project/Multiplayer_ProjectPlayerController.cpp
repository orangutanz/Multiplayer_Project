// Copyright Epic Games, Inc. All Rights Reserved.

#include "Multiplayer_ProjectPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Multiplayer_ProjectCharacter.h"
#include "Engine/World.h"
#include "Engine/GameEngine.h"

AMultiplayer_ProjectPlayerController::AMultiplayer_ProjectPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController::I have Authority!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController::I am just a client!"));
	}
}

void AMultiplayer_ProjectPlayerController::UpdateRoomList()
{
	if (!HasAuthority())
	{
		return;
	}
	// server should categorize players into differnt group to save CPU
	// this is where Replication Graph is recommended
	TArray<AMultiplayer_ProjectCharacter*> PlayerList;
	for (TPlayerControllerIterator<AMultiplayer_ProjectPlayerController>::ServerAll It(GetWorld()); It; ++It)
	{
		// PC is a PlayerController. It may local or remotely controlled.
		AMultiplayer_ProjectPlayerController* PC = *It;
		if (auto player = Cast<AMultiplayer_ProjectCharacter>(PC->GetCharacter()))
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerController::I have got a character!"));
			PlayerList.Add(player);
			bool isInList = false;
		}
		// This can only be done on the server! 
		// Only the server has player controllers for everyone!
		check(GetWorld()->GetNetMode() != NM_Client);
	}
	// now that server has the full list of players
	// do the seperation
}



