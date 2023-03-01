// Fill out your copyright notice in the Description page of Project Settings.

#include "AIShooter_OpenFire.h"
#include "DEnemyShooter.h"

UAIShooter_OpenFire* UAIShooter_OpenFire::AIShooterFire(UObject* WorldContextObject, ADEnemyShooter* InAIShooter, AActor* InHostile)
{
	UAIShooter_OpenFire* Shooter_OpenFire = NewObject<UAIShooter_OpenFire>(WorldContextObject);
	if (Shooter_OpenFire)
	{
		Shooter_OpenFire->Hostile = InHostile;
		Shooter_OpenFire->AIShooter = InAIShooter;
	}
	
	return Shooter_OpenFire;
}

void UAIShooter_OpenFire::Activate()
{
	if (AIShooter && Hostile)
	{
		NumberOfFire = AIShooter->RandomFiringRange.GetRandomInt();
		float Interval = 1 / AIShooter->FirePerMinute / 60.f;

		GetWorld()->GetTimerManager().SetTimer(Handle_Firing, this, &UAIShooter_OpenFire::OnFiring, Interval, true, 0.f);
	}
	else
	{
		OnCompleted.Broadcast();
	}
}

void UAIShooter_OpenFire::OnFiring()
{
	NumberOfFire--;

	//AIShooter->OpenFire(Hostile);

	if (NumberOfFire == 0)
	{
		OnCompleted.Broadcast();
		GetWorld()->GetTimerManager().ClearTimer(Handle_Firing);
	}
}
