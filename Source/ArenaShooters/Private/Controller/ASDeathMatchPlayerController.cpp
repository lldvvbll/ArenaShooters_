// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/ASDeathMatchPlayerController.h"
#include "GUI/HUD/ASDeathMatchRankingUserWidget.h"
#include "GUI/ASDmLeaderBoardUserWidget.h"
#include "GameMode/ASMatchGameStateBase.h"
#include "Character/ASCharacter.h"
#include "Controller/ASPlayerState.h"

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
			if (ensure(IsValid(GameState)))
			{
				DeathMatchRankingWidget = CreateWidget<UASDeathMatchRankingUserWidget>(this, DeathMatchRankingWidgetClass);
				if (DeathMatchRankingWidget != nullptr)
				{
					DeathMatchRankingWidget->AddToViewport(1);

					bCreateRankingWidget = true;
				}
			}
		}

		auto ASPlayerState = GetPlayerState<AASPlayerState>();
		if (ensure(IsValid(ASPlayerState)))
		{
			ASPlayerState->OnChangedDeathCount.AddUObject(this, &AASDeathMatchPlayerController::OnChangedDeathCount);
		}
	}
}

void AASDeathMatchPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		NotifyMessage(ItemSetChangeButtonNotification.ToString(), 10.0f);
	}
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
			RemoveFullScreenWidget();
			ShowLeaderBoardWidget();
		}
	}
}

void AASDeathMatchPlayerController::ShowLeaderBoardWidget()
{
	ShowFullScreenWidget<UASDmLeaderBoardUserWidget>(DmLeaderBoardWidgetClass);
}

void AASDeathMatchPlayerController::OnChangedDeathCount(int32 NewDeathCount)
{
	if (IsLocalPlayerController())
	{
		NotifyMessage(ItemSetChangeButtonNotification.ToString(), 3.0f);
	}
}
