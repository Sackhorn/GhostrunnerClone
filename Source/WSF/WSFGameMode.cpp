// Copyright Epic Games, Inc. All Rights Reserved.

#include "WSFGameMode.h"
#include "WSFHUD.h"
#include "WSFCharacter.h"
#include "UObject/ConstructorHelpers.h"

AWSFGameMode::AWSFGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AWSFHUD::StaticClass();
}
