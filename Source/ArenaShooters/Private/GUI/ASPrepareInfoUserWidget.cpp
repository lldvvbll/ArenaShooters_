// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/ASPrepareInfoUserWidget.h"
#include "Components/TextBlock.h"
#include "GameMode/ASMatchGameStateBase.h"

void UASPrepareInfoUserWidget::SetMaxNumPlayers(int32 Num)
{
	if (MaxNumPlayersTextBlock != nullptr)
	{
		MaxNumPlayersTextBlock->SetText(FText::FromString(FString::FromInt(Num)));
	}
}

void UASPrepareInfoUserWidget::SetNumPlayers(int32 Num)
{
	if (NumPlayersTextBlock != nullptr)
	{
		NumPlayersTextBlock->SetText(FText::FromString(FString::FromInt(Num)));
	}
}

void UASPrepareInfoUserWidget::StartCountDown(float InMatchStartTime)
{
	auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
	if (ensure(IsValid(GameState)))
	{
		bCountDown = true;

		float DeltaTime = InMatchStartTime - GameState->GetServerWorldTimeSeconds();
		MatchStartTime = FDateTime::Now() + FTimespan::FromSeconds(DeltaTime);

		if (MatchStartInTextBlock != nullptr)
		{
			MatchStartInTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
		}

		if (CountDownTextBlock != nullptr)
		{
			CountDownTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
}

void UASPrepareInfoUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	MaxNumPlayersTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("MaxNumPlayersTextBlock")));
	NumPlayersTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("NumPlayersTextBlock")));
	MatchStartInTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("MatchStartInTextBlock")));
	CountDownTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("CountDownTextBlock")));
	
	bCountDown = false;
	MatchStartTime = FDateTime::MaxValue();

	if (MatchStartInTextBlock != nullptr)
	{
		MatchStartInTextBlock->SetVisibility(ESlateVisibility::Hidden);
	}

	if (CountDownTextBlock != nullptr)
	{
		CountDownTextBlock->SetVisibility(ESlateVisibility::Hidden);
	}

	auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
	if (ensure(IsValid(GameState)))
	{
		GameState->OnStartTimeForProcess.AddUObject(this, &UASPrepareInfoUserWidget::StartCountDown);
		GameState->OnAddedPlayerState.AddUObject(this, &UASPrepareInfoUserWidget::OnAddedPlayerState);
		GameState->OnRemovedPlayerState.AddUObject(this, &UASPrepareInfoUserWidget::OnRemovedPlayerState);

		SetNumPlayers(GameState->GetNumPlayers());
		SetMaxNumPlayers(GameState->GetMaxNumPlayer());
	}
}

void UASPrepareInfoUserWidget::NativeDestruct()
{
	Super::NativeDestruct();

	auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
	if (ensure(IsValid(GameState)))
	{
		GameState->OnStartTimeForProcess.RemoveAll(this);
		GameState->OnAddedPlayerState.RemoveAll(this);
		GameState->OnRemovedPlayerState.RemoveAll(this);
	}
}

void UASPrepareInfoUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bCountDown)
	{
		if (CountDownTextBlock != nullptr)
		{
			FTimespan RemainTime = MatchStartTime - FDateTime::Now();
			int32 RemainSeconds = RemainTime.GetTotalSeconds();
			if (RemainSeconds < 0)
			{
				RemainSeconds = 0;
			}

			CountDownTextBlock->SetText(FText::FromString(FString::FromInt(RemainSeconds)));
		}
	}
}

void UASPrepareInfoUserWidget::OnAddedPlayerState(APlayerState* AddedPlayerState)
{
	auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
	if (ensure(IsValid(GameState)))
	{
		SetNumPlayers(GameState->PlayerArray.Num());
	}
}

void UASPrepareInfoUserWidget::OnRemovedPlayerState(APlayerState* AddedPlayerState)
{
	auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
	if (ensure(IsValid(GameState)))
	{
		SetNumPlayers(GameState->PlayerArray.Num());
	}
}
