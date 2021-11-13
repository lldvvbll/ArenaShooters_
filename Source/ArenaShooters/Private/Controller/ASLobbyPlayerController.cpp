// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/ASLobbyPlayerController.h"
#include "GUI/Lobby/ASMainMenuUserWidget.h"
#include "GUI/Lobby/ASServerBrowserUserWidget.h"
#include "GUI/ASTimerCaptionUserWidget.h"
#include "ASGameInstance.h"

void AASLobbyPlayerController::ShowMainMenu()
{
	if (MainMenuWidget != nullptr)
	{
		MainMenuWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		MainMenuWidget = CreateWidget<UASMainMenuUserWidget>(this, MainMenuWidgetClass);
		if (MainMenuWidget != nullptr)
		{
			MainMenuWidget->AddToViewport();
		}
	}	
}

void AASLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		ShowMainMenu();

		if (auto GameInstance = GetGameInstance<UASGameInstance>())
		{
			FString ErrorMsg = GameInstance->GetNetworkFailureMessage();
			if (!ErrorMsg.IsEmpty())
			{
				ShowCaption(ErrorMsg);
				GameInstance->ClearNetworkFailureMessage();
			}
		}
	}
}

void AASLobbyPlayerController::ShowCaption(const FString& Caption, float CaptionLifeTimeSec/* = 5.0f*/)
{
	auto CaptionWidget = CreateWidget<UASTimerCaptionUserWidget>(this, TimerCaptionUserWidgetClass);
	if (CaptionWidget != nullptr)
	{
		CaptionWidget->AddToViewport(5);

		CaptionWidget->SetInfoWithDuration(FText::FromString(Caption), CaptionLifeTimeSec, false);
	}
}
