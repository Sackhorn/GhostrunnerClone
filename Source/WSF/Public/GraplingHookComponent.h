// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Engine/StaticMeshActor.h"

class AWSFCharacter;
#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"
#include "WSF/WSFCharacter.h"
#include "GraplingHookComponent.generated.h"

static TAutoConsoleVariable<bool> CVarShowGrapplingHook(
	TEXT("ShowGrapplingHook"),
	false,
	TEXT("Shows grappling hook line trace vector\n"),
	ECVF_Scalability | ECVF_RenderThreadSafe);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WSF_API AGraplingHookComponent : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	AGraplingHookComponent();

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* X, AActor* D, UPrimitiveComponent* Q, int32 RWA);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSymmetrical = true;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* Sphere;
	
	UPROPERTY(VisibleAnywhere)
	USplineComponent* Spline;

	TWeakObjectPtr<UCapsuleComponent> PlayerCapsuleComponent;
	TWeakObjectPtr<AWSFCharacter> PlayerCharacter;
	TWeakObjectPtr<APlayerController> PlayerController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurveTime = 1.0f;

	FVector GetVelocityForCurve(const FVector& PlayerPosition);
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
};
