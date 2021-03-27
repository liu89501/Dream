// Fill out your copyright notice in the Description page of Project Settings.


#include "DPlayerState.h"
#include "UnrealNetwork.h"

ADPlayerState::ADPlayerState()
	: OneTeamColor(FLinearColor(.1f, .46f, .53f)),
	TwoTeamColor(FLinearColor(.75f, .75f, .18f))
{

}

void ADPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADPlayerState, KillNum);
	DOREPLIFETIME(ADPlayerState, DieNum);
	DOREPLIFETIME(ADPlayerState, Team);
}

FLinearColor ADPlayerState::GetTeamColor() const
{
	return Team == ETeamName::One_Team ? OneTeamColor : TwoTeamColor;
}
