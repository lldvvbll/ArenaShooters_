// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ASActionComponent.h"
#include "Common/ASEnums.h"

UASActionComponent::UASActionComponent()
{
	//PrimaryComponentTick.bCanEverTick = true;

	CurrentMovementState = EMovementState::Grounded;
}

void UASActionComponent::SetMovementState(EMovementState NewState)
{
	CurrentMovementState = NewState;
}
