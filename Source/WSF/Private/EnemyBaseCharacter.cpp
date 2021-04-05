// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBaseCharacter.h"

AEnemyBaseCharacter::AEnemyBaseCharacter()
{
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(false);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	FP_Gun->SetupAttachment(RootComponent);
	
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));
}

void AEnemyBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	FP_Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
}

void AEnemyBaseCharacter::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
				
			
				// const FRotator SpawnRotation = GetControlRotation();
				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = FP_MuzzleLocation->GetComponentLocation();

				APlayerController* PC = GetWorld()->GetFirstPlayerController();
				const FVector& PlayerLocation =  PC->GetPawn()->GetActorLocation();
				const FVector FromEnemyToPlayer = PlayerLocation - SpawnLocation;
				const FRotator SpawnRotation = FromEnemyToPlayer.Rotation();

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

				// spawn the projectile at the muzzle
				World->SpawnActor<AWSFProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		}
	}
}
