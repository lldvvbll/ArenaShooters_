// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "ASStructs.generated.h"

class AASPlayerState;

USTRUCT()
struct ARENASHOOTERS_API FRankedPlayerState
{
	GENERATED_BODY()

public:
	FRankedPlayerState() = default;
	FRankedPlayerState(int32 InRanking, AASPlayerState* InPlayerState);

public:
	UPROPERTY()
	int32 Ranking;

	UPROPERTY()
	AASPlayerState* PlayerState;
};
