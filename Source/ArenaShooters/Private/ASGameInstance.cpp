// Fill out your copyright notice in the Description page of Project Settings.


#include "ASGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Controller/ASLobbyPlayerController.h"

void UASGameInstance::Init()
{
	Super::Init();

	IOnlineSubsystem* OS = IOnlineSubsystem::Get();
	if (OS != nullptr)
	{
		IOnlineSessionPtr SessionInterface = OS->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			AS_LOG(Warning, TEXT("OnlineSubsystem: %s"), *(OS->GetSubsystemName().ToString()));

			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UASGameInstance::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UASGameInstance::OnFindSessionComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UASGameInstance::OnJoinSessionComplete);
			SessionInterface->OnRegisterPlayersCompleteDelegates.AddUObject(this, &UASGameInstance::OnRegisterPlayersComplete);
			SessionInterface->OnUnregisterPlayersCompleteDelegates.AddUObject(this, &UASGameInstance::OnUnregisterPlayersComplete);
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

void UASGameInstance::SearchServer()
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface.IsValid())
	{
		SessionSearch = MakeShareable(new FOnlineSessionSearch());
		SessionSearch->MaxSearchResults = 200000;
		SessionSearch->TimeoutInSeconds = 60.0f;

		//if (FString(FCommandLine::Get()).Find(TEXT("-searchlan")) != INDEX_NONE)
		if (!IsOnlineSubsystemSteam())
		{
			SessionSearch->bIsLanQuery = true;
			//SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		}

		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void UASGameInstance::JoinServer(const FOnlineSessionSearchResult& SearchResult)
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface.IsValid() && SearchResult.IsValid())
	{
		SessionInterface->JoinSession(0, NAME_GameSession, SearchResult);
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void UASGameInstance::OnStart()
{
	Super::OnStart();

	if (!GIsClient)
	{
		IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
		if (SessionInterface.IsValid())
		{
			FOnlineSessionSettings SessionSettings;
			SessionSettings.bAllowJoinInProgress = true;
			SessionSettings.bShouldAdvertise = true;
			SessionSettings.NumPublicConnections = 16;
			SessionSettings.Set(SERVER_NAME, FString(TEXT("Test Server Name")), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
			SessionSettings.Set(SETTING_MAPNAME, FString(TEXT("Test Server Map")), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
			SessionSettings.Set(NUMOPENPUBCONN, FString::FromInt(SessionSettings.NumPublicConnections), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
			
			//if (FString(FCommandLine::Get()).Find(TEXT("-lan")) != INDEX_NONE)
			if (!IsOnlineSubsystemSteam())
			{
				SessionSettings.bIsLANMatch = true;
				//SessionSettings.bUsesPresence = true;
			}
			else
			{
				SessionSettings.bIsDedicated = true;
			}

			if (!SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings))
			{
				AS_LOG_S(Error);
			}
		}
		else
		{
			AS_LOG_S(Error);
		}
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
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		AS_LOG(Error, TEXT("OnJoinSessionComplete. SessionName: %s, Result: %d"), *SessionName.ToString(), Result);
		return;
	}

	auto PlayerCtrlr = Cast<AASLobbyPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (PlayerCtrlr == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (!SessionInterface.IsValid())
	{
		AS_LOG_S(Error);
		return;
	}
		
	FString JoinAddress;
	SessionInterface->GetResolvedConnectString(SessionName, JoinAddress);
	if (JoinAddress.IsEmpty())
	{
		AS_LOG_S(Error);
		return;
	}

	PlayerCtrlr->ClientTravel(JoinAddress, ETravelType::TRAVEL_Absolute);
}

void UASGameInstance::OnRegisterPlayersComplete(FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& PlayerIds, bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (!SessionInterface.IsValid())
	{
		AS_LOG_S(Error);
		return;
	}

	FOnlineSessionSettings* SessionSettings = SessionInterface->GetSessionSettings(SessionName);
	if (SessionSettings == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	FString NumOpenPublicConnections;
	if (SessionSettings->Get(NUMOPENPUBCONN, NumOpenPublicConnections))
	{
		int32 NewNum = FCString::Atoi(*NumOpenPublicConnections) - 1;
		SessionSettings->Set(NUMOPENPUBCONN, FString::FromInt(NewNum), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		SessionInterface->UpdateSession(SessionName, *SessionSettings);
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void UASGameInstance::OnUnregisterPlayersComplete(FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& PlayerIds, bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (!SessionInterface.IsValid())
	{
		AS_LOG_S(Error);
		return;
	}

	FOnlineSessionSettings* SessionSettings = SessionInterface->GetSessionSettings(SessionName);
	if (SessionSettings == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	FString NumOpenPublicConnections;
	if (SessionSettings->Get(NUMOPENPUBCONN, NumOpenPublicConnections))
	{
		int32 NewNum = FCString::Atoi(*NumOpenPublicConnections) + 1;
		SessionSettings->Set(NUMOPENPUBCONN, FString::FromInt(NewNum), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		SessionInterface->UpdateSession(SessionName, *SessionSettings);
	}
	else
	{
		AS_LOG_S(Error);
	}
}

bool UASGameInstance::IsOnlineSubsystemSteam() const
{
	IOnlineSubsystem* OS = IOnlineSubsystem::Get();
	if (OS == nullptr)
		return false;

	return (OS->GetSubsystemName() == TEXT("Steam"));
}
