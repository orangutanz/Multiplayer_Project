// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Equipment.h"
#include "Multiplayer_ProjectCharacter.generated.h"

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnNewEquipment, class AMultiplayer_ProjectCharacter*,
	class AEquipment* /* New Equipment */, class AEquipment* /* Old Equipment */)

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnEnterLiveroom, class AMultiplayer_ProjectCharacter*, int)


UCLASS(Blueprintable)
class AMultiplayer_ProjectCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMultiplayer_ProjectCharacter();

	static FOnNewEquipment OnNewEquipment;
	static FOnEnterLiveroom OnEnterLiveroom;

	UFUNCTION(BlueprintCallable)
	void EnterLiveroomBroadcast(int roomNo);

	UFUNCTION(BlueprintCallable)
	void NewEquipmentBroadcast(AEquipment* NewEquipment);

protected:

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int roomNumber;	


private:
	AEquipment* mEquipment = nullptr;

	UPROPERTY(VisibleAnywhere)
	class UCharacterMovementComponent* MyMovement;
};

