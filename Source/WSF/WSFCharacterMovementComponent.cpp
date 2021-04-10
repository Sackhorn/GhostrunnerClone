// Fill out your copyright notice in the Description page of Project Settings.


#include "WSFCharacterMovementComponent.h"


#include "DrawDebugHelpers.h"
#include "WSFCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"


void UWSFCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	if(MovementMode == MOVE_Custom)
	{
		Acceleration = FVector::ZeroVector;
		VerticalVelocity = FVector::ZeroVector;
	}
	if(MovementMode == MOVE_Custom && CustomMovementMode == CUSTOM_GrapplingHook)
	{
		Velocity = Character->GrapplingHookVelocity;
	}
}

void UWSFCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	if(CustomMovementMode == CUSTOM_Wallrun)
	{
		//Resolve Wallrun
		if (!Character->bIsWallrunAvailable || Character->bPressedJump)
		{
			SetMovementMode(MOVE_Falling);
			return;
		}
		Velocity = Character->WallrunDirection * WallrunSpeed;
		float DistanceFromWall = FVector::PointPlaneDist(Character->GetActorLocation(), Character->HitPoint, Character->HitNormal);
		if(DistanceFromWall > 1.2f * Character->GetCapsuleComponent()->GetScaledCapsuleRadius())
		{
			FVector WallToCharacterDirection = Character->HitPoint - Character->GetActorLocation();
			WallToCharacterDirection.Normalize();
			Velocity += WallToCharacterDirection * 100.0f;			
		}
		
	}
	else if(CustomMovementMode == CUSTOM_Dash)
	{
		//Resolve Dashing process
		FVector DashMovementDirection = Character->GetFirstPersonCameraComponent()->GetForwardVector();
		if (DashRunningTime > DashDuration)
		{
			Velocity = DashMovementDirection * DashVelocity * DashResidueVelocityPercentage;
			SetMovementMode(MOVE_Falling);
			DashRunningTime = 0.0f;
			return;
		}
		DashRunningTime += deltaTime;
		Velocity = DashMovementDirection * DashVelocity;
	}
	else if(CustomMovementMode == CUSTOM_SidewaysDash)
	{
		float projectionLenght = FVector::DotProduct(Acceleration, Character->GetActorRightVector());
		FVector SidewaysDirection = projectionLenght * Character->GetActorRightVector();
		Velocity = SidewaysDirection * Character->SidewayDashMoveSpeed;
		Acceleration = FVector(0.0f, 0.0f, GetGravityZ());
		VerticalVelocity += Acceleration * deltaTime;
		Velocity += VerticalVelocity;
	}
	else if(CustomMovementMode == CUSTOM_GrapplingHook)
	{
		Acceleration = FVector(0.0f, 0.0f, Character->GravityCoefficient);
		Velocity += Acceleration * deltaTime;
	}

	// This part of movement is common
	float SimulationTimeRemaining = deltaTime;
	FHitResult Hit;
	while (SimulationTimeRemaining >= MIN_TICK_TIME && Iterations < MaxSimulationIterations)
	{
		Iterations++;
		float timeStep = GetSimulationTimeStep(SimulationTimeRemaining, Iterations);
		SimulationTimeRemaining -= timeStep;
		//W don't want to clip over wall when dashing but we will allow it in wallrun to avoid getting stuck in some wall corner
		SafeMoveUpdatedComponent(Velocity * timeStep, UpdatedComponent->GetComponentQuat(), CustomMovementMode > CUSTOM_Wallrun, Hit, ETeleportType::TeleportPhysics);
		if (Hit.IsValidBlockingHit() &&
			(Hit.Actor.Get() != Character->CurrWallRunningOn.Get() || CustomMovementMode==CUSTOM_GrapplingHook))
		{
			SetMovementMode(MOVE_Falling);
			return;
		}
	}
}

bool UWSFCharacterMovementComponent::CanAttemptJump() const
{
	return IsJumpAllowed() &&
           !bWantsToCrouch &&
           (IsMovingOnGround() || IsFalling() || IsWallrunning());
}

bool UWSFCharacterMovementComponent::DoJump(bool bReplayingMoves)
{
	return Super::DoJump(bReplayingMoves);
}

bool UWSFCharacterMovementComponent::IsWallrunning() const
{
	return MovementMode == MOVE_Custom &&
		CustomMovementMode == CUSTOM_Wallrun &&
		UpdatedComponent;
}