// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "WSFHUD.h"
#include "GameFramework/Character.h"

#include "WSFCharacter.generated.h"

enum ETraceSide: int
{
	RightTrace = 0,
    LeftTrace = 1,
    DeffectiveTrace = -1
};

class UInputComponent;

UCLASS(config=Game)
class AWSFCharacter : public ACharacter
{
	GENERATED_BODY()

	TWeakObjectPtr<AWSFHUD> PlayerHUD;	
	// static void SwitchCamerasWrapper(IConsoleVariable* Var);
	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* FP_Gun;

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class UStaticMeshComponent* FP_Sword;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	/** Gun mesh: VR view (attached to the VR controller directly, no arm, just the actual gun) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* VR_Gun;

	/** Location on VR gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* VR_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* ThirdPersonCameraComponent;

	/** Motion controller (right hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* R_MotionController;

	/** Motion controller (left hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* L_MotionController;

	UPROPERTY(VisibleAnywhere)
	class UCapsuleComponent* ReceiveHitCapsule;
	
	

public:
	AWSFCharacter(const FObjectInitializer& ObjectInitializer);
	void SwitchToDebugCamera(IConsoleVariable* Var);
	

protected:
	virtual void BeginPlay();
	virtual void Tick(float DeltaSeconds) override;

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AWSFProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

	/** Whether to use motion controller location for aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	uint32 bUsingMotionControllers : 1;

protected:
	
	/** Fires a projectile. */
	void OnFire();

	/** Resets HMD orientation and position in VR. */
	void OnResetVR();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	void TryPerformAttack();
	// End of APawn interface

	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

public:

	//////////////////////////////////////////////////////////////
	/*                       WALLRUN                           */
	bool hasJumpedOff, bIsWallrunAvailable, bIsWallrunDisabledTimeout, bIsRightSideWallrun;
	FTimerHandle WallrunDisablerTimer;
	TWeakObjectPtr<AActor> CurrWallRunningOn;
	TWeakObjectPtr<AActor> PrevWallRunningOn;
	FVector HitNormal, HitPoint, WallrunDirection;
	
	/** How long is the vector that looks for walls to run on*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Wallrun)
	float WallrunTraceLength = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Wallrun)
	float BackwardWallrunTraceLength = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Wallrun)
	float ForwardWallrunTraceLength = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Wallrun)
	float BackwardTracingVectorsRotationAngle=30.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Wallrun)
	float ForwardTracingVectorsRotationAngle=30.0f;

	/** The Force that we apply in direction of normal vector when jumping off the wall*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Wallrun)
	float JumpOffImpulse=40000.0f;

	/** How long after Jumping off the wall you cannot wallrun*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Wallrun)
	float WallrunTimeoutDuration = 0.7f;

	/** How long will it take to rotate to final Yaw rotation*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Wallrun)
	float YawRotationTime = 0.2;
	
	/** How long will it take to rotate to final Roll rotation*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Wallrun)
	float RollRotationTime = 0.2;

	/** The curve that determines character rotation dynamic when starting a wallrun*/
	UPROPERTY(EditAnywhere, Category=Wallrun)
	UCurveFloat* RotationCurve;

	/** We will not rotate character if difference between CharacterForwardVector and WallrunDirection is less that this (In degrees)*/
	UPROPERTY(EditAnywhere, Category=Wallrun)
	float RotationAngleDeadzone = 15.0f;

	/** This determines whether we add a Vector in Direction We are looking to the Jump Off vector*/
	UPROPERTY(EditAnywhere, Category=Wallrun)
	bool bAddLookingAtVectorToJumpOff;

	/** Camera Rotation roll we apply when doing a wallrun (In degrees) */
	UPROPERTY(EditAnywhere, Category=Wallrun)
	float WallrunCameraRoll = 15.0f;

	UPROPERTY(EditAnywhere, Category=Wallrun)
	bool bInverseMeshWhenWallruning = false;

	UPROPERTY(EditAnywhere, Category=Wallrun)
	float JumpOffDirectionDeadzone = 10.0f;

	//////////////////////////////////////////////////////////////
	/*                       DASH                               */
	bool bIsDashDisabledTimeout;
	FTimerHandle DashDisablerTimer;
	
	/** What force to apply for the Dash*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Dash)
	float DashImpulse=40000.0f;

	/** For how much we disable dash after dashing*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Dash)
	float DashTimeout = 0.5f;

	/** TODO: Implement time slowing down around us when we dash */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Dash)
	float DashTimeDilation;

	/** Here we handle going in and out of Dash and Wallrun Custom Movement Modes */
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	
	////////////////////////////////////////////////////////////////////////
    /*                       SIDEWAYS DASH                               */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SidewaysDash)
	float SidewaysDashMaxTime = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SidewaysDash)
	float SidewaysDashTimeDilitation = 0.5;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SidewaysDash)
	float SidewayDashMoveSpeed = 0.5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SidewaysDash)
	float DebounceTime = 0.1;
	
	////////////////////////////////////////////////////////////////////////
	/*                       GRAPPLING HOOK                              */
	bool bIsGrapplingHookAvailable;
	FVector GrapplingHookVelocity;
	void BeginGrapplingHook();
	void CleanGameplayKeyBindings();


	float GravityCoefficient;

	////////////////////////////////////////////////////////////////////////
	/*                       SWORD ATTACK                              */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SwordAttack)
	float ForwardSphereOffset = 100.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SwordAttack)
	float LeftSphereOffset = 100.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SwordAttack)
	float RightSphereOffset = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SwordAttack)
	float SphereRadius = 100.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SwordAttack)
	float DeathTimeDilitation = 0.02;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=SwordAttack)
	bool bIsSwordAttackMontage;
	
	
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	void UpdateGrapplingHookIndicator(FVector2D Position, bool Visibility);
private:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	           int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	//////////////////////////////////////////////////////////////
	/*                       DASH                               */
	void Dash();
		
	UFUNCTION()
	void RenableDash()
	{
		bIsDashDisabledTimeout = false;
	}
	
	
	//////////////////////////////////////////////////////////////
	/*                       SIDEWAYS DASH                     */
	void BeginSidewaysDash();
	void PerformSidewaysDash();
	void EndSidewayDash();
	FTimerHandle SidewayDashTimer;
	FTimerHandle DebounceDashTimer;
	
	
	/////////////////////////////////////	
	/*           WALLRUN             */
	float YawRotationProgress = 0.0f;
	float RollRotationProgress = 0.0f;
	FVector OriginalForwardVector, OriginalDirectionVector;
	FRotator OriginalMeshRotation;
	float OriginalRollRotation;
	void BeginWallrunRotation(bool UpdateOnlyYaw=false);
	void UpdateWallrunRotation(float DeltaSeconds);
	void BeginWallrun();
	void EndWallrun();
	FVector PlayerAddedJumpOffDirection();
	IConsoleVariable* CVarVisualizeWallrunVectors; 
	IConsoleVariable* CVarUseDebugCamera;
	IConsoleVariable* CVarVisualizeSword;
	TPair<ETraceSide, FHitResult> PerformTraces(const TArray<TPair<ETraceSide, FVector>>& TracingVectors);
	void SetupTracingVectors(TArray<TPair<ETraceSide, FVector>>* TracingVectors);
	bool WallrunCheckAndTick();
	void DebugDrawWallrunVectors(const FHitResult& HitResult, TArray<TPair<ETraceSide, FVector>> TracingVectors);
	void PerformTraceWallCheck(const FVector& ActorLocation, const FVector& TraceEndpoint, FHitResult* HitResult);
	bool HitBoxFrame(const FHitResult& Hit);
	UFUNCTION()
    void WallrunDisableTimeout()
	{
		bIsWallrunDisabledTimeout = false;
	}
	
};

