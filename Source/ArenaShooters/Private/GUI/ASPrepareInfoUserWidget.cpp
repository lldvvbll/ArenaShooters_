// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/ASPrepareInfoUserWidget.h"
#include "Components/TextBlock.h"

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

void UASPrepareInfoUserWidget::StartCountDown(FDateTime InMatchStartTime)
{
	bCountDown = true;
	MatchStartTime = InMatchStartTime;

	if (MatchStartInTextBlock != nullptr)
	{
		MatchStartInTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	if (CountDownTextBlock != nullptr)
	{
		CountDownTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
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
}

void UASPrepareInfoUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bCountDown)
	{
		if (CountDownTextBlock != nullptr)
		{
			FTimespan RemainTime = MatchStartTime - FDateTime::Now();
			int32 RemainSeconds = RemainTime.GetSeconds();
			if (RemainSeconds < 0)
			{
				RemainSeconds = 0;
			}

			CountDownTextBlock->SetText(FText::FromString(FString::FromInt(RemainSeconds)));
		}
	}
}
