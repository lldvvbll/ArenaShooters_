// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/ASRespawnTimerUserWidget.h"
#include "Components/TextBlock.h"
#include "GameMode/ASMatchGameStateBase.h"

void UASRespawnTimerUserWidget::SetEndTime(const FDateTime& Time)
{
	bSetEndTime = true;
	EndTime = Time;
}

void UASRespawnTimerUserWidget::SetEndTimeByServerWorldTime(float TimeSec)
{
	auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
	if (IsValid(GameState))
	{
		float DeltaTime = TimeSec - GameState->GetServerWorldTimeSeconds();
		SetEndTime(FDateTime::Now() + FTimespan::FromSeconds(DeltaTime));
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void UASRespawnTimerUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CountDownTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("TB_CountDown")));
}

void UASRespawnTimerUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
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
