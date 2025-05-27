// Copyright Epic Games, Inc. All Rights Reserved.

#include "Plugin_DevelopmentGameMode.h"
#include "Plugin_DevelopmentCharacter.h"
#include "UObject/ConstructorHelpers.h"

APlugin_DevelopmentGameMode::APlugin_DevelopmentGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game//Heroes/BP_HeroCharacter_Test")); //F:/UnrealProjects/Plugin_Development/Content/Heroes/BP_HeroCharacter_Test.uasset /Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
		
	}
}
