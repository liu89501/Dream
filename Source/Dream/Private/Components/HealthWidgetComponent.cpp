// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthWidgetComponent.h"

#include "HealthUIInterface.h"

void UHealthWidgetComponent::UpdateStatus(float HealthPercentage, float ShieldPercentage)
{
    IHealthUIInterface::Execute_UpdateUI(GetUserWidgetObject(), HealthPercentage, ShieldPercentage);
}
