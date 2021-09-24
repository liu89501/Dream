// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ScalableWidgetComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UScalableWidgetComponent::UScalableWidgetComponent()
{
    SetIsReplicated(false);
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UScalableWidgetComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (GetNetMode() != NM_DedicatedServer)
    {
        SetComponentTickEnabled(true);
    }
}

void UScalableWidgetComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (LocalPlayerCharacter == nullptr)
    {
        if (APlayerController* PlayerController = GEngine->GetFirstLocalPlayerController(GetWorld()))
        {
            LocalPlayerCharacter = PlayerController->GetPawn();
        }
    }

    bool bVisibleFlag = GetVisibleFlag();
        
    if (LocalPlayerCharacter)
    {
        float Distance = FVector::Distance(GetOwner()->GetActorLocation(), LocalPlayerCharacter->GetActorLocation());

        if (HiddenDistance > 0)
        {
            if (HiddenDistance < Distance)
            {
                if (bVisibleFlag)
                {
                    SetVisibility(false);
                }
            }
            else
            {
                if (!bVisibleFlag)
                {
                    SetVisibility(true);
                }
            }
        }

        if (bVisibleFlag && bUseScale)
        {
            float ScaleValue = UKismetMathLibrary::MapRangeClamped(Distance, DistanceA, DistanceB, ScaleA, ScaleB);
            if (UUserWidget* UserWidget = GetUserWidgetObject())
            {
                UserWidget->SetRenderScale(FVector2D(ScaleValue, ScaleValue));
            }
        }
    }
}
