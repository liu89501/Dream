// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ScalableWidgetComponent.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UScalableWidgetComponent::UScalableWidgetComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UScalableWidgetComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bUseScale)
    {
        return;
    }

    if (UUserWidget* UserWidget = GetUserWidgetObject())
    {
        if (ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0))
        {
            float Distance = (GetOwner()->GetActorLocation() - PlayerCharacter->GetActorLocation()).Size();
            float ScaleValue = UKismetMathLibrary::MapRangeClamped(Distance, DistanceA, DistanceB, ScaleA, ScaleB);
            UserWidget->SetRenderScale(FVector2D(ScaleValue, ScaleValue));
        }
    }
}
