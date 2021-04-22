// Fill out your copyright notice in the Description page of Project Settings.


#include "GraplingHookComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "WSF/WSFCharacterMovementComponent.h"

// Sets default values for this component's properties
UGraplingHookComponent::UGraplingHookComponent()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	Sphere = CreateDefaultSubobject<USphereComponent>("SphereCollider");
	Spline = CreateDefaultSubobject<USplineComponent>("SplineComponent");
	Mesh->SetupAttachment(this->GetAttachmentRoot());
	Sphere->SetupAttachment(this->GetAttachmentRoot());
	Spline->SetupAttachment(this->GetAttachmentRoot());

	Sphere->OnComponentEndOverlap.AddDynamic(this, &UGraplingHookComponent::OnOverlapEnd);
	
	//We create third point that constitutes our "parabola"  
	Spline->AddSplinePointAtIndex(FVector(200.0, 0.0f, 0.0f), 2, ESplineCoordinateSpace::Local);
	for(int i=0; i < Spline->GetNumberOfSplinePoints(); i++)
	{
		Spline->SetSplinePointType(i, ESplinePointType::CurveClamped);	
	}
	PrimaryComponentTick.bCanEverTick = true;
}

//We need this to disable grappling hook when leaving Sphere Component 
void UGraplingHookComponent::OnOverlapEnd(UPrimitiveComponent* X, AActor* D, UPrimitiveComponent* Q, int RWA)
{
	GEngine->AddOnScreenDebugMessage(111, 2.0f, FColor::Blue, D->GetName());
	PlayerCharacter->bIsGrapplingHookAvailable = false;
}

FVector UGraplingHookComponent::GetVelocityForCurve(const FVector& PlayerPosition)
{
	const FVector& CurveTopPosition = Spline->GetLocationAtSplinePoint(1.0, ESplineCoordinateSpace::World);
	FVector LandingPosition;
	if(!bIsSymmetrical)
	{
		LandingPosition = Spline->GetLocationAtSplinePoint(2.0, ESplineCoordinateSpace::World);
	}
	else
	{
		FVector EndPos = Spline->GetLocationAtSplinePoint(2.0, ESplineCoordinateSpace::World);
		FVector BeginPos = Spline->GetLocationAtSplinePoint(0.0, ESplineCoordinateSpace::World);
		float BeginDist = FVector::Distance(BeginPos, PlayerPosition);
		float EndDist = FVector::Distance(EndPos, PlayerPosition);
		LandingPosition = BeginDist > EndDist ? BeginPos : EndPos;
	}
	
	float Height = FMath::Abs(PlayerPosition.Z-CurveTopPosition.Z);
	FVector HorizontalDirection = LandingPosition - PlayerPosition;
	FVector FlatPlayerPosition = PlayerPosition;
	FlatPlayerPosition.Z = 0.0f;
	LandingPosition.Z = 0.0f;
	float Distance = FVector::Distance(FlatPlayerPosition, LandingPosition);
	HorizontalDirection.Z = 0.0f;
	HorizontalDirection.Normalize();
	FVector HorizontalVel = HorizontalDirection * (Distance/CurveTime);
	PlayerCharacter->GravityCoefficient = -2.0f * Height / ((CurveTime * CurveTime)/4.0f);
	float VerticalMagnitude = 2.0f * Height / (CurveTime / 2.0f ); 
	FVector VerticalVel = FVector(0.0f, 0.0f, 1.0f) * VerticalMagnitude;
	return VerticalVel + HorizontalVel;
}

void UGraplingHookComponent::BeginPlay()
{
	Super::BeginPlay();
	PlayerCharacter = Cast<AWSFCharacter>(GetWorld()->GetFirstPlayerController()->GetCharacter());
	PlayerCapsuleComponent = PlayerCharacter->GetCapsuleComponent();
	PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
}

void UGraplingHookComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);
	Spline->DestroyComponent();
	Sphere->DestroyComponent();
	Mesh->DestroyComponent();
}

void UGraplingHookComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if(PlayerCharacter==nullptr)
	{
		return;
	}
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UWSFCharacterMovementComponent* MovementComponent = Cast<UWSFCharacterMovementComponent>(PlayerCharacter->GetMovementComponent());
	bool bIsGrapplingHook = MovementComponent->MovementMode == MOVE_Custom && MovementComponent->CustomMovementMode == CUSTOM_GrapplingHook;
	if(Sphere->IsOverlappingComponent(PlayerCapsuleComponent.Get()) && !bIsGrapplingHook)
	{
		FHitResult Hit;
		FVector ComponentLocation = Mesh->GetComponentLocation();
		FVector PlayerCapsuleLocation = PlayerCapsuleComponent->GetComponentLocation();
		FCollisionObjectQueryParams QueryParams;
		QueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
		QueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
		QueryParams.AddObjectTypesToQuery(ECC_Pawn);
		DrawDebugDirectionalArrow(GetWorld(), ComponentLocation, PlayerCapsuleLocation, 10.0f, FColor::Red, false, -1, 0, 3.0f);
		GetWorld()->LineTraceSingleByObjectType(Hit, ComponentLocation, PlayerCapsuleLocation, QueryParams);
		if(Hit.IsValidBlockingHit() && Hit.Actor == PlayerCapsuleComponent->GetOwner())
		{
    		PlayerCharacter->GrapplingHookVelocity= GetVelocityForCurve(PlayerCapsuleComponent->GetComponentLocation());
			PlayerCharacter->bIsGrapplingHookAvailable = true;
			FVector2D ScreenLocation;
			bool isInScreen = UGameplayStatics::ProjectWorldToScreen(PlayerController.Get(), Mesh->GetComponentLocation(), ScreenLocation);
			GEngine->AddOnScreenDebugMessage(345, 2.0f, FColor::Blue, ScreenLocation.ToString() + "" + (isInScreen ? TEXT("TRUE") : TEXT("FALSE")));
			PlayerCharacter->UpdateGrapplingHookIndicator(ScreenLocation, isInScreen);
			return;
		}
		PlayerCharacter->bIsGrapplingHookAvailable = false;
	}
	
}