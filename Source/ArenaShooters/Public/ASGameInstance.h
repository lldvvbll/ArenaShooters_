// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Engine/GameInstance.h"
#include "ASGameInstance.generated.h"

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

protected:
	virtual void OnStart() override;

	virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	virtual void OnFindSessionComplete(bool bWasSuccessful);
	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

public:
	DECLARE_EVENT_OneParam(UASGameInstance, FOnSearchSessionResultEvent, const TArray<FOnlineSessionSearchResult>&);
	FOnSearchSessionResultEvent OnSearchSessionResult;

protected:
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

};
