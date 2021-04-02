// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "GameFramework/Actor.h"
#include "ASplineRenderActor.generated.h"

UCLASS()
class AASplineRenderActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AASplineRenderActor();

	UPROPERTY(VisibleAnywhere)
	USceneComponent * DefaultSceneComponent;
	
	UPROPERTY(VisibleAnywhere)
	USplineComponent* SplineComponent;

	UPROPERTY(EditAnywhere)
	USplineMeshComponent* BaseSplineMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<USplineMeshComponent*> SplineMeshComponents;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void OnConstruction(const FTransform& Transform) override;
	// virtual void BeginDestroy() override;
	virtual void Tick(float DeltaTime) override;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
};
