// Fill out your copyright notice in the Description page of Project Settings.


#include "DPlayerState.h"
#include "UnrealNetwork.h"

ADPlayerState::ADPlayerState()
	: TotalDamage(0),
	  Kills(0)
{
	bReplicates = true;
}

void ADPlayerState::RecordDamage(uint32 Damage)
{
	TotalDamage += Damage;
}

void ADPlayerState::IncreaseKills()
{
	Kills++;
}

void ADPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADPlayerState, Kills);
	DOREPLIFETIME(ADPlayerState, TotalDamage);
}
