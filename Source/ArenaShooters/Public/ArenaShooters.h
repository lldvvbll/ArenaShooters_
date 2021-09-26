// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "EngineMinimal.h"
#include "UObject/UObjectGlobals.h"

DECLARE_LOG_CATEGORY_EXTERN(ArenaShooters, Log, All);

#define AS_LOG_CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))
#define AS_LOG_S(Verbosity) UE_LOG(ArenaShooters, Verbosity, TEXT("%s"), *AS_LOG_CALLINFO)
#define AS_LOG(Verbosity, Format, ...) UE_LOG(ArenaShooters, Verbosity, TEXT("%s %s"), *AS_LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
//#define return_if(Expr, ...) { if (Expr) { LOG(Error, TEXT("ASSERTION: %s"), TEXT("'"#Expr"'")); return __VA_ARGS__; } }

#define AS_LOG_SCREEN(Time, Color, Format, ...) { if (GEngine != nullptr) { GEngine->AddOnScreenDebugMessage(INDEX_NONE, Time, Color, FString::Printf(Format, ##__VA_ARGS__)); } }
#define AS_LOG_SCREEN_S(Time, Color) { if (GEngine != nullptr) { GEngine->AddOnScreenDebugMessage(INDEX_NONE, Time, Color, AS_LOG_CALLINFO); } }

#define AS_LOG_A(Verbosity, Time, Format, ...) { AS_LOG(Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__)); AS_LOG_SCREEN(Time, ((ELogVerbosity::Verbosity == ELogVerbosity::Error) ? FColor::Red : FColor::Yellow), TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__)); }
#define AS_LOG_A_S(Verbosity, Time) { AS_LOG_S(Verbosity); AS_LOG_SCREEN(Time, ((ELogVerbosity::Verbosity == ELogVerbosity::Error) ? FColor::Red : FColor::Yellow), TEXT("%s"), *AS_LOG_CALLINFO); }
