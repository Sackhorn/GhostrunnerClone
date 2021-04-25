// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"

#include "Components/Widget.h"
#include "GameFramework/HUD.h"
#include "WSFHUD.generated.h"

UCLASS()
class AWSFHUD : public AHUD
{
	GENERATED_BODY()

public:
	AWSFHUD();
	void InitializeWidget(UClass* WidgetClass, UUserWidget*& Widget);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UClass* DashIndicatorWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UClass* HitMarkersWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UClass* GrapplingHookWidgetClass;

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;
	void OnDead();
	void UpdateDashIndicator(const FTimerHandle& SidewayDashTimer, const FTimerHandle& DashDisablerTimer);
	void UpdateGrapplingHookIndicator(FVector2D Position, bool Visibility);

private:
	uint8 WidgetZOrder = 0;

protected:
	virtual void BeginPlay() override;
private:
	TSharedPtr<SWidget> DashProgressBar;
	TSharedPtr<SWidget> GrapplingHookIndicator;
	
	UPROPERTY()
	UUserWidget* DashIndicatorWidget;
	
	UPROPERTY()
	UUserWidget* HitMarkersWidget;
	
	UPROPERTY()
	UUserWidget* GrapplingHookWidget;

	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

