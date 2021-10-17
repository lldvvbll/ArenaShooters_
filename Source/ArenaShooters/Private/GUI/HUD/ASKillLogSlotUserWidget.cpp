// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/HUD/ASKillLogSlotUserWidget.h"
#include "Components/TextBlock.h"

void UASKillLogSlotUserWidget::SetInfo(const FString& KillerName, const FString& DeadName, int32 MaxNameLen, float LifeTimeSec, FLinearColor LogColor)
{
	if (KillerNameTextBlock != nullptr)
	{
		KillerNameTextBlock->SetColorAndOpacity(LogColor);

		if (KillerName.Len() > MaxNameLen)
		{
			KillerNameTextBlock->SetText(FText::FromString(KillerName.Left(MaxNameLen - 3) + TEXT("...")));
		}
		else
		{
			KillerNameTextBlock->SetText(FText::FromString(KillerName));
		}
	}

	if (DeadNameTextBlock != nullptr)
	{
		DeadNameTextBlock->SetColorAndOpacity(LogColor);
		
		if (DeadName.Len() > MaxNameLen)
		{
			DeadNameTextBlock->SetText(FText::FromString(DeadName.Left(MaxNameLen - 3) + TEXT("...")));
		}
		else
		{
			DeadNameTextBlock->SetText(FText::FromString(DeadName));
		}
	}
	
	if (ArrowTextBlock != nullptr)
	{
		ArrowTextBlock->SetColorAndOpacity(LogColor);
	}

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UASKillLogSlotUserWidget::RemoveLog, LifeTimeSec);
}

void UASKillLogSlotUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	KillerNameTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("KillerNameTextBlock")));
	DeadNameTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("DeadNameTextBlock")));
	ArrowTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("ArrowTextBlock")));
}

void UASKillLogSlotUserWidget::RemoveLog()
{
	RemoveFromParent();
}