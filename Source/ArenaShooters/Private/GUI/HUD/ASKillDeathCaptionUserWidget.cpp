// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/HUD/ASKillDeathCaptionUserWidget.h"
#include "Components/TextBlock.h"

void UASKillDeathCaptionUserWidget::ShowKillCaption(const FString& DeadPlayerName, int32 KillCount)
{
	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (KillDeathCaptionTextBlock != nullptr)
	{
		KillDeathCaptionTextBlock->SetText(FText::FromString(FString::Printf(TEXT("You killed %s"), *DeadPlayerName)));
	}

	if (KillCountTextBlock != nullptr)
	{
		KillCountTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
		KillCountTextBlock->SetText(FText::FromString(FString::FromInt(KillCount)));
	}

	SetTimer();
}

void UASKillDeathCaptionUserWidget::ShowDeadCaption(const FString& KillerPlayerName)
{
	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (KillDeathCaptionTextBlock != nullptr)
	{
		KillDeathCaptionTextBlock->SetText(FText::FromString(FString::Printf(TEXT("You were killed by %s"), *KillerPlayerName)));
	}

	if (KillCountTextBlock != nullptr)
	{
		KillCountTextBlock->SetVisibility(ESlateVisibility::Hidden);
	}

	SetTimer();
}

void UASKillDeathCaptionUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	KillDeathCaptionTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("KillDeathCaptionTextBlock")));
	KillCountTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("KillCountTextBlock")));
}

void UASKillDeathCaptionUserWidget::SetTimer()
{
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UASKillDeathCaptionUserWidget::Hide, DurationSec);
}

void UASKillDeathCaptionUserWidget::Hide()
{
	SetVisibility(ESlateVisibility::Hidden);
}
