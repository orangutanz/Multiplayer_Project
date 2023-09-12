// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Equipment.h"
#include "Multiplayer_ProjectCharacter.generated.h"

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnNewEquipment, class AMultiplayer_ProjectCharacter*,
	class AEquipment* /* New Equipment */, class AEquipment* /* Old Equipment */)

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnEnterInstance, class AMultiplayer_ProjectCharacter*, int32)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLeaveInstance, class AMultiplayer_ProjectCharacter*, int32)

UCLASS(Blueprintable)
class AMultiplayer_ProjectCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMultiplayer_ProjectCharacter();

	static FOnNewEquipment OnNewEquipment;

	// Call ReplicationGraph to Update Node
	static FOnEnterInstance OnEnterInstance; 
	static FOnLeaveInstance OnLeaveInstance;


	// This code should call the server
	UFUNCTION(BlueprintCallable)
	void EnterInstance(AMultiplayer_ProjectCharacter* actor, int32 instanceNumber);

	UFUNCTION(BlueprintCallable)
	void LeaveInstance(AMultiplayer_ProjectCharacter* actor, int32 instanceNumber);

	UFUNCTION(BlueprintCallable)
	void NewEquipmentBroadcast(AEquipment* NewEquipment);

protected:

public:


private:
	AEquipment* mEquipment = nullptr;

	UPROPERTY(VisibleAnywhere)
	class UCharacterMovementComponent* MyMovement;
};

