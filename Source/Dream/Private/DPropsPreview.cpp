// Fill out your copyright notice in the Description page of Project Settings.


#include "DPropsPreview.h"
#include "PropsInterface.h"

AActor* ADPropsPreview::AddPreviewActorFromClass(TSubclassOf<AActor> ActorClass)
{
	if (ActorClass->ImplementsInterface(IPropsInterface::UClassType::StaticClass()))
	{
		AActor* NewPreviewActor = GetWorld()->SpawnActor(ActorClass);
		NewPreviewActor->AttachToComponent(Scene, FAttachmentTransformRules::KeepRelativeTransform);
		NewPreviewActor->PrestreamTextures(1, true);

		IPropsInterface* PropsInterface = Cast<IPropsInterface>(NewPreviewActor);
		NewPreviewActor->SetActorRelativeTransform(PropsInterface->GetPreviewRelativeTransform());
		return NewPreviewActor;
	}

	return nullptr;
}
