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

		SessionSearch->QuerySettings.Set(NUMOPENPUBCONN, 1, EOnlineComparisonOp::GreaterThanEquals);
		SessionSearch->QuerySettings.Set(PREPARED_MATCH, false, EOnlineComparisonOp::Equals);

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
		FNamedOnlineSession* NamedSession = SessionInterface->GetNamedSession(NAME_GameSession);
		if (NamedSession == nullptr)
		{
			SessionInterface->JoinSession(0, NAME_GameSession, SearchResult);
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

void UASGameInstance::SetPreparedMatchToSession(bool bPrepared)
{
	if (GetWorld()->GetNetMode() != NM_DedicatedServer)
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

	FOnlineSessionSettings* SessionSettings = SessionInterface->GetSessionSettings(NAME_GameSession);
	if (SessionSettings == nullptr)
		return;

	SessionSettings->Set(PREPARED_MATCH, bPrepared, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	SessionInterface->UpdateSession(NAME_GameSession, *SessionSettings);
}

const FString& UASGameInstance::GetNetworkFailureMessage() const
{
	return NetworkFailureMessage;
}

void UASGameInstance::ClearNetworkFailureMessage()
{
	NetworkFailureMessage.Empty();
}

void UASGameInstance::OnStart()
{
	Super::OnStart();

	if (GIsServer)
	{
		IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
		if (SessionInterface.IsValid())
		{
			FString LoadedMapName = GetWorld()->GetMapName();
			if (LoadedMapName.IsEmpty())
			{
				LoadedMapName = TEXT("Unknown");
			}

			FOnlineSessionSettings SessionSettings;
			SessionSettings.bAllowJoinInProgress = true;
			SessionSettings.bShouldAdvertise = true;
			SessionSettings.NumPublicConnections = 16;
			SessionSettings.Set(SERVER_NAME, FString(TEXT("Test Server Name")), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
			SessionSettings.Set(SETTING_MAPNAME, LoadedMapName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
			SessionSettings.Set(NUMOPENPUBCONN, SessionSettings.NumPublicConnections, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
			SessionSettings.Set(PREPARED_MATCH, false, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

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
	else if (GIsClient)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &UASGameInstance::BroadcastNetworkFailure);
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
			TArray<FOnlineSessionSearchResult> FilteredResults = FilterSessionResults(SessionSearch->SearchResults, SessionSearch->QuerySettings);
			OnSearchSessionResult.Broadcast(FilteredResults);
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

	TravelBySession(SessionName);
}

void UASGameInstance::OnRegisterPlayersComplete(FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& PlayerIds, bool bWasSuccessful)
{
	if (!bWasSuccessful)
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

	FOnlineSessionSettings* SessionSettings = SessionInterface->GetSessionSettings(SessionName);
	if (SessionSettings == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	FNamedOnlineSession* NamedOnlineSession = SessionInterface->GetNamedSession(SessionName);
	if (NamedOnlineSession == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (IsOnlineSubsystemSteam())
	{
		if (NamedOnlineSession->NumOpenPublicConnections > 0)
		{
			NamedOnlineSession->NumOpenPublicConnections--;
		}
	}

	SessionSettings->Set(NUMOPENPUBCONN, NamedOnlineSession->NumOpenPublicConnections, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	SessionInterface->UpdateSession(SessionName, *SessionSettings);

	AS_LOG(Warning, TEXT("NumOpenPublicConnections: %d"), NamedOnlineSession->NumOpenPublicConnections);
}

void UASGameInstance::OnUnregisterPlayersComplete(FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& PlayerIds, bool bWasSuccessful)
{
	if (!bWasSuccessful)
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

	FOnlineSessionSettings* SessionSettings = SessionInterface->GetSessionSettings(SessionName);
	if (SessionSettings == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	FNamedOnlineSession* NamedOnlineSession = SessionInterface->GetNamedSession(SessionName);
	if (NamedOnlineSession == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	if (IsOnlineSubsystemSteam())
	{
		if (NamedOnlineSession->NumOpenPublicConnections > 0)
		{
			NamedOnlineSession->NumOpenPublicConnections++;
		}
	}

	SessionSettings->Set(NUMOPENPUBCONN, NamedOnlineSession->NumOpenPublicConnections, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	SessionInterface->UpdateSession(SessionName, *SessionSettings);

	AS_LOG(Warning, TEXT("NumOpenPublicConnections: %d"), NamedOnlineSession->NumOpenPublicConnections);
}

void UASGameInstance::TravelBySession(FName SessionName)
{
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

	auto PlayerCtrlr = Cast<AASLobbyPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (PlayerCtrlr == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	PlayerCtrlr->ClientTravel(JoinAddress, ETravelType::TRAVEL_Absolute);
}

void UASGameInstance::BroadcastNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(World);
	if (!SessionInterface.IsValid())
	{
		AS_LOG_S(Error);
		return;
	}

	FNamedOnlineSession* NamedSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (NamedSession != nullptr)
	{
		SessionInterface->DestroySession(NAME_GameSession);
	}

	NetworkFailureMessage = ErrorString;
}

bool UASGameInstance::IsOnlineSubsystemSteam() const
{
	IOnlineSubsystem* OS = IOnlineSubsystem::Get();
	if (OS == nullptr)
		return false;

	return (OS->GetSubsystemName() == TEXT("Steam"));
}

TArray<FOnlineSessionSearchResult> UASGameInstance::FilterSessionResults(const TArray<FOnlineSessionSearchResult>& SearchResults, const FOnlineSearchSettings& SearchSettings) const
{
	TArray<FOnlineSessionSearchResult> FilteredResults;

	const FSearchParams& SearchParams = SearchSettings.SearchParams;
	if (SearchParams.Num() > 0)
	{
		for (auto& Result : SearchResults)
		{
			if (!Result.IsValid())
				continue;

			bool bAddResult = true;
			for (auto& Param : SearchParams)
			{
				const FOnlineSessionSetting* Setting = Result.Session.SessionSettings.Settings.Find(Param.Key);
				if (Setting != nullptr)
				{
					if (!CompareVariants(Setting->Data, Param.Value.Data, Param.Value.ComparisonOp))
					{
						bAddResult = false;
						break;
					}
				}
			}
			
			if (bAddResult)
			{
				FilteredResults.Emplace(Result);
			}			
		}
	}
	else
	{
		FilteredResults.Append(SearchResults);
	}

	return FilteredResults;
}

bool UASGameInstance::CompareVariants(const FVariantData& A, const FVariantData& B, EOnlineComparisonOp::Type Comparator) const
{
	if (A.GetType() != B.GetType())
		return false;

	switch (A.GetType())
	{
	case EOnlineKeyValuePairDataType::Bool:
		{
			bool bA, bB;
			A.GetValue(bA);
			B.GetValue(bB);
			switch (Comparator)
			{
			case EOnlineComparisonOp::Equals:
				return bA == bB; break;
			case EOnlineComparisonOp::NotEquals:
				return bA != bB; break;
			default:
				return false; break;
			}
		}
	case EOnlineKeyValuePairDataType::Double:
		{
			double bA, bB;
			A.GetValue(bA);
			B.GetValue(bB);
			switch (Comparator)
			{
			case EOnlineComparisonOp::Equals:
				return bA == bB; break;
			case EOnlineComparisonOp::NotEquals:
				return bA != bB; break;
			case EOnlineComparisonOp::GreaterThanEquals:
				return (bA == bB || bA > bB); break;
			case EOnlineComparisonOp::LessThanEquals:
				return (bA == bB || bA < bB); break;
			case EOnlineComparisonOp::GreaterThan:
				return bA > bB; break;
			case EOnlineComparisonOp::LessThan:
				return bA < bB; break;
			default:
				return false; break;
			}
		}
	case EOnlineKeyValuePairDataType::Float:
		{
			float tbA, tbB;
			double bA, bB;
			A.GetValue(tbA);
			B.GetValue(tbB);
			bA = (double)tbA;
			bB = (double)tbB;
			switch (Comparator)
			{
			case EOnlineComparisonOp::Equals:
				return bA == bB; break;
			case EOnlineComparisonOp::NotEquals:
				return bA != bB; break;
			case EOnlineComparisonOp::GreaterThanEquals:
				return (bA == bB || bA > bB); break;
			case EOnlineComparisonOp::LessThanEquals:
				return (bA == bB || bA < bB); break;
			case EOnlineComparisonOp::GreaterThan:
				return bA > bB; break;
			case EOnlineComparisonOp::LessThan:
				return bA < bB; break;
			default:
				return false; break;
			}
		}
	case EOnlineKeyValuePairDataType::Int32:
		{
			int32 bA, bB;
			A.GetValue(bA);
			B.GetValue(bB);
			switch (Comparator)
			{
			case EOnlineComparisonOp::Equals:
				return bA == bB; break;
			case EOnlineComparisonOp::NotEquals:
				return bA != bB; break;
			case EOnlineComparisonOp::GreaterThanEquals:
				return (bA == bB || bA > bB); break;
			case EOnlineComparisonOp::LessThanEquals:
				return (bA == bB || bA < bB); break;
			case EOnlineComparisonOp::GreaterThan:
				return bA > bB; break;
			case EOnlineComparisonOp::LessThan:
				return bA < bB; break;
			default:
				return false; break;
			}
		}
	case EOnlineKeyValuePairDataType::Int64:
		{
			uint64 bA, bB;
			A.GetValue(bA);
			B.GetValue(bB);
			switch (Comparator)
			{
			case EOnlineComparisonOp::Equals:
				return bA == bB; break;
			case EOnlineComparisonOp::NotEquals:
				return bA != bB; break;
			case EOnlineComparisonOp::GreaterThanEquals:
				return (bA == bB || bA > bB); break;
			case EOnlineComparisonOp::LessThanEquals:
				return (bA == bB || bA < bB); break;
			case EOnlineComparisonOp::GreaterThan:
				return bA > bB; break;
			case EOnlineComparisonOp::LessThan:
				return bA < bB; break;
			default:
				return false; break;
			}
		}

	case EOnlineKeyValuePairDataType::String:
		{
			FString bA, bB;
			A.GetValue(bA);
			B.GetValue(bB);
			switch (Comparator)
			{
			case EOnlineComparisonOp::Equals:
				return bA == bB; break;
			case EOnlineComparisonOp::NotEquals:
				return bA != bB; break;
			default:
				return false; break;
			}
		}

	case EOnlineKeyValuePairDataType::Empty:
	case EOnlineKeyValuePairDataType::Blob:
	default:
		return false; break;
	}

	return false;
}
