// Fill out your copyright notice in the Description page of Project Settings.


#include "GUI/Lobby/ASServerSlotUserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "OnlineSessionSettings.h"
#include "ASGameInstance.h"

void UASServerSlotUserWidget::SetServerInfo(const FOnlineSessionSearchResult& NewSearchResult)
{
	SearchResult = NewSearchResult;

	FString ServerName = TEXT("Unknown");
	ensure(SearchResult.Session.SessionSettings.Get(SERVER_NAME, ServerName));

	FString ServerMap = TEXT("Unknown");
	ensure(SearchResult.Session.SessionSettings.Get(SETTING_MAPNAME, ServerMap));

	int32 NumOpenPublicConnections = 0;
	ensure(SearchResult.Session.SessionSettings.Get(NUMOPENPUBCONN, NumOpenPublicConnections));

	if (ServerNameTextBlock != nullptr)
	{
		ServerNameTextBlock->SetText(FText::FromString(ServerName));
	}

	if (ServerMapTextBlock != nullptr)
	{
		ServerMapTextBlock->SetText(FText::FromString(ServerMap));
	}

	if (PlayersTextBlock != nullptr)
	{
		int32 MaxConnection = SearchResult.Session.SessionSettings.NumPublicConnections;
		int32 CurConnection = MaxConnection - NumOpenPublicConnections;

		PlayersTextBlock->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), CurConnection, MaxConnection)));
	}

	if (PingTextBlock != nullptr)
	{
		PingTextBlock->SetText(FText::FromString(FString::Printf(TEXT("%d ms"), SearchResult.PingInMs)));		
	}

	if (JoinButtonTextBlock != nullptr)
	{
		bool bPrepared = false;
		ensure(SearchResult.Session.SessionSettings.Get(PREPARED_MATCH, bPrepared));

		if (bPrepared)
		{
			JoinButtonTextBlock->SetText(FText::FromString(TEXT("In Progress")));

			SetButtonEnable(false);
		}
		else if (NumOpenPublicConnections < 1)
		{
			JoinButtonTextBlock->SetText(FText::FromString(TEXT("Full")));

			SetButtonEnable(false);
		}
	}
}

void UASServerSlotUserWidget::SetButtonEnable(bool bIsEnable)
{
	if (JoinButton != nullptr)
	{
		JoinButton->SetIsEnabled(bIsEnable);
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
	JoinButtonTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("TB_JoinButton")));

	if (JoinButton != nullptr)
	{
		JoinButton->OnClicked.AddDynamic(this, &UASServerSlotUserWidget::JoinServer);
	}
}

void UASServerSlotUserWidget::JoinServer()
{
	if (auto GameInst = GetGameInstance<UASGameInstance>())
	{
		GameInst->JoinServer(SearchResult);
	}

	OnButtonClicked.Broadcast();
}
