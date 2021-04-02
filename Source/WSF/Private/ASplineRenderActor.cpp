// Fill out your copyright notice in the Description page of Project Settings.


#include "ASplineRenderActor.h"

#include "Engine/EngineTypes.h"

// Sets default values
void CopySplineParamsForSegment(USplineMeshComponent* SplineMeshComponent, USplineComponent* SplineComponent, int SplineSegment)
{
	// SplineMeshComponent->SplineParams.StartRoll = SplineComponent->GetRollAtSplinePoint(SplineSegment, ESplineCoordinateSpace::World);
	// SplineMeshComponent->SplineParams.StartScale = static_cast<FVector2D>(SplineComponent->GetScaleAtSplinePoint(SplineSegment));
	// SplineMeshComponent->SplineParams.StartPos = SplineComponent->GetLocationAtSplinePoint(SplineSegment, ESplineCoordinateSpace::World);
	// SplineMeshComponent->SplineParams.StartTangent = SplineComponent->GetTangentAtSplinePoint(SplineSegment, ESplineCoordinateSpace::World);
	
	// SplineMeshComponent->SplineParams.EndRoll = SplineComponent->GetRollAtSplinePoint(SplineSegment+1, ESplineCoordinateSpace::World);
	// SplineMeshComponent->SplineParams.EndScale = static_cast<FVector2D>(SplineComponent->GetScaleAtSplinePoint(SplineSegment+1));
	// SplineMeshComponent->SplineParams.EndPos = SplineComponent->GetLocationAtSplinePoint(SplineSegment+1, ESplineCoordinateSpace::World);
	// SplineMeshComponent->SplineParams.EndTangent = SplineComponent->GetTangentAtSplinePoint(SplineSegment+1, ESplineCoordinateSpace::World);
	auto StartRoll = SplineComponent->GetRollAtSplinePoint(SplineSegment, ESplineCoordinateSpace::World);
	auto StartScale = static_cast<FVector2D>(SplineComponent->GetScaleAtSplinePoint(SplineSegment));
	auto StartPos = SplineComponent->GetLocationAtSplinePoint(SplineSegment, ESplineCoordinateSpace::World);
	auto StartTangent = SplineComponent->GetTangentAtSplinePoint(SplineSegment, ESplineCoordinateSpace::World);
	
	auto EndRoll = SplineComponent->GetRollAtSplinePoint(SplineSegment+1, ESplineCoordinateSpace::World);
	auto EndScale = static_cast<FVector2D>(SplineComponent->GetScaleAtSplinePoint(SplineSegment+1));
	auto EndPos = SplineComponent->GetLocationAtSplinePoint(SplineSegment+1, ESplineCoordinateSpace::World);
	auto EndTangent = SplineComponent->GetTangentAtSplinePoint(SplineSegment+1, ESplineCoordinateSpace::World);
	SplineMeshComponent->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);
}

AASplineRenderActor::AASplineRenderActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	DefaultSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Default Scene Component"));
	SetRootComponent(DefaultSceneComponent);
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	BaseSplineMeshComponent = CreateDefaultSubobject<USplineMeshComponent>(TEXT("SplineMeshComponent"));
	BaseSplineMeshComponent->SetupAttachment(DefaultSceneComponent);
	SplineComponent->SetupAttachment(DefaultSceneComponent);
}


void AASplineRenderActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	// for(auto& splineToRemove: SplineMeshComponents)
	// {
	// 	if(IsValid(splineToRemove))
	// 	{
	// 		splineToRemove->DestroyComponent();
	// 	}
	// }
	// SplineMeshComponents.Empty();
	//
	// const int Segments = SplineComponent->GetNumberOfSplineSegments();
	// for (int i=0 ; i < Segments ; i++)
	// {
	// 	// USplineMeshComponent * NewSplineSegment = NewObject<USplineMeshComponent>(DefaultSceneComponent);
	// 	USplineMeshComponent * NewSplineSegment = DuplicateObject(BaseSplineMeshComponent, DefaultSceneComponent);
	// 	NewSplineSegment->RegisterComponent();
	// 	SplineMeshComponents.Add(NewSplineSegment);
	// 	CopySplineParamsForSegment(NewSplineSegment, SplineComponent, i);
	// 	NewSplineSegment->SetVisibility(true);
	// 	NewSplineSegment->SetHiddenInGame(false);
	// 	NewSplineSegment->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// 	// NewSplineSegment->SetStaticMesh(StaticMesh);
	// }
	// BaseSplineMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void AASplineRenderActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AASplineRenderActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AASplineRenderActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	for(auto& splineToRemove: SplineMeshComponents)
	{
		if(IsValid(splineToRemove))
		{
			splineToRemove->DestroyComponent();
		}
	}
	SplineMeshComponents.Empty();

	const int Segments = SplineComponent->GetNumberOfSplineSegments();
	for (int i=0 ; i < Segments ; i++)
	{
		// USplineMeshComponent * NewSplineSegment = NewObject<USplineMeshComponent>(DefaultSceneComponent);
		USplineMeshComponent * NewSplineSegment = DuplicateObject(BaseSplineMeshComponent, DefaultSceneComponent);
		NewSplineSegment->RegisterComponent();
		SplineMeshComponents.Add(NewSplineSegment);
		CopySplineParamsForSegment(NewSplineSegment, SplineComponent, i);
		NewSplineSegment->SetVisibility(true);
		NewSplineSegment->SetHiddenInGame(false);
		NewSplineSegment->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		// NewSplineSegment->SetStaticMesh(StaticMesh);
	}
	BaseSplineMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

