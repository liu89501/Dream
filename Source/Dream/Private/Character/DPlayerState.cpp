// Fill out your copyright notice in the Description page of Project Settings.


#include "DPlayerState.h"
#include "UnrealNetwork.h"

ADPlayerState::ADPlayerState()
{

}

void ADPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
