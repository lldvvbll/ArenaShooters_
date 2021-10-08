// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Engine/GameInstance.h"
#include "Common/ASEnums.h"
#include "ASGameInstance.generated.h"

#define NUMOPENPUBCONN "NUMOPENPUBCONN"
#define SERVER_NAME FName(TEXT("SERVER_NAME"))

class FOnlineSessionSearch;
class FOnlineSessionSearchResult;

namespace EOnJoinSessionCompleteResult
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

	EInnerMatchState GetInnerMatchState() const;
	void SetInnerMatchState(EInnerMatchState State);
	bool IsMatchProcess() const;

protected:
	virtual void OnStart() override;

	virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	virtual void OnFindSessionComplete(bool bWasSuccessful);
	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	virtual void OnRegisterPlayersComplete(FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& PlayerIds, bool bWasSuccessful);
	virtual void OnUnregisterPlayersComplete(FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& PlayerIds, bool bWasSuccessful);

protected:
	bool IsOnlineSubsystemSteam() const;

public:
	DECLARE_EVENT_OneParam(UASGameInstance, FOnSearchSessionResultEvent, const TArray<FOnlineSessionSearchResult>&);
	FOnSearchSessionResultEvent OnSearchSessionResult;

protected:
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	EInnerMatchState InnerMatchState;
};
