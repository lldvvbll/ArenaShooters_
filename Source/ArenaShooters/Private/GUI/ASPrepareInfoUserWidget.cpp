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
	if (IsValid(GameState))
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
	else
	{
		AS_LOG_S(Error);
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
	if (IsValid(GameState))
	{
		if (GameState->GetInnerMatchState() == EInnerMatchState::Prepare)
		{
			GameState->OnStartTimeForProcess.AddUObject(this, &UASPrepareInfoUserWidget::StartCountDown);
			GameState->OnChangedNumPlayers.AddUObject(this, &UASPrepareInfoUserWidget::SetNumPlayers);

			SetNumPlayers(GameState->GetNumPlayers());
			SetMaxNumPlayers(GameState->GetMaxNumPlayer());
		}
		else
		{
			AS_LOG_S(Error);
		}
	}
	else
	{
		AS_LOG_S(Error);
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
