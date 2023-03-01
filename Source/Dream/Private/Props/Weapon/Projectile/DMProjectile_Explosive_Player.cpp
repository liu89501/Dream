// Fill out your copyright notice in the Description page of Project Settings.

#include "DMProjectile_Explosive_Player.h"

#include "Kismet/GameplayStatics.h"

ADMProjectile_Explosive_Player::ADMProjectile_Explosive_Player()
	: InnerCameraShakeRadius(300.f)
	, OuterCameraShakeRadius(2000.f)
{
}

void ADMProjectile_Explosive_Player::OnRep_Impact()
{
	Super::OnRep_Impact();

	APawn* InstigatorPawn = GetInstigator();
	if (InstigatorPawn && InstigatorPawn->IsLocallyControlled())
	{
		APlayerController* PlayerController = Cast<APlayerController>(InstigatorPawn->Controller);

		float Distance = FVector::Distance(InstigatorPawn->GetActorLocation(), GetActorLocation());
		if (Distance < OuterCameraShakeRadius)
		{
			float Scale = FMath::Min((OuterCameraShakeRadius - Distance) / InnerCameraShakeRadius, 1.f);
			
			PlayerController->ClientStartCameraShake(ExplodedCameraShake, Scale);
		}
	}
}
