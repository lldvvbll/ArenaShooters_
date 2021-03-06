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

void AASDeathMatchPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (IsLocalPlayerController())
	{
		auto GameState = GetWorld()->GetGameState();
		if (GameState != nullptr)	// nullable
		{
			CreateRankingWidget();
		}
		else
		{
			GetWorld()->GameStateSetEvent.AddUObject(this, &AASDeathMatchPlayerController::OnSetGameStateToWorld);
		}
	}
}

void AASDeathMatchPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (IsLocalPlayerController())
	{
		auto ASPlayerState = GetPlayerState<AASPlayerState>();
		if (ensure(ASPlayerState))
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
		auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
		if (ensure(GameState))
		{
			FText MsgTxt = FText::Format(WelcomeMessage, FText::AsNumber(GameState->GetMinNumPlayer()));
			NotifyMessage(MsgTxt.ToString(), 10.0f);
		}
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

void AASDeathMatchPlayerController::CreateRankingWidget()
{
	if (bCreateRankingWidget)
		return;

	DeathMatchRankingWidget = CreateWidget<UASDeathMatchRankingUserWidget>(this, DeathMatchRankingWidgetClass);
	if (DeathMatchRankingWidget != nullptr)
	{
		DeathMatchRankingWidget->AddToViewport(1);

		bCreateRankingWidget = true;
	}
}

void AASDeathMatchPlayerController::OnChangedDeathCount(int32 NewDeathCount)
{
	if (IsLocalPlayerController())
	{
		NotifyMessage(ItemSetChangeButtonNotification.ToString(), 3.0f);
	}
}

void AASDeathMatchPlayerController::OnSetGameStateToWorld(AGameStateBase* NewGameState)
{
	if (IsLocalPlayerController())
	{
		CreateRankingWidget();
	}
}
