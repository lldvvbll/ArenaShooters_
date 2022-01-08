// Fill out your copyright notice in the Description page of Project Settings.


#include "ASGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Controller/ASLobbyPlayerController.h"
#include "GameFramework/GameSession.h"

void UASGameInstance::Init()
{
	Super::Init();

	IOnlineSubsystem* OS = IOnlineSubsystem::Get();
	if (ensure(OS != nullptr))
	{
		IOnlineSessionPtr SessionInterface = OS->GetSessionInterface();
		if (ensure(SessionInterface.IsValid()))
		{
			AS_LOG(Warning, TEXT("OnlineSubsystem: %s"), *(OS->GetSubsystemName().ToString()));

			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UASGameInstance::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UASGameInstance::OnFindSessionComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UASGameInstance::OnJoinSessionComplete);
			SessionInterface->OnRegisterPlayersCompleteDelegates.AddUObject(this, &UASGameInstance::OnRegisterPlayersComplete);
			SessionInterface->OnUnregisterPlayersCompleteDelegates.AddUObject(this, &UASGameInstance::OnUnregisterPlayersComplete);
		}
	}
}

void UASGameInstance::SearchServer()
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (ensure(SessionInterface.IsValid()))
	{
		SessionSearch = MakeShareable(new FOnlineSessionSearch());
		SessionSearch->MaxSearchResults = 200000;
		SessionSearch->TimeoutInSeconds = 60.0f;

		if (!IsOnlineSubsystemSteam())
		{
			SessionSearch->bIsLanQuery = true;
		}

		//SessionSearch->QuerySettings.Set(NUMOPENSEAT, 1, EOnlineComparisonOp::GreaterThanEquals);
		//SessionSearch->QuerySettings.Set(PREPARED_MATCH, false, EOnlineComparisonOp::Equals);

		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UASGameInstance::JoinServer(const FOnlineSessionSearchResult& SearchResult)
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (ensure(SessionInterface.IsValid() && SearchResult.IsValid()))
	{
		FNamedOnlineSession* NamedSession = SessionInterface->GetNamedSession(NAME_GameSession);
		if (ensure(NamedSession == nullptr))
		{
			SessionInterface->JoinSession(0, NAME_GameSession, SearchResult);
		}	
	}
}

void UASGameInstance::SetPreparedMatchToSession(bool bPrepared)
{
	if (!ensure(GetWorld()->GetNetMode() == NM_DedicatedServer))
		return;

	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (!ensure(SessionInterface.IsValid()))
		return;	

	FOnlineSessionSettings* SessionSettings = SessionInterface->GetSessionSettings(NAME_GameSession);
	if (SessionSettings == nullptr)		// nullable
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

void UASGameInstance::DestroySession()
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (ensure(SessionInterface.IsValid()))
	{
		FNamedOnlineSession* NamedSession = SessionInterface->GetNamedSession(NAME_GameSession);
		if (ensure(NamedSession != nullptr))
		{
			SessionInterface->DestroySession(NAME_GameSession);
		}
	}
}

void UASGameInstance::OnStart()
{
	Super::OnStart();

	if (GIsServer)
	{
		IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
		if (ensure(SessionInterface.IsValid()))
		{
			FString ServerName = TEXT("ArenaShooters(Test Server)");

			TArray<FString> Tokens;
			TArray<FString> Switches;
			FCommandLine::Parse(FCommandLine::Get(), Tokens, Switches);
			for (auto& PairStr : Switches)
			{
				FString Key;
				FString Value;
				UGameplayStatics::GetKeyValue(PairStr, Key, Value);

				if (Key.Compare(TEXT("servername"), ESearchCase::IgnoreCase) == 0)
				{
					ServerName = Value.Replace(TEXT("\""), TEXT(""));
					break;
				}
			}

			FString LoadedMapName = GetWorld()->GetMapName();
			if (LoadedMapName.IsEmpty())
			{
				LoadedMapName = TEXT("Unknown");
			}

			int32 NumMaxPlayers = 16;
			auto GameMode = GetWorld()->GetAuthGameMode();
			if (ensure(GameMode != nullptr) && ensure(GameMode->GameSession != nullptr))
			{
				NumMaxPlayers = GameMode->GameSession->MaxPlayers;
			}

			FOnlineSessionSettings SessionSettings;
			SessionSettings.bAllowJoinInProgress = true;
			SessionSettings.bShouldAdvertise = true;
			SessionSettings.NumPublicConnections = NumMaxPlayers;
			SessionSettings.Set(SERVER_NAME, ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
			SessionSettings.Set(SETTING_MAPNAME, LoadedMapName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
			SessionSettings.Set(NUMOPENPUBCONN, NumMaxPlayers, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
			SessionSettings.Set(PREPARED_MATCH, false, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

			if (IsOnlineSubsystemSteam())
			{
				SessionSettings.bIsDedicated = true;
			}
			else
			{
				SessionSettings.bIsLANMatch = true;
			}

			ensure(SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings));
		}
	}
	else if (GIsClient)
	{
		if (!IsOnlineSubsystemSteam())
		{
			auto PlayerCtrlr = GetWorld()->GetFirstPlayerController<AASLobbyPlayerController>();
			if (ensure(PlayerCtrlr != nullptr))
			{
				PlayerCtrlr->NotifyMessage(TEXT("You are not logged in to Steam.\nPlease log in to Steam and restart the game."), 10.0f);
			}
		}

		GEngine->OnNetworkFailure().AddUObject(this, &UASGameInstance::BroadcastNetworkFailure);
	}
}

void UASGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (ensure(bWasSuccessful))
	{
		AS_LOG_S(Warning);
	}
}

void UASGameInstance::OnFindSessionComplete(bool bWasSuccessful)
{
	if (ensure(bWasSuccessful))
	{
		if (ensure(SessionSearch != nullptr))
		{
			TArray<FOnlineSessionSearchResult> FilteredResults = FilterSessionResults(SessionSearch->SearchResults, SessionSearch->QuerySettings);
			OnSearchSessionResult.Broadcast(FilteredResults);
		}
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
	if (!ensure(bWasSuccessful))
		return;

	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (!ensure(SessionInterface.IsValid()))
		return;

	FOnlineSessionSettings* SessionSettings = SessionInterface->GetSessionSettings(SessionName);
	if (!ensure(SessionSettings != nullptr))
		return;

	FNamedOnlineSession* NamedOnlineSession = SessionInterface->GetNamedSession(SessionName);
	if (!ensure(NamedOnlineSession != nullptr))
		return;

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
	if (!ensure(bWasSuccessful))
		return;

	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (!ensure(SessionInterface.IsValid()))
		return;

	FOnlineSessionSettings* SessionSettings = SessionInterface->GetSessionSettings(SessionName);
	if (!ensure(SessionSettings != nullptr))
		return;

	FNamedOnlineSession* NamedOnlineSession = SessionInterface->GetNamedSession(SessionName);
	if (!ensure(NamedOnlineSession != nullptr))
		return;

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
	if (!ensure(SessionInterface.IsValid()))
		return;

	FString JoinAddress;
	SessionInterface->GetResolvedConnectString(SessionName, JoinAddress);
	if (!ensure(!JoinAddress.IsEmpty()))
		return;

	auto PlayerCtrlr = GetWorld()->GetFirstPlayerController<AASLobbyPlayerController>();
	if (!ensure(IsValid(PlayerCtrlr)))
		return;

	PlayerCtrlr->ClientTravel(JoinAddress, ETravelType::TRAVEL_Absolute);
}

void UASGameInstance::BroadcastNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	DestroySession();

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
			bool bA = false;
			bool bB = false;
			A.GetValue(bA);
			B.GetValue(bB);

			switch (Comparator)
			{
			case EOnlineComparisonOp::Equals:
				return bA == bB;
			case EOnlineComparisonOp::NotEquals:
				return bA != bB;
			default:
				break;
			}
		}
	case EOnlineKeyValuePairDataType::Double:
		{
			double bA = 0.0;
			double bB = 0.0;
			A.GetValue(bA);
			B.GetValue(bB);

			switch (Comparator)
			{
			case EOnlineComparisonOp::Equals:
				return bA == bB;
			case EOnlineComparisonOp::NotEquals:
				return bA != bB;
			case EOnlineComparisonOp::GreaterThanEquals:
				return (bA == bB || bA > bB);
			case EOnlineComparisonOp::LessThanEquals:
				return (bA == bB || bA < bB);
			case EOnlineComparisonOp::GreaterThan:
				return bA > bB;
			case EOnlineComparisonOp::LessThan:
				return bA < bB;
			default:
				break;
			}
		}
	case EOnlineKeyValuePairDataType::Float:
		{
			float tbA = 0.0f;
			float tbB = 0.0f;
			A.GetValue(tbA);
			B.GetValue(tbB);
			double bA = static_cast<double>(tbA);
			double bB = static_cast<double>(tbB);

			switch (Comparator)
			{
			case EOnlineComparisonOp::Equals:
				return bA == bB;
			case EOnlineComparisonOp::NotEquals:
				return bA != bB;
			case EOnlineComparisonOp::GreaterThanEquals:
				return (bA == bB || bA > bB);
			case EOnlineComparisonOp::LessThanEquals:
				return (bA == bB || bA < bB);
			case EOnlineComparisonOp::GreaterThan:
				return bA > bB;
			case EOnlineComparisonOp::LessThan:
				return bA < bB;
			default:
				break;
			}
		}
	case EOnlineKeyValuePairDataType::Int32:
		{
			int32 bA = 0;
			int32 bB = 0;
			A.GetValue(bA);
			B.GetValue(bB);

			switch (Comparator)
			{
			case EOnlineComparisonOp::Equals:
				return bA == bB;
			case EOnlineComparisonOp::NotEquals:
				return bA != bB;
			case EOnlineComparisonOp::GreaterThanEquals:
				return (bA == bB || bA > bB);
			case EOnlineComparisonOp::LessThanEquals:
				return (bA == bB || bA < bB);
			case EOnlineComparisonOp::GreaterThan:
				return bA > bB;
			case EOnlineComparisonOp::LessThan:
				return bA < bB;
			default:
				break;
			}
		}
	case EOnlineKeyValuePairDataType::Int64:
		{
			uint64 bA = 0;
			uint64 bB = 0;
			A.GetValue(bA);
			B.GetValue(bB);

			switch (Comparator)
			{
			case EOnlineComparisonOp::Equals:
				return bA == bB;
			case EOnlineComparisonOp::NotEquals:
				return bA != bB;
			case EOnlineComparisonOp::GreaterThanEquals:
				return (bA == bB || bA > bB);
			case EOnlineComparisonOp::LessThanEquals:
				return (bA == bB || bA < bB);
			case EOnlineComparisonOp::GreaterThan:
				return bA > bB;
			case EOnlineComparisonOp::LessThan:
				return bA < bB;
			default:
				break;
			}
		}
	case EOnlineKeyValuePairDataType::String:
		{
			FString bA;
			FString bB;
			A.GetValue(bA);
			B.GetValue(bB);

			switch (Comparator)
			{
			case EOnlineComparisonOp::Equals:
				return bA == bB;
			case EOnlineComparisonOp::NotEquals:
				return bA != bB;
			default:
				break;
			}
		}
	case EOnlineKeyValuePairDataType::Empty:
	case EOnlineKeyValuePairDataType::Blob:
	default:
		break;
	}

	return false;
}
