// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/HUD/ASKillLogUserWidget.h"
#include "GUI/HUD/ASKillLogSlotUserWidget.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"

void UASKillLogUserWidget::AddLog(const FString& KillerName, const FString& DeadName, EKillLogType KillLogType)
{
	if (KillLogScrollBox == nullptr)
		return;

	KillLogScrollBox->ClearChildren();

	if (auto KillLogSlotUserWidget = CreateWidget<UASKillLogSlotUserWidget>(this, KillLogSlotUserWidgetClass))
	{
		FLinearColor LogColor;
		switch (KillLogType)
		{
		case EKillLogType::Other:
			LogColor = OtherKillLogColor;
			break;
		case EKillLogType::MyKill:
			LogColor = MyKillLogColor;
			break;
		case EKillLogType::MyDeath:
			LogColor = MyDeathLogColor;
			break;
		default:
			checkNoEntry();
			break;
		}

		KillLogSlotUserWidget->SetInfo(KillerName, DeadName, MaxNameLen, LogLifeTimeSec, LogColor);

		CachedLogWidgets.EmplaceAt(0, KillLogSlotUserWidget);

		while (CachedLogWidgets.Num() > MaxLineNum)
		{
			CachedLogWidgets.RemoveAt(CachedLogWidgets.Num() - 1, 1, false);
		}
		
		for (auto& LogWidget : CachedLogWidgets)
		{
			KillLogScrollBox->AddChild(LogWidget);
		}
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void UASKillLogUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	KillLogScrollBox = Cast<UScrollBox>(GetWidgetFromName(TEXT("KillLogScrollBox")));

	if (KillLogScrollBox != nullptr)
	{
		KillLogScrollBox->ClearChildren();
	}

	CachedLogWidgets.Reserve(MaxLineNum);
}