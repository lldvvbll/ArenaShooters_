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

		GameState->OnUpdatedRanking.AddUObject(this, &UASDeathMatchRankingUserWidget::UpdatePlayerRanking);

		UpdatePlayerRanking(GameState->GetRankedPlayerStates());
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void UASDeathMatchRankingUserWidget::NativeDestruct()
{
	Super::NativeDestruct();

	auto GameState = GetWorld()->GetGameState<AASDeathmatchGameState>();
	if (IsValid(GameState))
	{
		GameState->OnUpdatedRanking.RemoveAll(this);
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void UASDeathMatchRankingUserWidget::UpdatePlayerRanking(const TArray<FRankedPlayerState>& RankedPlayerStates)
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

	int32 MyIdx = INDEX_NONE;
	for (int32 Idx = 0; Idx < RankedPlayerStates.Num(); ++Idx)
	{
		if (IsValid(RankedPlayerStates[Idx].PlayerState) &&
			RankedPlayerStates[Idx].PlayerState->GetPlayerId() == MyPlayerState->GetPlayerId())
		{
			MyIdx = Idx;
			break;
		}
	}

	if (MyIdx == INDEX_NONE)
		return;

	int32 NumPlayers = RankedPlayerStates.Num();

	TArray<int32> Indices;
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
				RankingSlot->SetPlayerInfo(RankedPlayerStates[Idx].Ranking, RankedPlayerStates[Idx].PlayerState);

				RankScrollBox->AddChild(RankingSlot);
			}
			else
			{
				AS_LOG_S(Error);
			}
		}
	}
}
