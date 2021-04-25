// Copyright Epic Games, Inc. All Rights Reserved.

#include "WSFHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "CanvasItem.h"
#include "WSFCharacterMovementComponent.h"
#include "Components/Image.h"
#include "UObject/ConstructorHelpers.h"
#include "UMG/Public/Blueprint/UserWidget.h"
#include "Widgets/Notifications/SProgressBar.h"


AWSFHUD::AWSFHUD()
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairTexObj(TEXT("/Game/FirstPerson/Textures/FirstPersonCrosshair"));
	CrosshairTex = CrosshairTexObj.Object;
}

void AWSFHUD::InitializeWidget(UClass* WidgetClass, UUserWidget*& Widget)
{
	if(WidgetClass != nullptr)
	{
		Widget = CreateWidget(PlayerOwner, WidgetClass, FName(WidgetClass->GetName()));
		Widget->AddToViewport(++WidgetZOrder);
	}
}

void AWSFHUD::BeginPlay()
{
	Super::BeginPlay();
	InitializeWidget(DashIndicatorWidgetClass, DashIndicatorWidget);
	InitializeWidget(HitMarkersWidgetClass, HitMarkersWidget);
	InitializeWidget(GrapplingHookWidgetClass, GrapplingHookWidget);
	DashProgressBar = DashIndicatorWidget->GetSlateWidgetFromName(FName(TEXT("ProgressBar_0")));
	GrapplingHookIndicator = GrapplingHookWidget->GetSlateWidgetFromName(FName(TEXT("GrapplingHookIndicator")));
	HitMarkersWidget->SetVisibility(ESlateVisibility::Hidden);
}

void AWSFHUD::DrawHUD()
{
	Super::DrawHUD();
	
	// find center of the Canvas
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	const FVector2D CrosshairDrawPosition( (Center.X),
										   (Center.Y + 20.0f));

	// draw the crosshair
	FCanvasTileItem TileItem( CrosshairDrawPosition, CrosshairTex->Resource, FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem( TileItem );
}

void AWSFHUD::OnDead()
{
	HitMarkersWidget->SetVisibility(ESlateVisibility::Visible);
}

void AWSFHUD::UpdateDashIndicator(const FTimerHandle& SidewayDashTimer, const FTimerHandle& DashDisablerTimer)
{
	UWSFCharacterMovementComponent* MovementComponent = static_cast<UWSFCharacterMovementComponent*>(GetOwningPlayerController()->GetCharacter()->GetMovementComponent());
	auto ProgressBar = (SProgressBar*)DashProgressBar.Get();
	float timeElapsed;
	if(MovementComponent->MovementMode == MOVE_Custom && MovementComponent->CustomMovementMode == CUSTOM_SidewaysDash)
	{
		timeElapsed = GetWorldTimerManager().GetTimerElapsed(SidewayDashTimer)/GetWorldTimerManager().GetTimerRate(SidewayDashTimer);
		ProgressBar->SetBarFillType(EProgressBarFillType::RightToLeft);
	}
	else
	{
		timeElapsed = GetWorldTimerManager().GetTimerElapsed(DashDisablerTimer)/GetWorldTimerManager().GetTimerRate(DashDisablerTimer);
		ProgressBar->SetBarFillType(EProgressBarFillType::LeftToRight);
	}
	ProgressBar->SetPercent(FMath::Abs(timeElapsed));
}

void AWSFHUD::UpdateGrapplingHookIndicator(FVector2D Position, bool Visibility)
{
	GrapplingHookIndicator->SetVisibility(Visibility ? EVisibility::Visible : EVisibility::Hidden);
	GrapplingHookIndicator->SetRenderTransform(FTransform2D(Position));
}