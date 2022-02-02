// Fill out your copyright notice in the Description page of Project Settings.

#include "DMPreviewProps.h"
#include "PreviewInterface.h"

AActor* ADMPreviewProps::AddPreviewActorFromClass(TSubclassOf<AActor> ActorClass)
{
	if (ActorClass->ImplementsInterface(IPreviewInterface::UClassType::StaticClass()))
	{
		AActor* NewPreviewActor = GetWorld()->SpawnActor(ActorClass);
		NewPreviewActor->AttachToComponent(Scene, FAttachmentTransformRules::KeepRelativeTransform);
		NewPreviewActor->PrestreamTextures(1, true);

		IPreviewInterface* PreviewInterface = Cast<IPreviewInterface>(NewPreviewActor);
		NewPreviewActor->SetActorRelativeTransform(PreviewInterface->GetPreviewRelativeTransform());
		return NewPreviewActor;
	}

	return nullptr;
}