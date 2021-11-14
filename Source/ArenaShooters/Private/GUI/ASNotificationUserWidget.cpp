// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/ASNotificationUserWidget.h"
#include "Components/TextBlock.h"

void UASNotificationUserWidget::SetInfo(const FString& Message, float Duration)
{
	if (MessageTextBlock != nullptr)
	{
		MessageTextBlock->SetText(FText::FromString(Message));
	}

	if (Duration > 0.0f)
	{
		ExpireTime = FDateTime::Now() + FTimespan::FromSeconds(Duration);
		bSetDuration = true;
	}
}

void UASNotificationUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	MessageTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("TB_Message")));
}

void UASNotificationUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bSetDuration)
	{
		FTimespan RemainTime = ExpireTime - FDateTime::Now();
		int32 RemainSeconds = RemainTime.GetTotalSeconds();
		if (RemainSeconds <= 0)
		{
			RemainSeconds = 0;
			bSetDuration = false;

			RemoveFromParent();
		}
	}
}
