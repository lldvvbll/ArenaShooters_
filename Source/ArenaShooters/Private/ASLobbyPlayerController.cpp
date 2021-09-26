// Fill out your copyright notice in the Description page of Project Settings.


#include "ASLobbyPlayerController.h"
#include "GUI/Lobby/ASMainMenuUserWidget.h"
#include "GUI/Lobby/ASServerBrowserUserWidget.h"

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

void AASLobbyPlayerController::ShowServerBrowser()
{
	ServerBrowserWidget = CreateWidget<UASServerBrowserUserWidget>(this, ServerBrowserWidgetClass);
	if (ServerBrowserWidget != nullptr)
	{
		ServerBrowserWidget->AddToViewport();
	}
}

void AASLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		ShowMainMenu();
	}
}
