// Copyright Epic Games, Inc. All Rights Reserved.

#include "WSFCharacter.h"

#include "DrawDebugHelpers.h"
#include "EnemyBaseCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "WSF.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "GameFramework/CharacterMovementComponent.h"
#include "WSFCharacterMovementComponent.h"
#include "WSFHUD.h"
#include "Components/BoxComponent.h"
#include "Components/Image.h"
#include "UMG/Public/Blueprint/UserWidget.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);


//////////////////////////////////////////////////////////////////////////
// AWSFCharacter

void SwitchCamerasWrapper(IConsoleVariable* Var)
{
	AWSFCharacter* Character = Cast<AWSFCharacter>(GWorld->GetFirstPlayerController()->GetCharacter());
	Character->SwitchToDebugCamera(Var);
}

void AWSFCharacter::SwitchToDebugCamera(IConsoleVariable* Var)
{
	bool bIsDebug = Var->GetBool();
	if(bIsDebug)
	{
		ThirdPersonCameraComponent->Activate();
		FirstPersonCameraComponent->Deactivate();
	}
	else
	{
		ThirdPersonCameraComponent->Deactivate();
		FirstPersonCameraComponent->Activate();
	}
}

AWSFCharacter::AWSFCharacter(const FObjectInitializer& ObjectInitializer):
Super(ObjectInitializer.SetDefaultSubobjectClass<UWSFCharacterMovementComponent>(CharacterMovementComponentName))
{

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	ReceiveHitCapsule = CreateDefaultSubobject<UCapsuleComponent>("ReceiveHitCapsule");
	ReceiveHitCapsule->SetupAttachment(RootComponent);
	ReceiveHitCapsule->InitCapsuleSize(130.0f, 80.0f);

	
	
	

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	ThirdPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	ThirdPersonCameraComponent->SetRelativeLocation(FVector(-500.0f, 0.0f, 500.0f)); // Position the camera
	ThirdPersonCameraComponent->bUsePawnControlRotation = true;

	// Generate Console Variables
	CVarVisualizeWallrunVectors = IConsoleManager::Get().RegisterConsoleVariable(
        TEXT("VisualizeWallrun"),
        false,
        TEXT("Shows Wallrun vectors \n"),
        ECVF_Scalability | ECVF_RenderThreadSafe);

	CVarVisualizeSword = IConsoleManager::Get().RegisterConsoleVariable(
        TEXT("VisualizeSword"),
        false,
        TEXT("Shows Sweep colliders for the sword attack \n"),
        ECVF_Scalability | ECVF_RenderThreadSafe);

	CVarUseDebugCamera = IConsoleManager::Get().RegisterConsoleVariable(
		TEXT("UseDebugCamera"),
		false,
		TEXT("Uses Character Debug Camera \n"),
		ECVF_Scalability | ECVF_RenderThreadSafe);
	CVarUseDebugCamera->AsVariable()->SetOnChangedCallback(FConsoleVariableDelegate::CreateStatic(&SwitchCamerasWrapper));
	
	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	HookEmmiter = CreateDefaultSubobject<UNiagaraComponent>(TEXT("HookEmmiter"));
	HookEmmiter->SetupAttachment(Mesh1P);
	HookEmmiter->Deactivate();

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_Sword = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FP_Sword"));
	FP_Sword->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Sword->bCastDynamicShadow = false;
	FP_Sword->CastShadow = false;
	// FP_Sword->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Sword->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	
	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;
}

void AWSFCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	PlayerHUD = Cast<AWSFHUD>(Cast<APlayerController>(Controller)->GetHUD());
	ReceiveHitCapsule->OnComponentBeginOverlap.AddDynamic(this, &AWSFCharacter::OnHit);
	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	// FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	// FP_Sword->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	// FP_Sword->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true), TEXT("GripPoint"));
	FP_Sword->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AWSFCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);
	
	FInputActionBinding ResetPressed("Reset", IE_Pressed);
	ResetPressed.ActionDelegate.GetDelegateForManualSet().BindLambda([this]()
	{
		UGameplayStatics::OpenLevel(GetWorld(), "TutorialMap");
	});
	PlayerInputComponent->AddActionBinding(ResetPressed);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AWSFCharacter::OnFire);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AWSFCharacter::OnResetVR);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AWSFCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AWSFCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &AWSFCharacter::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AWSFCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &AWSFCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AWSFCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &AWSFCharacter::BeginSidewaysDash);
	PlayerInputComponent->BindAction("Dash", IE_Released, this, &AWSFCharacter::EndSidewayDash);

	PlayerInputComponent->BindAction("GrapplingHook", IE_Pressed, this, &AWSFCharacter::OnGrapplingHook);
}

void AWSFCharacter::TryPerformAttack()
{
	if(!bIsSwordAttackMontage)
	{
		return;
	}
	TArray<FHitResult> HitResults;
	FVector Forward = FirstPersonCameraComponent->GetForwardVector();
	FVector Right = FirstPersonCameraComponent->GetRightVector();
	
	FVector Start = GetActorLocation() + Forward * ForwardSphereOffset + Right * -LeftSphereOffset;
	FVector End = Start + GetActorRightVector() * RightSphereOffset;
	if(CVarVisualizeSword->GetBool())
	{
		DrawDebugSphere(GetWorld(), Start, SphereRadius, 50, FColor::Red, false, 0.5f);
		DrawDebugSphere(GetWorld(), End, SphereRadius, 50, FColor::Red, false, 0.5f);
	}
	FCollisionShape ColShape = FCollisionShape::MakeSphere(SphereRadius);
	bool bHitSword = GetWorld()->SweepMultiByObjectType(HitResults,
        Start,
        End,
        FQuat::Identity,
        FCollisionObjectQueryParams(ECC_Pawn),
        ColShape);
	for(FHitResult Hit : HitResults)
	{
		auto Enemy = Cast<AEnemyBaseCharacter>(Hit.Actor);
		if(Enemy)
		{
			FVector HitDirection = GetActorLocation() - Enemy->GetActorLocation();
			HitDirection.Normalize();
			Enemy->OnDeath(HitDirection);
		}
	}
}

void AWSFCharacter::OnFire()
{
	// try and play the sound if specified
	// if (FireSound != NULL)
	// {
		// UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	// }

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		UAnimMontage* Montage = AnimInstance->GetCurrentActiveMontage();
		bool isTheSame = Montage == FireAnimation;
		if (AnimInstance != NULL && !isTheSame)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void AWSFCharacter::OnGrapplingHook()
{
	if (GrapplingMontage != NULL && bIsGrapplingHookAvailable)
	{
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		UAnimMontage* Montage = AnimInstance->GetCurrentActiveMontage();
		bool isTheSame = Montage == GrapplingMontage;
		if (AnimInstance != NULL && !isTheSame)
		{
			AnimInstance->Montage_Play(GrapplingMontage, 1.f);
		}
	}
}

void AWSFCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AWSFCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AWSFCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

void AWSFCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AWSFCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AWSFCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AWSFCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AWSFCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AWSFCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AWSFCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AWSFCharacter::TouchUpdate);
		return true;
	}
	
	return false;
}

////////////////////////////////////////////////////////////////////////////
/// Wallrun

void AWSFCharacter::Tick(float DeltaSeconds)
{
	CharacterAditionalTicks.Broadcast();
	TryPerformAttack();
	bIsWallrunAvailable = WallrunCheckAndTick() && !bIsWallrunDisabledTimeout;
	TEnumAsByte<EMovementMode> MovementMode =  GetCharacterMovement()->MovementMode;
	uint8 CustomMovementMode = GetCharacterMovement()->CustomMovementMode;
	if(bIsWallrunAvailable &&
		(MovementMode == MOVE_Falling ||
			(MovementMode == MOVE_Custom && CustomMovementMode == CUSTOM_Dash) ||
			MovementMode==MOVE_Custom && CustomMovementMode== CUSTOM_GrapplingHook))
	{
		if(PrevWallRunningOn != CurrWallRunningOn)
		{
			BeginWallrun();
		}
	}
	if(MovementMode==MOVE_Custom && GetCharacterMovement()->CustomMovementMode == CUSTOM_Wallrun)
	{
		UpdateWallrunRotation(DeltaSeconds);
	}
	PlayerHUD->UpdateDashIndicator(SidewayDashTimer, DashDisablerTimer);
}

void AWSFCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	
	TEnumAsByte<EMovementMode> CurrMovementMode = GetCharacterMovement()->MovementMode;
	uint8 CurrCustomMode = GetCharacterMovement()->CustomMovementMode;
	if(PrevMovementMode==MOVE_Custom && PreviousCustomMode==CUSTOM_Wallrun)
	{
		EndWallrun();
	}
	else if(CurrMovementMode == MOVE_Custom &&  CurrCustomMode == CUSTOM_Wallrun)
	{
		BeginWallrunRotation();
	}
	// else if(PrevMovementMode == MOVE_Custom && PreviousCustomMode == CUSTOM_Dash)
	// {
		// EndDash();
	// }
	// else if(PrevMovementMode == MOVE_Custom && PreviousCustomMode == CUSTOM_SidewaysDash)
	// {
		// EndSidewayDash();
	// }
}

void AWSFCharacter::PerformTraceWallCheck(const FVector& ActorLocation, const FVector& TraceEndpoint, FHitResult* HitResult)
{
	GetWorld()->LineTraceSingleByChannel(*HitResult, ActorLocation, TraceEndpoint, ECC_Wallrun);
}

void AWSFCharacter::DebugDrawWallrunVectors(const FHitResult& HitResult, TArray<TPair<ETraceSide, FVector>> TracingVectors)
{
	if(!CVarVisualizeWallrunVectors->GetBool())
	{
		return;
	}
	FVector ActorLocation = GetActorLocation();
	UWorld* World = GetWorld();
	for(auto& Elem: TracingVectors)
	{
		DrawDebugDirectionalArrow(GetWorld(), ActorLocation, Elem.Value, 10.0f, FColor::Red, false, -1, 0, 3.0f);
	}
	if(HitResult.IsValidBlockingHit())
	{
		GEngine->AddOnScreenDebugMessage(WALLRUN_MESSAGE_KEY, 2.0f, FColor::Blue, HitResult.Actor->GetName());
		DrawDebugDirectionalArrow(World, HitPoint,ActorLocation, 10.0f, FColor::Blue, false, -1, 0, 3.0f);
		DrawDebugDirectionalArrow(World, HitPoint, HitPoint + HitNormal * WallrunTraceLength, 10.0f, FColor::Green, false, -1, 0, 3.0f);
		DrawDebugDirectionalArrow(World, ActorLocation, ActorLocation + WallrunDirection * 100.0f, 10.0f, FColor::Magenta, false, -1, 0, 3.0f);
	}
}

void AWSFCharacter::SetupTracingVectors(TArray<TPair<ETraceSide, FVector>>* TracingVectors)
{
	FVector ActorRightVector = GetActorRightVector();
	FVector ActorLocation = GetActorLocation();
	FVector RightTraceEndPoint = ActorLocation + (ActorRightVector * WallrunTraceLength); //We should be checking for a closest point within range, not for a tracer (we can be rotated a bit and catch a normal from ahead of us)
	FVector LeftTraceEndPoint = ActorLocation + (-ActorRightVector * WallrunTraceLength);
	TracingVectors->Emplace(LeftTrace, LeftTraceEndPoint);
	TracingVectors->Emplace(LeftTrace, ActorLocation + (-ActorRightVector * ForwardWallrunTraceLength).RotateAngleAxis(ForwardTracingVectorsRotationAngle, GetActorUpVector()));
	TracingVectors->Emplace(LeftTrace, ActorLocation + (-ActorRightVector * BackwardWallrunTraceLength).RotateAngleAxis(-BackwardTracingVectorsRotationAngle, GetActorUpVector()));
	TracingVectors->Emplace(RightTrace, RightTraceEndPoint);
	TracingVectors->Emplace(RightTrace, ActorLocation + (ActorRightVector * BackwardWallrunTraceLength).RotateAngleAxis(BackwardTracingVectorsRotationAngle, GetActorUpVector()));
	TracingVectors->Emplace(RightTrace, ActorLocation + (ActorRightVector * ForwardWallrunTraceLength).RotateAngleAxis(-ForwardTracingVectorsRotationAngle, GetActorUpVector()));;
}

bool AWSFCharacter::WallrunCheckAndTick()
{
	TArray<TPair<ETraceSide, FVector>> TracingVectors;
	SetupTracingVectors(&TracingVectors);
	TPair<ETraceSide, FHitResult> FinalHit = PerformTraces(TracingVectors);
	bool bIsAnyHit = FinalHit.Key != DeffectiveTrace;
	TEnumAsByte<EMovementMode> MovementMode =  GetCharacterMovement()->MovementMode;
	uint8 CustomMovementMode = GetCharacterMovement()->CustomMovementMode;
	if(bIsAnyHit && (MovementMode == MOVE_Falling || MovementMode == MOVE_Custom))
	{
		bIsRightSideWallrun = FinalHit.Key == RightTrace;
		FVector ActorUpVector = bIsRightSideWallrun ? GetActorUpVector() : -GetActorUpVector();
		HitPoint = FinalHit.Value.ImpactPoint;
		HitNormal = FinalHit.Value.ImpactNormal;
		PrevWallRunningOn = CurrWallRunningOn;
		CurrWallRunningOn = FinalHit.Value.Actor;
		WallrunDirection = FVector::CrossProduct(ActorUpVector, HitNormal);
		WallrunDirection.Normalize();
	}
	else
	{
		CurrWallRunningOn = NULL;
	}
	DebugDrawWallrunVectors(FinalHit.Value, TracingVectors);
	return bIsAnyHit;
}

TPair<ETraceSide, FHitResult> AWSFCharacter::PerformTraces(const TArray<TPair<ETraceSide, FVector>>& TracingVectors)
{
		TArray<TPair<ETraceSide, FHitResult>> Hits;
		UWSFCharacterMovementComponent* MC = Cast<UWSFCharacterMovementComponent>(GetMovementComponent());
		ETraceSide MovementSide = bIsRightSideWallrun ? RightTrace : LeftTrace;
		for(auto& Trace : TracingVectors)
		{
			//This is needed so we don't suddenly start wallruning on the different side
			if(MC->MovementMode == MOVE_Custom &&
			   MC->CustomMovementMode == CUSTOM_Wallrun &&
			   Trace.Key != MovementSide)
			{
				continue; 
			}
			FHitResult Hit;
			PerformTraceWallCheck(GetActorLocation(), Trace.Value, &Hit);
			if(Hit.IsValidBlockingHit() && !HitBoxFrame(Hit))
			{
				Hits.Emplace(Trace.Key, Hit);
			}
		}
	Hits.Sort([](TPair<ETraceSide, FHitResult> HitA, TPair<ETraceSide, FHitResult> HitB)
	{
		return HitA.Value.Distance < HitB.Value.Distance;
	});
	if(Hits.Num() == 0)
	{
		return TPair<ETraceSide, FHitResult>(DeffectiveTrace, FHitResult());
	}
	return Hits[0];
}

bool AWSFCharacter::HitBoxFrame(const FHitResult& Hit)
{
	FVector Impact = Hit.ImpactPoint;
	UBoxComponent* BoundBox = Cast<UBoxComponent>(Hit.Component);
	if(!BoundBox)
	{
		GEngine->AddOnScreenDebugMessage(WALLRUN_COLLISION_MESSAGE_KEY,2.0f,
			FColor::Blue, TEXT("Hit NON-BOX Collider of Actor") + Hit.Actor->GetName());	
		return true;
	}
	FVector BoxLocation = BoundBox->GetComponentLocation();
	FRotator BoxRotation = BoundBox->GetComponentRotation();
	FVector ScaledBoxExtent = BoundBox->GetScaledBoxExtent();
	FVector RotatedExtent = BoxRotation.RotateVector(ScaledBoxExtent);
	float MaxDifference = 0.2;
	return FMath::Abs(Impact.X - (BoxLocation.X + RotatedExtent.X)) < MaxDifference||
	FMath::Abs(Impact.X - (BoxLocation.X - RotatedExtent.X)) < MaxDifference||
	FMath::Abs(Impact.Y - (BoxLocation.Y + RotatedExtent.Y)) < MaxDifference||
	FMath::Abs(Impact.Y - (BoxLocation.Y - RotatedExtent.Y)) < MaxDifference||
	FMath::Abs(Impact.Z - (BoxLocation.Z + RotatedExtent.Z)) < MaxDifference||
    FMath::Abs(Impact.Z - (BoxLocation.Z - RotatedExtent.Z)) < MaxDifference;
}

void AWSFCharacter::BeginWallrun()
{
	UWSFCharacterMovementComponent* MovementComponent = static_cast<UWSFCharacterMovementComponent*>(GetMovementComponent());
	MovementComponent->SetJumpAllowed(true);
	MovementComponent->SetMovementMode(MOVE_Custom, CUSTOM_Wallrun);
	//This is a dirty fucking hack cause my lazy ass didn't want to do a second animation
	if(bInverseMeshWhenWallruning)
	{
		Mesh1P->SetRelativeScale3D(bIsRightSideWallrun ? FVector(1.0f, -1.0f, 1.0f): Mesh1P->GetRelativeScale3D() );
		OriginalMeshRotation = Mesh1P->GetRelativeRotation();
		FRotator InversedRotation  = FRotator(-OriginalMeshRotation.Pitch, -OriginalMeshRotation.Yaw, -OriginalMeshRotation.Roll);
		Mesh1P->SetRelativeRotation(bIsRightSideWallrun ? InversedRotation : OriginalMeshRotation );
	}
}

void AWSFCharacter::EndWallrun()
{
	CurrWallRunningOn = NULL;
	FRotator CurrentRotation = GetController()->GetControlRotation();
	CurrentRotation.Roll = 0.0f;
	GetController()->SetControlRotation(CurrentRotation);
	if(bInverseMeshWhenWallruning)
	{
		Mesh1P->SetRelativeScale3D(FVector(1.0f));
		Mesh1P->SetRelativeRotation(OriginalMeshRotation);
	}
	//TODO: Do we need timeout ?
	bIsWallrunDisabledTimeout = true;
	GetWorldTimerManager().SetTimer(WallrunDisablerTimer, this, &AWSFCharacter::WallrunDisableTimeout, WallrunTimeoutDuration, false);
	if(bPressedJump)
	{
		FVector JumpOffDirection = HitNormal;
		JumpOffDirection += PlayerAddedJumpOffDirection();
		GetCharacterMovement()->AddImpulse(JumpOffDirection * JumpOffImpulse);
	}
}

FVector AWSFCharacter::PlayerAddedJumpOffDirection()
{
	bool bIsJumpOffDirectionAffected = FVector::DotProduct(GetActorForwardVector(), WallrunDirection) > FMath::Cos(FMath::DegreesToRadians(JumpOffDirectionDeadzone));
	return bIsJumpOffDirectionAffected ? GetActorForwardVector() : FVector::ZeroVector;
}

//TODO: Stop doing that when we move the mouse more than some predefined value
void AWSFCharacter::BeginWallrunRotation(bool UpdateOnlyYaw)
{
	if(!UpdateOnlyYaw)
	{
		RollRotationProgress = 0.0f;
		OriginalRollRotation = GetController()->GetControlRotation().Roll;
	}
	YawRotationProgress = 0.0f;
	OriginalDirectionVector = WallrunDirection;
	OriginalForwardVector = GetActorForwardVector();
	if(FVector::DotProduct(OriginalDirectionVector, OriginalForwardVector) > FMath::Cos(FMath::DegreesToRadians(RotationAngleDeadzone)))
	{
		// Here we setup time in a way that makes sure that UpdateRotation will not execute rotation
		YawRotationProgress = 2*YawRotationTime;
	}
}

// TODO: This should be speed based not time based
void AWSFCharacter::UpdateWallrunRotation(float DeltaSeconds)
{
	if(PrevWallRunningOn != CurrWallRunningOn)
	{
		BeginWallrunRotation(true);
	}
	YawRotationProgress += DeltaSeconds;
	RollRotationProgress += DeltaSeconds;
	float Alpha = YawRotationProgress/YawRotationTime;
	float Beta = RollRotationProgress/RollRotationTime;
	if(Alpha < 1.0f)
	{
		float YawCurvePoint = RotationCurve->GetFloatValue(Alpha);
		FRotator Rotation = FMath::Lerp(OriginalForwardVector.Rotation(), OriginalDirectionVector.Rotation(), YawCurvePoint);
		GetController()->SetControlRotation(Rotation);
	}
	if(Beta < 1.0f)
	{
		float RollCurvePoint = RotationCurve->GetFloatValue(Beta);
		float FinalRotation = OriginalRollRotation + bIsRightSideWallrun ? -WallrunCameraRoll : WallrunCameraRoll;
		FRotator CurrentRotation = GetController()->GetControlRotation();
		CurrentRotation.Roll = FMath::Lerp(OriginalRollRotation, FinalRotation, RollCurvePoint);
		GetController()->SetControlRotation(CurrentRotation);
	}
}

////////////////////////////////////////////////////////////////////////////
/// DASH

//TODO: Slight player sway and rotation
void AWSFCharacter::BeginSidewaysDash()
{
	if(bIsDashDisabledTimeout)
	{
		return;
	}
	bIsDashDisabledTimeout=true;
	ensureAlwaysMsgf(SidewaysDashMaxTime >= DebounceTime,
		TEXT("DebounceTime is Higher than SidewaysDashMaxTime this will cause issues with PlayerCharacter"));
	GetWorldTimerManager().SetTimer(SidewayDashTimer, this, &AWSFCharacter::EndSidewayDash, SidewaysDashMaxTime, false);
	GetWorldTimerManager().SetTimer(DebounceDashTimer, this, &AWSFCharacter::PerformSidewaysDash, DebounceTime, false);
}

void AWSFCharacter::PerformSidewaysDash()
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), SidewaysDashTimeDilitation);
	this->CustomTimeDilation = 1.0f;
	UWSFCharacterMovementComponent* MovementComponent = static_cast<UWSFCharacterMovementComponent*>(GetMovementComponent());
	MovementComponent->SetMovementMode(MOVE_Custom, CUSTOM_SidewaysDash);
}

void AWSFCharacter::EndSidewayDash()
{
	if(GetWorldTimerManager().IsTimerActive(SidewayDashTimer))
	{
		GetWorldTimerManager().ClearTimer(DebounceDashTimer);
		GetWorldTimerManager().ClearTimer(SidewayDashTimer);
		UWSFCharacterMovementComponent* MovementComponent = static_cast<UWSFCharacterMovementComponent*>(GetMovementComponent());
		//We need this because player may release Dash (Shift) key before timer runs out
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
		Dash();
	}
}

void AWSFCharacter::Dash()
{
	GetWorldTimerManager().ClearTimer(SidewayDashTimer);
	UWSFCharacterMovementComponent* MovementComponent = static_cast<UWSFCharacterMovementComponent*>(GetMovementComponent());
	MovementComponent->SetMovementMode(MOVE_Custom, CUSTOM_Dash);
	GetWorldTimerManager().SetTimer(DashDisablerTimer, this, &AWSFCharacter::RenableDash, DashTimeout+MovementComponent->DashDuration, false);
}

////////////////////////////////////////////////////////////////////////////
/// GRAPPLING HOOK

void AWSFCharacter::BeginGrapplingHook()
{
		HookEmmiter->Deactivate();
		CharacterAditionalTicks.Clear();	
		UWSFCharacterMovementComponent* MovementComponent = static_cast<UWSFCharacterMovementComponent*>(GetMovementComponent());
		MovementComponent->SetMovementMode(MOVE_Custom, CUSTOM_GrapplingHook);
}

void AWSFCharacter::CleanGameplayKeyBindings()
{
	for(int i = 0; i < InputComponent->GetNumActionBindings(); i++)
	{
		InputComponent->RemoveActionBinding(i);
	}
	FInputActionBinding ResetPressed("Reset", IE_Pressed);
	ResetPressed.ActionDelegate.GetDelegateForManualSet().BindLambda([this]()
	{
		UGameplayStatics::OpenLevel(GetWorld(), "FirstPersonExampleMap");
	});
	InputComponent->AddActionBinding(ResetPressed);
}

void AWSFCharacter::OnHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	PlayerHUD->OnDead();
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), DeathTimeDilitation);
	CleanGameplayKeyBindings();
}

void AWSFCharacter::UpdateGrapplingEmitter()
{
	FVector SocketLocation = Mesh1P->GetSocketLocation("grappling_socket");
	HookEmmiter->SetWorldLocation(SocketLocation);
	auto Rotation = UKismetMathLibrary::FindLookAtRotation(SocketLocation, GrapplingHookLocation);
	HookEmmiter->SetWorldRotation(Rotation);
	float Dist = FVector::Distance(SocketLocation, GrapplingHookLocation);
	HookEmmiter->SetVectorParameter("User.BeamEnd", FVector(Dist, 0.0f, 0.0f));
}

void AWSFCharacter::PreBeginGrapplingHook()
{
	UpdateGrapplingEmitter();
	HookEmmiter->ResetSystem();
	HookEmmiter->Activate();
	
	CharacterAditionalTicks.AddWeakLambda(this, [this]()
	{
		UpdateGrapplingEmitter();
	});
}
