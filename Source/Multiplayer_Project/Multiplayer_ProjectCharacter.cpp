// Copyright Epic Games, Inc. All Rights Reserved.

#include "Multiplayer_ProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"

FOnEnterInstance AMultiplayer_ProjectCharacter::OnEnterInstance;
FOnLeaveInstance AMultiplayer_ProjectCharacter::OnLeaveInstance;
FOnChangeInstance AMultiplayer_ProjectCharacter::OnChangeInstance;

AMultiplayer_ProjectCharacter::AMultiplayer_ProjectCharacter()
{
	//// Configure character movement
	MyMovement = GetCharacterMovement();
	MyMovement->bOrientRotationToMovement = true; // Rotate character to moving direction
	MyMovement->RotationRate = FRotator(0.f, 640.f, 0.f);
	MyMovement->bConstrainToPlane = true;
	MyMovement->bSnapToPlaneAtStart = true;
	
	// Networking
	bReplicates = true;
}

void AMultiplayer_ProjectCharacter::EnterInstance(AMultiplayer_ProjectCharacter* actor, int32 instanceNumber)
{
	OnEnterInstance.Execute(actor, instanceNumber);
}

void AMultiplayer_ProjectCharacter::LeaveInstance(AMultiplayer_ProjectCharacter* actor, int32 instanceNumber)
{
	OnLeaveInstance.Execute(actor, instanceNumber);
}

void AMultiplayer_ProjectCharacter::ChangeInstance(AMultiplayer_ProjectCharacter* actor, int32 OldNumber, int32 NewNumber)
{
	OnChangeInstance.Execute(actor, OldNumber, NewNumber);
}
