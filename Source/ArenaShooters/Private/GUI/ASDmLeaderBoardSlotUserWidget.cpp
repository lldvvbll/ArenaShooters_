// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/ASDmLeaderBoardSlotUserWidget.h"
#include "Controller/ASPlayerController.h"
#include "Controller/ASPlayerState.h"
#include "Components/TextBlock.h"

void UASDmLeaderBoardSlotUserWidget::SetInfo(int32 Ranking, AASPlayerState* PlayerState)
{
	if (!ensure(IsValid(PlayerState)))
		return;

	PlayerStatePtr = MakeWeakObjectPtr(PlayerState);
	if (!ensure(PlayerStatePtr.IsValid()))
		return;

	auto OwningPlayerState = GetOwningPlayer()->GetPlayerState<AASPlayerState>();
	if (!ensure(IsValid(OwningPlayerState)))
		return;

	PlayerStatePtr->OnChangedPlayerName.AddUObject(this, &UASDmLeaderBoardSlotUserWidget::OnChangedPlayerName);
	PlayerStatePtr->OnChangedKillCount.AddUObject(this, &UASDmLeaderBoardSlotUserWidget::OnChangedPlayerKillCount);
	PlayerStatePtr->OnChangedDeathCount.AddUObject(this, &UASDmLeaderBoardSlotUserWidget::OnChangedPlayerDeathCount);

	FLinearColor TextColor = (OwningPlayerState->GetPlayerId() == PlayerStatePtr->GetPlayerId() ? MyPlayerStateTextColor : OtherPlayerStateTextColor);

	if (RankingTextBlock != nullptr)
	{
		RankingTextBlock->SetColorAndOpacity(TextColor);
		RankingTextBlock->SetText(FText::FromString(FString::FromInt(Ranking)));
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

	if (DeathCountTextBlock != nullptr)
	{
		DeathCountTextBlock->SetColorAndOpacity(TextColor);
		OnChangedPlayerDeathCount(PlayerStatePtr->GetDeathCount());
	}
}

void UASDmLeaderBoardSlotUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RankingTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("RankingTextBlock")));
	NameTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("NameTextBlock")));
	KillCountTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("KillCountTextBlock")));
	DeathCountTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("DeathCountTextBlock")));
}

void UASDmLeaderBoardSlotUserWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (PlayerStatePtr.IsValid())
	{
		PlayerStatePtr->OnChangedPlayerName.RemoveAll(this);
		PlayerStatePtr->OnChangedKillCount.RemoveAll(this);
		PlayerStatePtr->OnChangedDeathCount.RemoveAll(this);
	}	
}

void UASDmLeaderBoardSlotUserWidget::OnChangedPlayerName(FString NewName)
{
	if (NameTextBlock != nullptr)
	{
		NameTextBlock->SetText(FText::FromString(NewName));
	}
}

void UASDmLeaderBoardSlotUserWidget::OnChangedPlayerKillCount(int32 NewCount)
{
	if (KillCountTextBlock != nullptr)
	{
		KillCountTextBlock->SetText(FText::FromString(FString::FromInt(NewCount)));
	}
}

void UASDmLeaderBoardSlotUserWidget::OnChangedPlayerDeathCount(int32 NewCount)
{
	if (DeathCountTextBlock != nullptr)
	{
		DeathCountTextBlock->SetText(FText::FromString(FString::FromInt(NewCount)));
	}
}
