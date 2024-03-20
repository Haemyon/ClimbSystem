// Copyright Epic Games, Inc. All Rights Reserved.

#include "ClimbSystemGameMode.h"
#include "ClimbSystemCharacter.h"
#include "UObject/ConstructorHelpers.h"

AClimbSystemGameMode::AClimbSystemGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
