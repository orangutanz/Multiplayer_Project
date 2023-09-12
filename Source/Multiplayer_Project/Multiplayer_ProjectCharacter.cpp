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

FOnNewEquipment AMultiplayer_ProjectCharacter::OnNewEquipment;
FOnEnterInstance AMultiplayer_ProjectCharacter::OnEnterInstance;
FOnLeaveInstance AMultiplayer_ProjectCharacter::OnLeaveInstance;

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


void AMultiplayer_ProjectCharacter::NewEquipmentBroadcast(AEquipment* NewEquipment)
{
	if (!NewEquipment)
	{
		return;
	}

	OnNewEquipment.Broadcast(this, NewEquipment, mEquipment);
	mEquipment = NewEquipment;
}

void AMultiplayer_ProjectCharacter::EnterInstance(AMultiplayer_ProjectCharacter* actor, int32 instanceNumber)
{
	OnEnterInstance.Broadcast(actor, instanceNumber);
}

void AMultiplayer_ProjectCharacter::LeaveInstance(AMultiplayer_ProjectCharacter* actor, int32 instanceNumber)
{
	OnLeaveInstance.Broadcast(actor, instanceNumber);
}