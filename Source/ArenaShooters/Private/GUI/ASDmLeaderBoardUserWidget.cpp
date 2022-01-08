// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/ASDmLeaderBoardUserWidget.h"
#include "GUI/ASDmLeaderBoardSlotUserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "GameMode/ASDeathmatchGameState.h"
#include "Controller/ASPlayerState.h"

void UASDmLeaderBoardUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	TitleTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("TitleTextBlock")));
	GoalNumOfKillsTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("GoalNumOfKillsTextBlock")));
	RankingScrollBox = Cast<UScrollBox>(GetWidgetFromName(TEXT("RankingScrollBox")));

	auto GameState = GetWorld()->GetGameState<AASDeathmatchGameState>();
	if (GameState)
	{
		GameState->OnUpdatedRanking.AddUObject(this, &UASDmLeaderBoardUserWidget::UpdateRanking);

		if (GameState->GetInnerMatchState() == EInnerMatchState::Finish)
		{
			if (TitleTextBlock != nullptr)
			{
				FText TitleText = FText::FromString(TEXT("MATCH RESULT"));

				TitleTextBlock->SetColorAndOpacity(MatchResultTitleColor);
				TitleTextBlock->SetText(TitleText);
			}
		}

		if (GoalNumOfKillsTextBlock != nullptr)
		{
			GoalNumOfKillsTextBlock->SetText(FText::FromString(FString::FromInt(GameState->GetGoalNumOfKills())));
		}

		UpdateRanking(GameState->GetRankedPlayerStates());
	}

	// M, ESC
	FOnInputAction BackToGameAction;
	BackToGameAction.BindDynamic(this, &UASDmLeaderBoardUserWidget::BackToGame);
	ListenForInputAction(FName(TEXT("ShowLeaderBoard")), EInputEvent::IE_Pressed, true, BackToGameAction);
	ListenForInputAction(FName(TEXT("ShowGameMenu")), EInputEvent::IE_Pressed, true, BackToGameAction);

	OnConstructed.Broadcast(this);
}

void UASDmLeaderBoardUserWidget::NativeDestruct()
{
	Super::NativeDestruct();

	auto GameState = GetWorld()->GetGameState<AASDeathmatchGameState>();
	if (GameState)
	{
		GameState->OnUpdatedRanking.RemoveAll(this);
	}

	OnDestructed.Broadcast(this);
}

void UASDmLeaderBoardUserWidget::BackToGame()
{
	RemoveFromParent();
}

void UASDmLeaderBoardUserWidget::UpdateRanking(const TArray<FRankedPlayerState>& RankedPlayerStates)
{
	if (RankingScrollBox != nullptr)
	{
		RankingScrollBox->ClearChildren();

		auto GameState = GetWorld()->GetGameState<AASDeathmatchGameState>();
		if (GameState)
		{
			for (int32 Idx = 0; Idx < RankedPlayerStates.Num(); ++Idx)
			{
				auto SlotWidget = CreateWidget<UASDmLeaderBoardSlotUserWidget>(this, DmLeaderBoardSlotWidgetClass);
				if (ensure(SlotWidget != nullptr))
				{
					SlotWidget->SetInfo(RankedPlayerStates[Idx].Ranking, RankedPlayerStates[Idx].PlayerState);

					RankingScrollBox->AddChild(SlotWidget);
				}
			}
		}
	}
}
