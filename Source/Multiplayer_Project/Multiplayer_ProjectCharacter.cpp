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

AMultiplayer_ProjectCharacter::AMultiplayer_ProjectCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	MyMovement = GetCharacterMovement();
	MyMovement->bOrientRotationToMovement = true; // Rotate character to moving direction
	MyMovement->RotationRate = FRotator(0.f, 640.f, 0.f);
	MyMovement->bConstrainToPlane = true;
	MyMovement->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Mesh
	MyMesh = GetMesh();

	// Networking
	bReplicates = true;
}

void AMultiplayer_ProjectCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void AMultiplayer_ProjectCharacter::OnRep_HideCharacter()
{
	// replication logic here
	// set mesh to be invisible
	// set entire actor to COND_Custom (only replicate to the custom condition which is the same room)

	UE_LOG(LogTemp, Warning, TEXT("Multiplayer_ProjectCharacter::OnRep_HideCharacter()"));
	// MyMesh->SetVisibility(false);
}

//void AMultiplayer_ProjectCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
//	DOREPLIFETIME_CONDITION_NOTIFY(AMultiplayer_ProjectCharacter, roomNumber, COND_Custom, REPNOTIFY_Always);
//	UE_LOG(LogTemp, Warning, TEXT("Multiplayer_ProjectCharacter::GetLifetimeReplicatedProps()"));
//}

