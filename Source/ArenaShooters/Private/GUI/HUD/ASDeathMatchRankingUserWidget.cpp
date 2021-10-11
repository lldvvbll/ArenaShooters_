// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/HUD/ASDeathMatchRankingUserWidget.h"
#include "GameMode/ASDeathmatchGameState.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Controller/ASPlayerState.h"
#include "GUI/HUD/ASDmRankingSlotUserWidget.h"

void UASDeathMatchRankingUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	GoalNumOfKillsTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("GoalNumOfKillsTextBlock")));
	RankScrollBox = Cast<UScrollBox>(GetWidgetFromName(TEXT("RankScrollBox")));

	auto GameState = GetWorld()->GetGameState<AASDeathmatchGameState>();
	if (IsValid(GameState))
	{
		if (GoalNumOfKillsTextBlock != nullptr)
		{
			GoalNumOfKillsTextBlock->SetText(FText::FromString(FString::FromInt(GameState->GetGoalNumOfKills())));
		}

		GameState->OnKill.AddUObject(this, &UASDeathMatchRankingUserWidget::OnKill);
		GameState->OnChangedNumPlayers.AddUObject(this, &UASDeathMatchRankingUserWidget::OnChangedNumPlayers);

		UpdatePlayerRanking();
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void UASDeathMatchRankingUserWidget::OnKill(AASPlayerState* KillerPlayerState, AASPlayerState* DeadPlayerState)
{
	UpdatePlayerRanking();
}

void UASDeathMatchRankingUserWidget::OnChangedNumPlayers(int32 PlayerNum)
{
	UpdatePlayerRanking();
}

void UASDeathMatchRankingUserWidget::UpdatePlayerRanking()
{
	if (RankScrollBox == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	RankScrollBox->ClearChildren();

	auto GameState = GetWorld()->GetGameState<AASDeathmatchGameState>();
	if (!IsValid(GameState))
	{
		AS_LOG_S(Error);
		return;
	}

	auto MyPlayerState = GetOwningPlayer()->GetPlayerState<AASPlayerState>();
	if (!IsValid(MyPlayerState))
	{
		AS_LOG_S(Error);
		return;
	}

	TArray<AASPlayerState*> Players = GameState->GetPlayersSortedByKillCount();
	
	int32 MyIdx = Players.Find(MyPlayerState);
	if (MyIdx == INDEX_NONE)
	{
		AS_LOG_S(Error);
		return;
	}

	TArray<int32> Indices;
	int32 NumPlayers = Players.Num();
	if (MyIdx == 0)
	{
		Indices.Append({ 0, 1, 2 });
	}
	else if (MyIdx == NumPlayers - 1)
	{
		Indices.Append({ MyIdx - 2, MyIdx - 1, MyIdx });
	}
	else
	{
		Indices.Append({ MyIdx - 1, MyIdx, MyIdx + 1 });
	}

	for (auto& Idx : Indices)
	{
		if (Idx >= 0 && Idx < NumPlayers)
		{
			auto RankingSlot = CreateWidget<UASDmRankingSlotUserWidget>(this, DmRankingSlotWidgetClass);
			if (RankingSlot != nullptr)
			{
				RankingSlot->SetPlayerInfo(Idx + 1, Players[Idx]);

				RankScrollBox->AddChild(RankingSlot);
			}
			else
			{
				AS_LOG_S(Error);
			}
		}
	}
}
