// Copyright Epic Games, Inc. All Rights Reserved.

#include "ASGameMode.h"
#include "Character/ASCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "ASPlayerController.h"

AASGameMode::AASGameMode()
{
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ArenaShooters/Blueprints/Characters/BP_Character"));
	//if (PlayerPawnBPClass.Class != nullptr)
	//{
	//	DefaultPawnClass = PlayerPawnBPClass.Class;
	//}

	//PlayerControllerClass = AASPlayerController::StaticClass();
}
