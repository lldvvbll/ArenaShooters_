// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/Lobby/ASServerSlotUserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "OnlineSessionSettings.h"

void UASServerSlotUserWidget::SetServerInfo(const FOnlineSessionSearchResult& NewSearchResult)
{
	SearchResult = NewSearchResult;

	if (ServerNameTextBlock != nullptr)
	{
		FString ServerName = TEXT("Unknown");
		SearchResult.Session.SessionSettings.Get(FName(TEXT("SERVER_NAME")), ServerName);

		ServerNameTextBlock->SetText(FText::FromString(ServerName));
	}

	if (ServerMapTextBlock != nullptr)
	{
		FString ServerMap = TEXT("Unknown");
		SearchResult.Session.SessionSettings.Get(SETTING_MAPNAME, ServerMap);

		ServerMapTextBlock->SetText(FText::FromString(ServerMap));
	}

	if (PlayersTextBlock != nullptr)
	{
		int32 MaxConnection = SearchResult.Session.SessionSettings.NumPublicConnections;
		int32 CurConnection = MaxConnection - SearchResult.Session.NumOpenPublicConnections;

		PlayersTextBlock->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), CurConnection, MaxConnection)));
	}

	if (PingTextBlock != nullptr)
	{
		PingTextBlock->SetText(FText::FromString(FString::Printf(TEXT("%d ms"), SearchResult.PingInMs)));		
	}

	if (JoinButton != nullptr)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UASServerSlotUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ServerNameTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("ServerNameTextBlock")));
	ServerMapTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("ServerMapTextBlock")));
	PlayersTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("PlayersTextBlock")));
	PingTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("PingTextBlock")));
	JoinButton = Cast<UButton>(GetWidgetFromName(TEXT("JoinButton")));

	if (JoinButton != nullptr)
	{
		JoinButton->SetIsEnabled(false);
		JoinButton->OnClicked.AddDynamic(this, &UASServerSlotUserWidget::JoinServer);
	}
}

void UASServerSlotUserWidget::JoinServer()
{
	AS_LOG_S(Warning);
}
