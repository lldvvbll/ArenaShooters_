// Fill out your copyright notice in the Description page of Project Settings.


#include "ASGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"

void UASGameInstance::Init()
{
	Super::Init();

	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface.IsValid())
	{
		AS_LOG(Warning, TEXT("OnlineSubsystem: %s"), *IOnlineSubsystem::Get()->GetSubsystemName().ToString());

		SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UASGameInstance::OnCreateSessionComplete);
		SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UASGameInstance::OnFindSessionComplete);
		SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UASGameInstance::OnJoinSessionComplete);
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void UASGameInstance::SearchServer()
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface.IsValid())
	{
		SessionSearch = MakeShareable(new FOnlineSessionSearch());
		SessionSearch->MaxSearchResults = 200000;
		SessionSearch->TimeoutInSeconds = 60.0f;
		SessionSearch->bIsLanQuery = (FString(FCommandLine::Get()).Find(TEXT("-searchlan")) != INDEX_NONE);
		//SessionSearch->QuerySettings.Set(SEARCH_DEDICATED_ONLY, true, EOnlineComparisonOp::Equals);

		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UASGameInstance::OnStart()
{
	Super::OnStart();

	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface.IsValid())
	{
		FOnlineSessionSettings SessionSettings;
		SessionSettings.bAllowJoinInProgress = true;
		SessionSettings.bIsDedicated = true;
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.NumPublicConnections = 16;
		SessionSettings.bIsLANMatch = (FString(FCommandLine::Get()).Find(TEXT("-lan")) != INDEX_NONE);
		SessionSettings.Set(FName(TEXT("SERVER_NAME")), FString(TEXT("Test Server Name")), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		SessionSettings.Set(SETTING_MAPNAME, FString(TEXT("Test Server Map")), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		if (!SessionInterface->CreateSession(0, FName(TEXT("My Session")), SessionSettings))
		{
			AS_LOG_S(Error);
		}
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void UASGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		AS_LOG_S(Warning);
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void UASGameInstance::OnFindSessionComplete(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (SessionSearch != nullptr)
		{
			OnSearchSessionResult.Broadcast(SessionSearch->SearchResults);
		}
		else
		{
			AS_LOG_S(Error);
		}
	}
	else
	{
		AS_LOG_S(Error);
	}	
}

void UASGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{

}
