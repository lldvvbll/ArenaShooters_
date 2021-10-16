// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/ASStructs.h"
#include "Controller/ASPlayerState.h"

FRankedPlayerState::FRankedPlayerState(int32 InRanking, AASPlayerState* InPlayerState)
	: Ranking(InRanking), PlayerState(InPlayerState)
{
}
