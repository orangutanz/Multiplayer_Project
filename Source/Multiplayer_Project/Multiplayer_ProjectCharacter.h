// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Equipment.h"
#include "Multiplayer_ProjectCharacter.generated.h"

DECLARE_DELEGATE_TwoParams(FOnEnterInstance, class AMultiplayer_ProjectCharacter*, int32)
DECLARE_DELEGATE_TwoParams(FOnLeaveInstance, class AMultiplayer_ProjectCharacter*, int32)
DECLARE_DELEGATE_ThreeParams(FOnChangeInstance, class AMultiplayer_ProjectCharacter*, int32,int32)

UCLASS(Blueprintable)
class AMultiplayer_ProjectCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMultiplayer_ProjectCharacter();


	// Call ReplicationGraph to Update Node
	static FOnEnterInstance OnEnterInstance; 
	static FOnLeaveInstance OnLeaveInstance;
	static FOnChangeInstance OnChangeInstance;


	// This code should call the server
	UFUNCTION(BlueprintCallable)
	void EnterInstance(AMultiplayer_ProjectCharacter* actor, int32 instanceNumber);

	UFUNCTION(BlueprintCallable)
	void LeaveInstance(AMultiplayer_ProjectCharacter* actor, int32 instanceNumber);

	UFUNCTION(BlueprintCallable)
	void ChangeInstance(AMultiplayer_ProjectCharacter* actor, int32 OldNumber, int32 NewNumber);

protected:


public:


private:
	AEquipment* mEquipment = nullptr;

	UPROPERTY(VisibleAnywhere)
	class UCharacterMovementComponent* MyMovement;
};

