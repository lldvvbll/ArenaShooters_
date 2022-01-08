// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/ASTimerCaptionUserWidget.h"
#include "Components/TextBlock.h"
#include "GameMode/ASMatchGameStateBase.h"

void UASTimerCaptionUserWidget::SetInfo(const FText& Caption, const FDateTime& InEndTime, bool bShowTime/* = true*/)
{
	bSetEndTime = true;
	EndTime = InEndTime;

	if (CaptionTextBlock != nullptr)
	{
		CaptionTextBlock->SetText(Caption);
	}

	if (CountDownTextBlock != nullptr)
	{
		if (!bShowTime)
		{
			CountDownTextBlock->SetVisibility(ESlateVisibility::Hidden);
		}		
	}
}

void UASTimerCaptionUserWidget::SetInfoWithEndTime(const FText& Caption, float EndTimeSec, bool bShowTime/* = true*/)
{
	auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
	if (ensure(GameState))
	{
		float DeltaTime = EndTimeSec - GameState->GetServerWorldTimeSeconds();
		SetInfoWithDuration(Caption, DeltaTime, bShowTime);
	}
}

void UASTimerCaptionUserWidget::SetInfoWithDuration(const FText& Caption, float Duration, bool bShowTime)
{
	SetInfo(Caption, FDateTime::Now() + FTimespan::FromSeconds(Duration), bShowTime);
}

void UASTimerCaptionUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CaptionTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("TB_Caption")));
	CountDownTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("TB_CountDown")));
}

void UASTimerCaptionUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bSetEndTime)
	{
		FTimespan RemainTime = EndTime - FDateTime::Now();
		int32 RemainSeconds = RemainTime.GetTotalSeconds();
		if (RemainSeconds <= 0)
		{
			RemainSeconds = 0;
			bSetEndTime = false;

			RemoveFromParent();
		}
		
		if (CountDownTextBlock != nullptr)
		{
			CountDownTextBlock->SetText(FText::FromString(FString::FromInt(RemainSeconds)));
		}
	}
}
