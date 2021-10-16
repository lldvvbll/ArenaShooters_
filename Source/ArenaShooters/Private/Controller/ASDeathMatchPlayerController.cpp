// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/ASDeathMatchPlayerController.h"
#include "GUI/HUD/ASDeathMatchRankingUserWidget.h"
#include "GUI/ASDmLeaderBoardUserWidget.h"
#include "GameMode/ASMatchGameStateBase.h"
#include "Character/ASCharacter.h"

AASDeathMatchPlayerController::AASDeathMatchPlayerController()
{
	bCreateRankingWidget = false;
}

void AASDeathMatchPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (IsLocalPlayerController())
	{
		if (!bCreateRankingWidget)
		{
			auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
			if (IsValid(GameState))
			{
				DeathMatchRankingWidget = CreateWidget<UASDeathMatchRankingUserWidget>(this, DeathMatchRankingWidgetClass);
				if (DeathMatchRankingWidget != nullptr)
				{
					DeathMatchRankingWidget->AddToViewport(1);

					bCreateRankingWidget = true;
				}
			}
		}		
	}
}

void AASDeathMatchPlayerController::BeginPlay()
{
	Super::BeginPlay();
		
}

void AASDeathMatchPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction(TEXT("ShowLeaderBoard"), EInputEvent::IE_Pressed, this, &AASDeathMatchPlayerController::ToggleShowLeaderBoardWidget);
}

void AASDeathMatchPlayerController::ToggleShowLeaderBoardWidget()
{
	auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
	if (IsValid(GameState))
	{
		EInnerMatchState InnerMatchState = GameState->GetInnerMatchState();
		if (InnerMatchState == EInnerMatchState::Process ||
			InnerMatchState == EInnerMatchState::Finish)
		{
			ToggleFullScreenWidget<UASDmLeaderBoardUserWidget>(DmLeaderBoardWidgetClass);
		}
	}
}
