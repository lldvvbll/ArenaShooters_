// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/ASTimerCaptionUserWidget.h"
#include "Components/TextBlock.h"
#include "GameMode/ASMatchGameStateBase.h"

void UASTimerCaptionUserWidget::SetInfo(const FText& Caption, const FDateTime& Time)
{
	bSetEndTime = true;
	EndTime = Time;

	if (CaptionTextBlock != nullptr)
	{
		CaptionTextBlock->SetText(Caption);
	}
}

void UASTimerCaptionUserWidget::SetInfo(const FText& Caption, float TimeSec)
{
	auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
	if (IsValid(GameState))
	{
		float DeltaTime = TimeSec - GameState->GetServerWorldTimeSeconds();
		SetInfo(Caption, FDateTime::Now() + FTimespan::FromSeconds(DeltaTime));
	}
	else
	{
		AS_LOG_S(Error);
	}
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
		if (CountDownTextBlock != nullptr)
		{
			FTimespan RemainTime = EndTime - FDateTime::Now();
			int32 RemainSeconds = RemainTime.GetTotalSeconds();
			if (RemainSeconds <= 0)
			{
				RemainSeconds = 0;
				bSetEndTime = false;

				RemoveFromParent();
			}

			CountDownTextBlock->SetText(FText::FromString(FString::FromInt(RemainSeconds)));
		}
	}
}
