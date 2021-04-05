// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* FP_Gun;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;
	
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AWSFProjectile> ProjectileClass;

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	UFUNCTION(BlueprintCallable)
	void OnFire();
};
