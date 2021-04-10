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
	void HandleCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* FP_Gun;

	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AWSFProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attack)
	float BeforeShootDelay = 1.0f;

	void GenerateProcMesh();
	void DoRagdoll(const FVector& ForceDirection);
	void OnDeath(const FVector& ForceDirection);
	void DisableAI();
	void DetachGun();
	bool isDead;
	
	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* ProcMesh;
	
	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* ProcMesh2;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* SkelMesh;

	UPROPERTY(EditAnywhere)
	float RagDollForce;
	
	UPROPERTY(VisibleAnywhere)
	UPoseableMeshComponent* PoseableMesh;
	
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	UFUNCTION(BlueprintCallable)
	void OnFire();
};
