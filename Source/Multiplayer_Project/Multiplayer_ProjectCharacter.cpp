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
FOnEnterLiveroom AMultiplayer_ProjectCharacter::OnEnterLiveroom;

AMultiplayer_ProjectCharacter::AMultiplayer_ProjectCharacter()
{
	// Set size for player capsule
	//GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	//bUseControllerRotationPitch = false;
	//bUseControllerRotationYaw = false;
	//bUseControllerRotationRoll = false;
	//
	//// Configure character movement
	MyMovement = GetCharacterMovement();
	MyMovement->bOrientRotationToMovement = true; // Rotate character to moving direction
	MyMovement->RotationRate = FRotator(0.f, 640.f, 0.f);
	MyMovement->bConstrainToPlane = true;
	MyMovement->bSnapToPlaneAtStart = true;
	//
	//// Create a camera boom...
	//CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	//CameraBoom->SetupAttachment(RootComponent);
	//CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	//CameraBoom->TargetArmLength = 800.f;
	//CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	//CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level
	//
	//// Create a camera...
	//TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	//TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	//TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	//// Activate ticking in order to update the cursor every frame.
	//PrimaryActorTick.bCanEverTick = true;
	//PrimaryActorTick.bStartWithTickEnabled = true;

	// Networking
	bReplicates = true;
}

void AMultiplayer_ProjectCharacter::EnterLiveroomBroadcast(int roomNo)
{
	OnEnterLiveroom.Broadcast(this, roomNo);
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

