// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/ASDeathMatchPlayerController.h"
#include "GUI/HUD/ASDeathMatchRankingUserWidget.h"
#include "GameMode/ASMatchGameStateBase.h"

AASDeathMatchPlayerController::AASDeathMatchPlayerController()
{
	bCreateRankingWidget = false;
}

void AASDeathMatchPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	AS_LOG_S(Warning);

	if (IsLocalPlayerController())
	{
		if (!bCreateRankingWidget)
		{
			auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
			if (IsValid(GameState))
			{
				EInnerMatchState InnerMatchState = GameState->GetInnerMatchState();
				if (InnerMatchState == EInnerMatchState::Process ||
					InnerMatchState == EInnerMatchState::Finish)
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
}

void AASDeathMatchPlayerController::BeginPlay()
{
	Super::BeginPlay();
		
}