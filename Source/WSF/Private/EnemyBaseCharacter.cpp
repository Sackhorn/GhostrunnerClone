// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBaseCharacter.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "SkeletalRenderPublic.h"
#include "Components/CapsuleComponent.h"
#include "ProceduralMeshComponent/Public/ProceduralMeshComponent.h"
#include "ProceduralMeshComponent/Public/KismetProceduralMeshLibrary.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Components/SkinnedMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WSF/WSFCharacter.h"

AEnemyBaseCharacter::AEnemyBaseCharacter()
{
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(false);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	FP_Gun->SetupAttachment(RootComponent);
	
	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	ProcMesh->SetupAttachment(RootComponent);
	ProcMesh->SetVisibility(false);

	ProcMesh2 = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh2"));
	ProcMesh2->SetupAttachment(RootComponent);
	ProcMesh2->SetVisibility(false);

	SkelMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh2"));
	SkelMesh->SetupAttachment(RootComponent);
	SkelMesh->SetVisibility(true);
}

void AEnemyBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	FP_Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AEnemyBaseCharacter::HandleCollision);
	
}

void AEnemyBaseCharacter::HandleCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if(OtherActor->GetClass()->IsChildOf(AWSFCharacter::StaticClass()) && !isDead && OtherComp->StaticClass()->IsChildOf(USphereComponent::StaticClass()))
	{
		FVector ForceDirection = OtherActor->GetActorLocation() - GetActorLocation();
		OnDeath(ForceDirection);
	}
}

void AEnemyBaseCharacter::OnDeath(const FVector& ForceDirection)
{
	isDead = true;
	GenerateProcMesh();
	DoRagdoll(ForceDirection);
	DetachGun();
	DisableAI();
}

void AEnemyBaseCharacter::DisableAI()
{
	Cast<AAIController>(GetController())->BrainComponent->Deactivate();
}

void AEnemyBaseCharacter::DetachGun()
{
	FVector RagDollDirection = GetWorld()->GetFirstPlayerController()->GetTargetLocation() - GetActorLocation();
	FP_Gun->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	FP_Gun->SetCollisionProfileName("BlockAll");
	FP_Gun->SetSimulatePhysics(true);
	FP_Gun->SetAllBodiesBelowSimulatePhysics("Root_Bone", true);
}

void AEnemyBaseCharacter::GenerateProcMesh()
{
	auto CharMesh = GetMesh();
	TArray<FVector> SkinnedVertices;
	TArray<FMatrix> CachedRefToLocal;
	CharMesh->GetCurrentRefToLocalMatrices(CachedRefToLocal, 0);
	USkeletalMeshComponent::ComputeSkinnedPositions(CharMesh, SkinnedVertices,
		CachedRefToLocal, CharMesh->MeshObject->GetSkeletalMeshRenderData().LODRenderData[0],
		CharMesh->MeshObject->GetSkeletalMeshRenderData().LODRenderData[0].SkinWeightVertexBuffer);
	auto IndexBuffer = CharMesh->MeshObject->GetSkeletalMeshRenderData().LODRenderData[0].MultiSizeIndexContainer.GetIndexBuffer();
	auto VertexColor = &CharMesh->MeshObject->GetSkeletalMeshRenderData().LODRenderData[0].StaticVertexBuffers.ColorVertexBuffer;
	TArray<FVector> SkinnedTangent;
	USkeletalMeshComponent::ComputeSkinnedTangentBasis(CharMesh, SkinnedTangent, CachedRefToLocal,
		CharMesh->MeshObject->GetSkeletalMeshRenderData().LODRenderData[0],
		CharMesh->MeshObject->GetSkeletalMeshRenderData().LODRenderData[0].SkinWeightVertexBuffer);
	TArray<FColor> Color;
	TArray<int32> Triangles;
	TArray<FVector2D> UVs;
	for(int i=0; i < IndexBuffer->Num(); i++)
	{
		Triangles.Add(IndexBuffer->Get(i));
	}
	VertexColor->GetVertexColors(Color);
	for(int32 i=0; i < SkinnedVertices.Num(); i++)
	{
		UVs.Add(CharMesh->MeshObject->GetSkeletalMeshRenderData().LODRenderData[0].StaticVertexBuffers.StaticMeshVertexBuffer.GetVertexUV(i, 0));
	}
	TArray<FVector> MyNormals; 
	TArray<FProcMeshTangent> MyTangentsX;
	for(int32 i = 0; i < SkinnedVertices.Num(); i++)
	{
		FVector TangentX = SkinnedTangent[2*i];
		FVector TangentZ = SkinnedTangent[2*i + 1];
		FVector TangentY = TangentX ^ TangentZ;
		TangentX.Normalize();
		TangentZ.Normalize();
		MyNormals.Add(TangentZ);
		TangentX -= TangentZ * (TangentZ | TangentX);
		TangentX.Normalize();
		const bool bFlipBitangent = ((TangentZ ^ TangentX) | TangentY) < 0.f;
		MyTangentsX.Add(FProcMeshTangent(TangentX, bFlipBitangent));
	}
	ProcMesh->CreateMeshSection(0, SkinnedVertices, Triangles, MyNormals, UVs, Color, MyTangentsX, false);
	ProcMesh->SetWorldTransform(CharMesh->GetComponentToWorld());
	ProcMesh->SetMaterial(0, CharMesh->GetMaterial(0));
	ProcMesh->SetMaterial(1, CharMesh->GetMaterial(1));
	UKismetProceduralMeshLibrary::SliceProceduralMesh(ProcMesh, GetActorLocation(), GetActorUpVector(), true, ProcMesh2, EProcMeshSliceCapOption::NoCap, CharMesh->GetMaterial(0));
	ProcMesh2->SetVisibility(false);
}

void AEnemyBaseCharacter::DoRagdoll(const FVector& ForceDirection)
{
	auto CharMesh = GetMesh();
	GetCapsuleComponent()->SetCollisionProfileName("OverlapAll");
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();	
	CharMesh->SetAllBodiesBelowSimulatePhysics("pelvis", true);
	CharMesh->AddImpulse(ForceDirection * RagDollForce);
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
