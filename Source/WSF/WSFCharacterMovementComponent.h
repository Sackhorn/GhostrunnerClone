// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "WSFCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WSFCharacterMovementComponent.generated.h"

class AWSFCharacter;
enum ECustomMoveMode: uint8 {
	CUSTOM_Wallrun = 0,
	CUSTOM_Dash = 1,
	CUSTOM_SidewaysDash = 2,
	CUSTOM_GrapplingHook = 3,
};

/**
 * 
 */
UCLASS()
class UWSFCharacterMovementComponent : public UCharacterMovementComponent
{
protected:
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
private:
	GENERATED_BODY()

public:
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	virtual bool CanAttemptJump() const override;
	
	virtual bool DoJump(bool bReplayingMoves) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Wallrun)
	float WallrunSpeed = 100.0f;

	virtual void BeginPlay() override
	{
		Super::BeginPlay();
		Character = (AWSFCharacter*)(CharacterOwner);
	};
	UFUNCTION(BlueprintCallable)
	bool IsWallrunning() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Dash)
	float DashDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Dash)
	float DashVelocity = 4000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Dash)
	float DashResidueVelocityPercentage = 0.1f;

private:
	//Why does using static_cast here returns Null or something improper ?
	TWeakObjectPtr<AWSFCharacter> Character;
	float DashRunningTime = 0.0f;
};
