// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class AWSFCharacter;
#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"
#include "WSF/WSFCharacter.h"
#include "GraplingHookComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WSF_API UGraplingHookComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGraplingHookComponent();

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
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
