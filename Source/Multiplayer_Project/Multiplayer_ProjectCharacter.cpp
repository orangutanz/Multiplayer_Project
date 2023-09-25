// Copyright Epic Games, Inc. All Rights Reserved.

#include "Multiplayer_ProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Engine/LevelStreamingDynamic.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"

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


void AMultiplayer_ProjectCharacter::CLIENT_LoadInstancedMap_Implementation(const FString& MapName, FVector Position, const FString& OverrideName)
{
	ULevelStreamingDynamic* UnloadLevel = nullptr;
	if (mInstancedLevel)
	{
		UnloadLevel = mInstancedLevel;
	}
	UWorld* World = GetWorld();
	if (World)
	{
		// Level package name to load, ex: /Game/Maps/MyMapName, 
		// specifying short name like MyMapName will force very slow search on disk
		bool Success = false;
		FTransform transform;
		transform.SetLocation(Position);
		ULevelStreamingDynamic::FLoadLevelInstanceParams params(
			GetWorld(),
			MapName,
			transform
		);
		params.OptionalLevelNameOverride = &OverrideName;
		mInstancedLevel = ULevelStreamingDynamic::LoadLevelInstance(params, Success);
		if (Success)
		{
			//auto i = CharacterMovement.Get();
			mapPosition = Position;
			auto newPosition = mapPosition;
			newPosition.Z += 120;
			mInstancedLevel->OnLevelLoaded.Add(OnLevelLoaded);
			if (UnloadLevel)
			{
				UnloadLevel->SetShouldBeLoaded(false);
				UnloadLevel->SetShouldBeVisible(false);
			}
		}
	}
}

AMultiplayer_ProjectCharacter::~AMultiplayer_ProjectCharacter()
{
	OnLevelLoaded.Unbind();
	if (mInstancedLevel)
	{
		mInstancedLevel->SetShouldBeLoaded(false);
		mInstancedLevel->SetShouldBeVisible(false);
		mInstancedLevel = nullptr;
	}
}

void AMultiplayer_ProjectCharacter::CLIENT_RemoveInstancedMap_Implementation()
{
	if (!mInstancedLevel)
	{
		return;
	}
	// MapCleanUp would on the Server 
	mInstancedLevel->SetShouldBeLoaded(false);
	mInstancedLevel->SetShouldBeVisible(false);
	mInstancedLevel = nullptr;
	SetActorLocationAndRotation(oldPosition, FRotator::ZeroRotator, false, nullptr, ETeleportType::ResetPhysics);
}

void AMultiplayer_ProjectCharacter::EnterInstance(AMultiplayer_ProjectCharacter* actor, int32 instanceNumber, const FString& mapName)
{
	if (!OnEnterInstance.IsBound())
	{
		return;
	}
	OnEnterInstance.Execute(actor, instanceNumber, mapName);
}

void AMultiplayer_ProjectCharacter::LeaveInstance(AMultiplayer_ProjectCharacter* actor, int32 instanceNumber)
{
	if (!OnLeaveInstance.IsBound())
	{
		return;
	}
	OnLeaveInstance.Execute(actor, instanceNumber);
}

void AMultiplayer_ProjectCharacter::ChangeInstance(AMultiplayer_ProjectCharacter* actor, int32 OldNumber, int32 NewNumber, const FString& mapName)
{
	if (!OnChangeInstance.IsBound())
	{
		return;
	}
	OnChangeInstance.Execute(actor, OldNumber, NewNumber, mapName);
}


void AMultiplayer_ProjectCharacter::InstanceTeleport()
{
	auto newPosition = mapPosition;
	newPosition.Z += 120;
	SetActorLocationAndRotation(newPosition, FRotator::ZeroRotator, false, nullptr, ETeleportType::ResetPhysics);
}


void AMultiplayer_ProjectCharacter::InstanceTeleport_Async()
{
	AsyncTask(ENamedThreads::GameThread, [this]() {
		auto newPosition = mapPosition;
		newPosition.Z += 120;
		SetActorLocationAndRotation(newPosition, FRotator::ZeroRotator, false, nullptr, ETeleportType::ResetPhysics);
		auto StreamingLevels = GetWorld()->GetStreamingLevels();
		for (int i = 0; i < StreamingLevels.Num(); i++) {
			auto levelData = StreamingLevels.GetData()[i];

			auto actors = levelData->GetLoadedLevel()->Actors.Num();
		}
	});
}

void AMultiplayer_ProjectCharacter::InstanceTeleportBack()
{
	SetActorLocationAndRotation(oldPosition, FRotator::ZeroRotator, false, nullptr, ETeleportType::ResetPhysics);
}
