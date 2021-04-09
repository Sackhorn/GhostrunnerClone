// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBaseCharacter.h"

#include "SkeletalRenderPublic.h"
#include "Components/CapsuleComponent.h"
#include "ProceduralMeshComponent/Public/ProceduralMeshComponent.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Components/SkinnedMeshComponent.h"
#include "ProceduralMeshComponent/Public/KismetProceduralMeshLibrary.h"

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
	ProcMesh->SetVisibility(true);
}

void AEnemyBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	FP_Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AEnemyBaseCharacter::HandleCollision);
	
}

void AEnemyBaseCharacter::HandleCollision(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if(OtherActor->GetClass()->IsChildOf(AWSFProjectile::StaticClass()))
	{
		GnerateProcMesh();		
	}
}

void AEnemyBaseCharacter::GnerateProcMesh()
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
	
	
	TArray<FColor> Color;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FProcMeshTangent> Tangents;
	for(int i=0; i < IndexBuffer->Num(); i++)
	{
		Triangles.Add(IndexBuffer->Get(i));
	}
	VertexColor->GetVertexColors(Color);
	for(int32 i=0; i < SkinnedVertices.Num(); i++)
	{
		Normals.Add(FVector(0.0f));
		UVs.Add(CharMesh->MeshObject->GetSkeletalMeshRenderData().LODRenderData[0].StaticVertexBuffers.StaticMeshVertexBuffer.GetVertexUV(i, 0));
		auto X = CharMesh->MeshObject->GetSkeletalMeshRenderData().LODRenderData[0].StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentX(i);
		auto Z = CharMesh->MeshObject->GetSkeletalMeshRenderData().LODRenderData[0].StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentY(i);
		Tangents.Add(FProcMeshTangent(X.X, X.Y, X.Z));
		Normals.Add(FVector(Z));
	}
	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(SkinnedVertices, Triangles, UVs, Normals, Tangents);
	ProcMesh->CreateMeshSection(0, SkinnedVertices, Triangles, Normals, UVs, Color, Tangents, false);
	ProcMesh->SetWorldTransform(CharMesh->GetComponentToWorld());
	ProcMesh->SetMaterial(0, CharMesh->GetMaterial(0));
	ProcMesh->SetMaterial(1, CharMesh->GetMaterial(1));
	UE_LOG(LogTemp, Warning, TEXT("Shits calculated"));
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
