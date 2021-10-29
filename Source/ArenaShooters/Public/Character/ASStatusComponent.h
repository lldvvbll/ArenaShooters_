// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaShooters.h"
#include "Components/ActorComponent.h"
#include "ASStatusComponent.generated.h"


UCLASS()
class ARENASHOOTERS_API UASStatusComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UASStatusComponent();

	virtual void InitializeComponent() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	float GetMaxHealth() const;
	float GetCurrentHealth() const;
	void SetCurrentHealth(float NewHealth);
	void ModifyCurrentHealth(float Value);

	float GetMaxStemina() const;
	float GetCurrentStemina() const;
	void SetCurrentStemina(float NewStemina);
	void ModifyCurrentStemina(float Value);

	void SetStatusDefaults();

	UFUNCTION()
	void OnRep_CurrentHealth();
	
	UFUNCTION()
	void OnRep_CurrentStemina();

public:
	DECLARE_EVENT_OneParam(UASStatusComponent, FOnChangeCurrentHealthEvent, float);
	FOnChangeCurrentHealthEvent OnChangeCurrentHealth;

	DECLARE_EVENT(UASStatusComponent, FOnHealthZeroEvent);
	FOnHealthZeroEvent OnHealthZero;

private:
	UPROPERTY(EditDefaultsOnly, Meta = (AllowPrivateAccess = true))
	float MaxHealth;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;

	UPROPERTY(EditDefaultsOnly, Meta = (AllowPrivateAccess = true))
	float MaxStemina;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentStemina)
	float CurrentStemina;
};
