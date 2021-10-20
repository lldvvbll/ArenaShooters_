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

	InputComponent->BindAction(TEXT("ShowLeaderBoard"), EInputEvent::IE_Pressed, this, &AASDeathMatchPlayerController::ShowLeaderBoardWidget);
}

void AASDeathMatchPlayerController::OnChangedInnerMatchState(EInnerMatchState State)
{
	Super::OnChangedInnerMatchState(State);

	ENetMode NetMode = GetNetMode();
	if (NetMode == NM_Client)
	{
		if (State == EInnerMatchState::Finish)
		{
			ShowMatchResultWidget();
		}
	}
}

void AASDeathMatchPlayerController::ShowLeaderBoardWidget()
{
	ShowFullScreenWidget<UASDmLeaderBoardUserWidget>(DmLeaderBoardWidgetClass);
}

void AASDeathMatchPlayerController::ShowMatchResultWidget()
{
	UASDmLeaderBoardUserWidget* LeaderBoardWidget = nullptr;

	if (CurrentFullScreenWidget != nullptr)
	{
		LeaderBoardWidget = Cast<UASDmLeaderBoardUserWidget>(CurrentFullScreenWidget);
		if (LeaderBoardWidget == nullptr)
		{
			CurrentFullScreenWidget->RemoveFromParent();
			OnDestructedFullScreenWidget(CurrentFullScreenWidget);
		}
	}
	
	if (LeaderBoardWidget == nullptr)
	{
		LeaderBoardWidget = ShowFullScreenWidget<UASDmLeaderBoardUserWidget>(DmLeaderBoardWidgetClass);
	}

	if (LeaderBoardWidget != nullptr)
	{
		LeaderBoardWidget->ChangeToMatchResultWidget();
	}
	else
	{
		AS_LOG_S(Error);
	}
}
