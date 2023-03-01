// Fill out your copyright notice in the Description page of Project Settings.

#include "DMPreviewProps.h"
#include "GearInterface.h"

AActor* ADMPreviewProps::AddPreviewActorFromClass(TSubclassOf<AActor> ActorClass)
{
	if (ActorClass->ImplementsInterface(IGearInterface::UClassType::StaticClass()))
	{
		AActor* NewPreviewActor = GetWorld()->SpawnActor(ActorClass);
		NewPreviewActor->AttachToComponent(Scene, FAttachmentTransformRules::KeepRelativeTransform);
		NewPreviewActor->PrestreamTextures(1, true);

		if (IGearInterface* GearInterface = Cast<IGearInterface>(NewPreviewActor))
		{
			NewPreviewActor->SetActorRelativeTransform(GearInterface->GetPreviewRelativeTransform());
		}
		return NewPreviewActor;
	}

	return nullptr;
}
