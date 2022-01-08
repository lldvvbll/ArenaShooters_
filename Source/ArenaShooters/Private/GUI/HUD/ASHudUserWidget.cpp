// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/HUD/ASHudUserWidget.h"
#include "GUI/HUD/ASInventoryStatusUserWidget.h"
#include "GUI/HUD/ASKillDeathCaptionUserWidget.h"
#include "GUI/HUD/ASKillLogUserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Border.h"
#include "Controller/ASPlayerController.h"
#include "Character/ASCharacter.h"
#include "Character/ASStatusComponent.h"
#include "GameMode/ASMatchGameStateBase.h"
#include "Controller/ASPlayerState.h"
#include "Common/ASEnums.h"

void UASHudUserWidget::StopFinishTimer()
{
	bSetMatchFinishTime = false;
}

void UASHudUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InventoryStatusWidget = Cast<UASInventoryStatusUserWidget>(GetWidgetFromName(TEXT("InventoryStatusWidget")));
	FinishCountDownBorder = Cast<UBorder>(GetWidgetFromName(TEXT("FinishCountDownBorder")));
	FinishCountDownTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("FinishCountDownTextBlock")));
	KillLogWidget = Cast<UASKillLogUserWidget>(GetWidgetFromName(TEXT("KillLogWidget")));
	KillDeathCaptionWidget = Cast<UASKillDeathCaptionUserWidget>(GetWidgetFromName(TEXT("KillDeathCaptionWidget")));
	HealthProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("HealthProgressBar")));

	bSetMatchFinishTime = false;
	MatchFinishTime = FDateTime::MaxValue();

	if (FinishCountDownBorder != nullptr)
	{
		FinishCountDownBorder->SetVisibility(ESlateVisibility::Hidden);
	}

	if (KillDeathCaptionWidget != nullptr)
	{
		KillDeathCaptionWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
	if (ensure(GameState))
	{
		GameState->OnSetMatchFinishTime.AddUObject(this, &UASHudUserWidget::OnSetMatchFinishTime);
		GameState->OnKill.AddUObject(this, &UASHudUserWidget::OnKill);
	}

	auto ASPlayerController = GetOwningPlayer<AASPlayerController>();
	if (ensure(IsValid(ASPlayerController)))
	{
		auto Char = ASPlayerController->GetPawn<AASCharacter>();
		if (ensure(IsValid(Char)))
		{
			auto StatusComp = Char->GetStatusComponent();
			if (ensure(StatusComp != nullptr))
			{
				StatusComp->OnChangeCurrentHealth.AddUObject(this, &UASHudUserWidget::OnChangedCharacterHealth);

				MaxCharHealth = StatusComp->GetMaxHealth();
				OnChangedCharacterHealth(StatusComp->GetCurrentHealth());
			}
		}
	}
}

void UASHudUserWidget::NativeDestruct()
{
	Super::NativeDestruct();

	auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
	if (ensure(GameState))
	{
		GameState->OnSetMatchFinishTime.RemoveAll(this);
		GameState->OnKill.RemoveAll(this);
	}
}

void UASHudUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bSetMatchFinishTime)
	{
		if (FinishCountDownTextBlock != nullptr)
		{
			FTimespan RemainTime = MatchFinishTime - FDateTime::Now();

			int32 RemainMinutes = RemainTime.GetMinutes();
			if (RemainMinutes < 0)
			{
				RemainMinutes = 0;
			}

			int32 RemainSeconds = RemainTime.GetSeconds();
			if (RemainSeconds < 0)
			{
				RemainSeconds = 0;
			}

			FinishCountDownTextBlock->SetText(FText::FromString(FString::Printf(TEXT("%d : %d"), RemainMinutes, RemainSeconds)));
		}
	}
}

void UASHudUserWidget::OnSetMatchFinishTime(float Time)
{
	auto GameState = GetWorld()->GetGameState<AASMatchGameStateBase>();
	if (ensure(GameState))
	{
		bSetMatchFinishTime = true;

		float DeltaTime = Time - GameState->GetServerWorldTimeSeconds();
		MatchFinishTime = FDateTime::Now() + FTimespan::FromSeconds(DeltaTime);

		if (FinishCountDownBorder != nullptr)
		{
			FinishCountDownBorder->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
}

void UASHudUserWidget::OnKill(AASPlayerState* KillerPlayerState, AASPlayerState* DeadPlayerState, int32 KillCount)
{
	auto MyPlayerState = GetOwningPlayer()->GetPlayerState<AASPlayerState>();
	if (!ensure(IsValid(MyPlayerState)))
		return;

	FString KillerName = IsValid(KillerPlayerState) ? *KillerPlayerState->GetPlayerName() : TEXT("Unknown");
	FString DeadName = IsValid(DeadPlayerState) ? *DeadPlayerState->GetPlayerName() : TEXT("Unknown");

	EKillLogType KillLogType = EKillLogType::Other;
	if (KillDeathCaptionWidget != nullptr)
	{
		if (IsValid(KillerPlayerState) && MyPlayerState->GetPlayerId() == KillerPlayerState->GetPlayerId())
		{
			KillLogType = EKillLogType::MyKill;
			KillDeathCaptionWidget->ShowKillCaption(DeadName, KillCount);

		}
		else if (IsValid(DeadPlayerState) && MyPlayerState->GetPlayerId() == DeadPlayerState->GetPlayerId())
		{
			KillLogType = EKillLogType::MyDeath;
			KillDeathCaptionWidget->ShowDeadCaption(KillerName);
		}
	}

	if (KillLogWidget != nullptr)
	{
		KillLogWidget->AddLog(KillerName, DeadName, KillLogType);
	}
}

void UASHudUserWidget::OnChangedCharacterHealth(float NewHealth) const
{
	if (HealthProgressBar != nullptr)
	{
		HealthProgressBar->SetPercent(NewHealth / MaxCharHealth);
	}
}
