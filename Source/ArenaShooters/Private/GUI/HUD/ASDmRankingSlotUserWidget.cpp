// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/HUD/ASDmRankingSlotUserWidget.h"
#include "Controller/ASPlayerState.h"
#include "Components/TextBlock.h"

void UASDmRankingSlotUserWidget::SetPlayerInfo(int32 Rank, AASPlayerState* PlayerState)
{
	if (!IsValid(PlayerState))
	{
		AS_LOG_S(Error);
		return;
	}

	bool bMyPlayerState = (PlayerState != nullptr && GetOwningPlayer()->GetPlayerState<AASPlayerState>() == PlayerState);
	FLinearColor TextColor = bMyPlayerState ? MyPlayerStateTextColor : OtherPlayerStateTextColor;

	if (RankTextBlock != nullptr)
	{
		RankTextBlock->SetText(FText::FromString(FString::FromInt(Rank)));
		RankTextBlock->SetColorAndOpacity(TextColor);
	}

	if (NameTextBlock != nullptr)
	{
		NameTextBlock->SetText(FText::FromString(PlayerState->GetPlayerName()));
		NameTextBlock->SetColorAndOpacity(TextColor);

		AS_LOG(Warning, TEXT("PlayerName: %s"), *PlayerState->GetPlayerName());
	}

	if (KillCountTextBlock != nullptr)
	{
		KillCountTextBlock->SetText(FText::FromString(FString::FromInt(PlayerState->GetKillCount())));
		KillCountTextBlock->SetColorAndOpacity(TextColor);
	}
}

void UASDmRankingSlotUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RankTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("RankTextBlock")));
	NameTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("NameTextBlock")));
	KillCountTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("KillCountTextBlock")));
}
