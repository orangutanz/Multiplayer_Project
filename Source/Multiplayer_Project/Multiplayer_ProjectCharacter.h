// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Multiplayer_ProjectCharacter.generated.h"

class ULevelStreamingDynamic;

DECLARE_DELEGATE_TwoParams(FOnLeaveInstance, class AMultiplayer_ProjectCharacter*, int32)
DECLARE_DELEGATE_ThreeParams(FOnEnterInstance, class AMultiplayer_ProjectCharacter*, int32, const FString& mapName)
DECLARE_DELEGATE_FourParams(FOnChangeInstance, class AMultiplayer_ProjectCharacter*, int32,int32, const FString& mapName)

UCLASS(Blueprintable)
class AMultiplayer_ProjectCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMultiplayer_ProjectCharacter();
	~AMultiplayer_ProjectCharacter();

	// Call ReplicationGraph to Update Node
	static FOnEnterInstance OnEnterInstance; 
	static FOnLeaveInstance OnLeaveInstance;
	static FOnChangeInstance OnChangeInstance;

	// server call on client
	UFUNCTION(Client, Reliable)
	void CLIENT_LoadInstancedMap(const FString& MapName, FVector Position, const FString& OverrideName);
	UFUNCTION(Client, Reliable)
	void CLIENT_RemoveInstancedMap();

	UFUNCTION()
	void InstanceTeleport();
	UFUNCTION()
	void InstanceTeleport_Async();
	UFUNCTION()
	void InstanceTeleportBack();

	// This code should call the server
	UFUNCTION(BlueprintCallable)
	void EnterInstance(AMultiplayer_ProjectCharacter* actor, int32 instanceNumber, const FString& mapName);
	UFUNCTION(BlueprintCallable)
	void LeaveInstance(AMultiplayer_ProjectCharacter* actor, int32 instanceNumber);
	UFUNCTION(BlueprintCallable)
	void ChangeInstance(AMultiplayer_ProjectCharacter* actor, int32 OldNumber, int32 NewNumber, const FString& mapName);


	TScriptDelegate<FWeakObjectPtr> OnLevelLoaded;

	FVector mapPosition;
	FVector oldPosition = { 0,1800,2200 };

private:
	ULevelStreamingDynamic* mInstancedLevel;

	UPROPERTY(VisibleAnywhere)
	class UCharacterMovementComponent* MyMovement;
};

