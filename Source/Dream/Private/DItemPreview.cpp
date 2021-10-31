// Fill out your copyright notice in the Description page of Project Settings.


#include "DItemPreview.h"

#include "DCharacterPlayer.h"
#include "Components/SkyLightComponent.h"
#include "Components/DirectionalLightComponent.h"

// Sets default values
ADItemPreview::ADItemPreview()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	PreviewActorRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PreviewRoot"));
	PreviewActorRoot->SetupAttachment(RootComponent);
}

void ADItemPreview::ProcessMeshComponents(UMeshComponent* PreviewBody, UMeshComponent* Mesh)
{
	TArray<USceneComponent*> Components = Mesh->GetAttachChildren();

	if (Components.Num() > 0)
	{
		for (USceneComponent* Component : Components)
		{
			UMeshComponent* MeshComponent = nullptr;
			UMeshComponent* NewMeshComponent = nullptr;
			
			if (USkeletalMeshComponent* SKMeshComponent = Cast<USkeletalMeshComponent>(Component))
			{
				USkeletalMeshComponent* NewSKMeshComponent = NewObject<USkeletalMeshComponent>(
                    this, USkeletalMeshComponent::StaticClass(), NAME_None, RF_NoFlags, SKMeshComponent);
				
				NewSKMeshComponent->SetAnimClass(SKMeshComponent->GetAnimClass());
				NewMeshComponent = NewSKMeshComponent;
				MeshComponent = SKMeshComponent;
			}
			else if (UStaticMeshComponent* SMMeshComponent = Cast<UStaticMeshComponent>(Component))
			{
				UStaticMeshComponent* NewSMMeshComponent = NewObject<UStaticMeshComponent>(
					this, UStaticMeshComponent::StaticClass(), NAME_None, RF_NoFlags, SMMeshComponent);

				NewMeshComponent = NewSMMeshComponent;
				MeshComponent = SMMeshComponent;
			}

			if (NewMeshComponent)
			{
				NewMeshComponent->RegisterComponent();
				NewMeshComponent->AttachToComponent(PreviewBody, FAttachmentTransformRules::KeepRelativeTransform, Component->GetAttachSocketName());
				ProcessMeshComponents(NewMeshComponent, MeshComponent);
			}
		}
	}
}

USkeletalMeshComponent* ADItemPreview::AddPreviewSkeletalMeshFromComponent(USkeletalMeshComponent* CharacterMesh)
{
	USkeletalMeshComponent* Body = NewObject<USkeletalMeshComponent>(
                    this, USkeletalMeshComponent::StaticClass(), NAME_None, RF_NoFlags, CharacterMesh);
	Body->RegisterComponent();
	Body->SetRelativeTransform(FTransform::Identity);
	Body->AttachToComponent(PreviewActorRoot, FAttachmentTransformRules::KeepRelativeTransform);
	Body->SetAnimClass(CharacterMesh->GetAnimClass());
	ProcessMeshComponents(Body, CharacterMesh);
	return Body;
}

AActor* ADItemPreview::AddPreviewActor(AActor* Template)
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Template = Template;
	AActor* SpawnPreviewActor = GetWorld()->SpawnActor<AActor>(Template->GetClass(), SpawnParameters);
	SpawnPreviewActor->AttachToComponent(PreviewActorRoot, FAttachmentTransformRules::KeepRelativeTransform);
	return SpawnPreviewActor;
}

// Called when the game starts or when spawned
void ADItemPreview::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADItemPreview::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

