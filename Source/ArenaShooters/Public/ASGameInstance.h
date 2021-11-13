// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Engine/GameInstance.h"
#include "ASGameInstance.generated.h"

#define NUMOPENPUBCONN FName(TEXT("NUMOPENPUBCONN"))
#define SERVER_NAME FName(TEXT("SERVER_NAME"))
#define PREPARED_MATCH FName(TEXT("PREPARED_MATCH"))

class FOnlineSessionSearch;
class FOnlineSessionSearchResult;
class FVariantData;
class FOnlineSearchSettings;

namespace EOnJoinSessionCompleteResult
{
	enum Type;
}

namespace EOnlineComparisonOp
{
	enum Type;
}

UCLASS()
class ARENASHOOTERS_API UASGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;

	void SearchServer();
	void JoinServer(const FOnlineSessionSearchResult& SearchResult);

	void SetPreparedMatchToSession(bool bPrepared);

	const FString& GetNetworkFailureMessage() const;
	void ClearNetworkFailureMessage();

protected:
	virtual void OnStart() override;

	virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	virtual void OnFindSessionComplete(bool bWasSuccessful);
	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	virtual void OnRegisterPlayersComplete(FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& PlayerIds, bool bWasSuccessful);
	virtual void OnUnregisterPlayersComplete(FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& PlayerIds, bool bWasSuccessful);

	void TravelBySession(FName SessionName);

	void BroadcastNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString = TEXT(""));

	bool IsOnlineSubsystemSteam() const;

	TArray<FOnlineSessionSearchResult> FilterSessionResults(const TArray<FOnlineSessionSearchResult>& SearchResults, const FOnlineSearchSettings& SearchSettings) const;
	bool CompareVariants(const FVariantData& A, const FVariantData& B, EOnlineComparisonOp::Type Comparator) const;

public:
	DECLARE_EVENT_OneParam(UASGameInstance, FOnSearchSessionResultEvent, const TArray<FOnlineSessionSearchResult>&);
	FOnSearchSessionResultEvent OnSearchSessionResult;

protected:
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FString NetworkFailureMessage;
};
