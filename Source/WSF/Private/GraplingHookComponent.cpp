// Fill out your copyright notice in the Description page of Project Settings.


#include "GraplingHookComponent.h"
#include "DrawDebugHelpers.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "WSF/WSFCharacterMovementComponent.h"

// Sets default values for this component's properties
AGraplingHookComponent::AGraplingHookComponent()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	Sphere = CreateDefaultSubobject<USphereComponent>("SphereCollider");
	Spline = CreateDefaultSubobject<USplineComponent>("SplineComponent");
	SetRootComponent(Mesh);
	
	Sphere->OnComponentEndOverlap.AddDynamic(this, &AGraplingHookComponent::OnOverlapEnd);
	
	//We create third point that constitutes our "parabola"  
	Spline->AddSplinePointAtIndex(FVector(200.0, 0.0f, 0.0f), 2, ESplineCoordinateSpace::Local);
	for(int i=0; i < Spline->GetNumberOfSplinePoints(); i++)
	{
		Spline->SetSplinePointType(i, ESplinePointType::CurveClamped);	
	}
}

//We need this to disable grappling hook when leaving Sphere Component 
void AGraplingHookComponent::OnOverlapEnd(UPrimitiveComponent* X, AActor* D, UPrimitiveComponent* Q, int RWA)
{
	GEngine->AddOnScreenDebugMessage(111, 2.0f, FColor::Blue, D->GetName());
	PlayerCharacter->bIsGrapplingHookAvailable = false;
	PlayerHUD->UpdateGrapplingHookIndicator(FVector2D::ZeroVector, false);
}

FVector AGraplingHookComponent::GetVelocityForCurve(const FVector& PlayerPosition)
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

void AGraplingHookComponent::BeginPlay()
{
	Super::BeginPlay();
	PlayerCharacter = Cast<AWSFCharacter>(GetWorld()->GetFirstPlayerController()->GetCharacter());
	PlayerCapsuleComponent = PlayerCharacter->GetCapsuleComponent();
	PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PlayerHUD = Cast<AWSFHUD>(PlayerController->GetHUD());
}

void AGraplingHookComponent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(PlayerCharacter==nullptr)
	{
		return;
	}
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
		if(CVarShowGrapplingHook.GetValueOnGameThread())
		{
			DrawDebugDirectionalArrow(GetWorld(), ComponentLocation, PlayerCapsuleLocation, 10.0f, FColor::Red, false, -1, 0, 3.0f);
		}
		GetWorld()->LineTraceSingleByObjectType(Hit, ComponentLocation, PlayerCapsuleLocation, QueryParams);
		if(Hit.IsValidBlockingHit() && Hit.Actor == PlayerCapsuleComponent->GetOwner())
		{
			FVector2D ScreenLocation;
			UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PlayerController.Get(), GetActorLocation(), ScreenLocation, false);
			bool bIsInScreen = Mesh->WasRecentlyRendered(0.1);
			GEngine->AddOnScreenDebugMessage(345, 2.0f, FColor::Blue, ScreenLocation.ToString() + "" + (bIsInScreen ? TEXT("TRUE") : TEXT("FALSE")));
			PlayerCharacter->bIsGrapplingHookAvailable = bIsInScreen;
			PlayerCharacter->GrapplingHookVelocity= GetVelocityForCurve(PlayerCapsuleComponent->GetComponentLocation());
			PlayerCharacter->SetGrapplingHookLocation(Mesh->GetComponentLocation());
			PlayerHUD->UpdateGrapplingHookIndicator(FVector2D::Max(ScreenLocation, FVector2D::ZeroVector), bIsInScreen);
			return;
		}
		PlayerCharacter->bIsGrapplingHookAvailable = false;
	}
	// PlayerHUD->UpdateGrapplingHookIndicator(FVector2D::ZeroVector, false);
}