// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ASStatusComponent.h"
#include "Net/UnrealNetwork.h"

UASStatusComponent::UASStatusComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);
}

void UASStatusComponent::InitializeComponent()
{
	Super::InitializeComponent();

	CurrentHealth = MaxHealth;
	CurrentStemina = MaxStemina;
}

void UASStatusComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UASStatusComponent, CurrentHealth, COND_AutonomousOnly);
	DOREPLIFETIME_CONDITION(UASStatusComponent, CurrentStemina, COND_AutonomousOnly);
}

float UASStatusComponent::GetMaxHealth() const
{
	return MaxHealth;
}

float UASStatusComponent::GetCurrentHealth() const
{
	return CurrentHealth;
}

void UASStatusComponent::SetCurrentHealth(float NewHealth)
{
	CurrentHealth = FMath::Clamp<float>(NewHealth, 0.0f, MaxHealth);

	OnChangeCurrentHealth.Broadcast(CurrentHealth);

	if (CurrentHealth < KINDA_SMALL_NUMBER)
	{
		OnHealthZero.Broadcast();
	}

	AS_LOG(Warning, TEXT("Current Health : %f"), CurrentHealth);
}

void UASStatusComponent::ModifyCurrentHealth(float Value)
{
	SetCurrentHealth(CurrentHealth + Value);
}

float UASStatusComponent::GetMaxStemina() const
{
	return MaxStemina;
}

float UASStatusComponent::GetCurrentStemina() const
{
	return CurrentHealth;
}

void UASStatusComponent::SetCurrentStemina(float NewStemina)
{
	CurrentStemina = FMath::Clamp<float>(NewStemina, 0.0f, MaxStemina);
}

void UASStatusComponent::ModifyCurrentStemina(float Value)
{
	SetCurrentStemina(CurrentStemina + Value);
}

void UASStatusComponent::OnRep_CurrentHealth()
{
	OnChangeCurrentHealth.Broadcast(CurrentHealth);
}

void UASStatusComponent::OnRep_CurrentStemina()
{
	AS_LOG_SCREEN(1.0f, FColor::Yellow, TEXT("CurrentStemina: %f"), CurrentStemina);
}
