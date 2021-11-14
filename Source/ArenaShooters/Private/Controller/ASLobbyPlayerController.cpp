// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/ASLobbyPlayerController.h"
#include "GUI/Lobby/ASMainMenuUserWidget.h"
#include "GUI/Lobby/ASServerBrowserUserWidget.h"
#include "GUI/ASNotificationUserWidget.h"
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

void AASLobbyPlayerController::NotifyMessage(const FString& Message, float Duration/* = 5.0f*/)
{
	auto NotiWidget = CreateWidget<UASNotificationUserWidget>(this, NotificationWidgetClass);
	if (NotiWidget != nullptr)
	{
		NotiWidget->AddToViewport(5);

		NotiWidget->SetInfo(Message, Duration);
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
				NotifyMessage(ErrorMsg);
				GameInstance->ClearNetworkFailureMessage();
			}
		}
	}
}
