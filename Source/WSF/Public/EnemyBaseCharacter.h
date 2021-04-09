// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ProceduralMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "GameFramework/Character.h"
#include "WSF/WSFProjectile.h"

#include "EnemyBaseCharacter.generated.h"

UCLASS()
class WSF_API AEnemyBaseCharacter : public ACharacter
{
	GENERATED_BODY()
	AEnemyBaseCharacter();
public:
	virtual void BeginPlay() override;
	UFUNCTION()
	void HandleCollision(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                     FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* FP_Gun;

	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AWSFProjectile> ProjectileClass;

	void GnerateProcMesh();
	
	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* ProcMesh;
	
	UPROPERTY(VisibleAnywhere)
	UPoseableMeshComponent* PoseableMesh;
	
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	UFUNCTION(BlueprintCallable)
	void OnFire();
};
