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


void AMultiplayer_ProjectPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if(bInputPressed)
	{
		FollowTime += DeltaTime;

		// Look for the touch location
		FVector HitLocation = FVector::ZeroVector;
		FHitResult Hit;
		if(bIsTouch)
		{
			GetHitResultUnderFinger(ETouchIndex::Touch1, ECC_Visibility, true, Hit);
		}
		else //curser control
		{
			GetHitResultUnderCursor(ECC_Visibility, true, Hit);
		}
		HitLocation = Hit.Location;

		// Direct the Pawn towards that location
		APawn* const MyPawn = GetPawn();
		if(MyPawn)
		{
			FVector WorldDirection = (HitLocation - MyPawn->GetActorLocation()).GetSafeNormal();
			MyPawn->AddMovementInput(WorldDirection, 1.f, false);
		}
	}
	else
	{
		FollowTime = 0.f;
	}
}

void AMultiplayer_ProjectPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	InputComponent->BindAction("SetDestination", IE_Pressed, this, &AMultiplayer_ProjectPlayerController::OnSetDestinationPressed);
	InputComponent->BindAction("SetDestination", IE_Released, this, &AMultiplayer_ProjectPlayerController::OnSetDestinationReleased);

	// support touch devices 
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AMultiplayer_ProjectPlayerController::OnTouchPressed);
	InputComponent->BindTouch(EInputEvent::IE_Released, this, &AMultiplayer_ProjectPlayerController::OnTouchReleased);

}

void AMultiplayer_ProjectPlayerController::OnSetDestinationPressed()
{
	// We flag that the input is being pressed
	bInputPressed = true;
	// Just in case the character was moving because of a previous short press we stop it
	StopMovement();
}

void AMultiplayer_ProjectPlayerController::OnSetDestinationReleased()
{
	// Player is no longer pressing the input
	bInputPressed = false;

	// If it was a short press
	if(FollowTime <= ShortPressThreshold)
	{
		// We look for the location in the world where the player has pressed the input
		FVector HitLocation = FVector::ZeroVector;
		FHitResult Hit;
		GetHitResultUnderCursor(ECC_Visibility, true, Hit);
		HitLocation = Hit.Location;

		// We move there and spawn some particles
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, HitLocation);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, HitLocation, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}
}

void AMultiplayer_ProjectPlayerController::OnTouchPressed(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	bIsTouch = true;
	OnSetDestinationPressed();
}

void AMultiplayer_ProjectPlayerController::OnTouchReleased(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	bIsTouch = false;
	OnSetDestinationReleased();
}
