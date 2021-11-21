// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/HUD/ASDmRankingSlotUserWidget.h"
#include "Controller/ASPlayerState.h"
#include "Components/TextBlock.h"

void UASDmRankingSlotUserWidget::SetPlayerInfo(int32 Rank, AASPlayerState* PlayerState)
{
	if (!IsValid(PlayerState))
		return;

	PlayerStatePtr = MakeWeakObjectPtr(PlayerState);
	if (!ensure(PlayerStatePtr.IsValid()))
		return;

	auto OwningPlayerState = GetOwningPlayer()->GetPlayerState<AASPlayerState>();
	if (!ensure(IsValid(OwningPlayerState)))
		return;

	PlayerStatePtr->OnChangedPlayerName.AddUObject(this, &UASDmRankingSlotUserWidget::OnChangedPlayerName);
	PlayerStatePtr->OnChangedKillCount.AddUObject(this, &UASDmRankingSlotUserWidget::OnChangedPlayerKillCount);

	bool bMyPlayerState = OwningPlayerState->GetPlayerId() == PlayerStatePtr->GetPlayerId();
	FLinearColor TextColor = bMyPlayerState ? MyPlayerStateTextColor : OtherPlayerStateTextColor;

	if (RankTextBlock != nullptr)
	{
		RankTextBlock->SetColorAndOpacity(TextColor);
		RankTextBlock->SetText(FText::FromString(FString::FromInt(Rank)));
	}

	if (NameTextBlock != nullptr)
	{
		NameTextBlock->SetColorAndOpacity(TextColor);
		OnChangedPlayerName(PlayerStatePtr->GetPlayerName());
	}

	if (KillCountTextBlock != nullptr)
	{
		KillCountTextBlock->SetColorAndOpacity(TextColor);
		OnChangedPlayerKillCount(PlayerStatePtr->GetKillCount());
	}
}

void UASDmRankingSlotUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RankTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("RankTextBlock")));
	NameTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("NameTextBlock")));
	KillCountTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("KillCountTextBlock")));
}

void UASDmRankingSlotUserWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (PlayerStatePtr.IsValid())
	{
		PlayerStatePtr->OnChangedPlayerName.RemoveAll(this);
		PlayerStatePtr->OnChangedKillCount.RemoveAll(this);
	}
}

void UASDmRankingSlotUserWidget::OnChangedPlayerName(FString NewName)
{
	if (NameTextBlock != nullptr)
	{
		NameTextBlock->SetText(FText::FromString(NewName));
	}
}

void UASDmRankingSlotUserWidget::OnChangedPlayerKillCount(int32 NewCount)
{
	if (KillCountTextBlock != nullptr)
	{
		KillCountTextBlock->SetText(FText::FromString(FString::FromInt(NewCount)));
	}
}
