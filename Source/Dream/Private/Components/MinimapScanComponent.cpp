// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/MinimapScanComponent.h"


#include "DCharacterBase.h"
#include "IconInterface.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values for this component's properties
UMinimapScanComponent::UMinimapScanComponent()
{
	ScanInterval = 1.f;
	
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.

#if UE_SERVER

	PrimaryComponentTick.bCanEverTick = false;

#else

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = ScanInterval;
	
#endif

	SetIsReplicated(false);

	MaxInfiniteItemNum = 16;
	MaxWithinRadiusItemNum = 24;
}


// Called when the game starts
void UMinimapScanComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ScanObjectTypes.Num() == 0)
	{
		SetComponentTickEnabled(false);
	}

	IgnoredTraceOwner.Add(GetOwner());
}

void UMinimapScanComponent::AddInfiniteActors(const TArray<AActor*>& TargetActors)
{
	for (AActor* Actor : TargetActors)
	{
		if (!AddInfiniteActor(Actor))
		{
			break;
		}
	}
}

bool UMinimapScanComponent::AddInfiniteActor(AActor* TargetActor)
{
	if (InfiniteActors.Num() == MaxInfiniteItemNum)
	{
		return false;
	}
	
	if (Cast<IIconInterface>(TargetActor) != nullptr)
	{
		InfiniteActors.Add(TargetActor);
	}

	return true;
}

void UMinimapScanComponent::RemoveInfiniteActor(AActor* TargetActor)
{
	InfiniteActors.Remove(TargetActor); 
}

void UMinimapScanComponent::ClearInfiniteActors()
{
	InfiniteActors.Reset();
}

FMinimapDataIterator UMinimapScanComponent::GetScannedItemsIterator()
{
	return FMinimapDataIterator(this);
}

// Called every frame
void UMinimapScanComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ActorWithinRadius.Num() == MaxWithinRadiusItemNum)
	{
		return;
	}

	AActor* Owner = GetOwner();

	if (Owner == nullptr)
	{
		return;
	}
	
	TArray<FHitResult> OutHits;
	FVector Location = Owner->GetActorLocation();

	UKismetSystemLibrary::SphereTraceMultiForObjects(this, Location, Location, RadarScanRadius,
        ScanObjectTypes, false, IgnoredTraceOwner, EDrawDebugTrace::None, OutHits, true);

	ActorWithinRadius.Reset();

	for (FArrayDistinctIterator<FHitResult, FHitResultKeyFuncs> It(OutHits); It; ++It)
	{
		if (ActorWithinRadius.Num() == MaxWithinRadiusItemNum)
		{
			break;
		}
		
		AActor* Actor = (*It).GetActor();

		if (Actor->IsA<ADCharacterBase>())
		{
			ActorWithinRadius.Add(Actor);
		}
		else if (IIconInterface* IconInterface = Cast<IIconInterface>(Actor))
		{
			UIconComponent* IconComponent = IconInterface->GetIconComponent();

			if (IconComponent && IconComponent->IsActive())
			{
				ActorWithinRadius.Add(Actor);
			}
		}
	}
}

void CalcDistanceAndDirection(ACharacter* PlayerCharacter, AActor* Target, float MaxRadarRadius, FRotator& Rot, float& Distance)
{
	FVector Location = PlayerCharacter->GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();

	Rot = (TargetLocation - Location).Rotation() - PlayerCharacter->GetControlRotation();
	Rot.Pitch = 0;
	Rot.Normalize();

	Distance = FMath::Clamp(FVector2D::Distance(FVector2D(TargetLocation), FVector2D(Location)) / MaxRadarRadius, 0.f, 1.f);
}

TSharedPtr<FMinimapDataBase> FMinimapDataIterator::Get() const
{
	TSharedPtr<FMinimapDataBase> Result;

	UMinimapScanComponent* ScanComponent = GetComponent();
	
	if (ScanComponent == nullptr)
	{
		return Result;
	}

	ACharacter* Character = Cast<ACharacter>(ScanComponent->GetOwner());
	if (Character == nullptr || Character->IsPendingKillPending())
	{
		return Result;
	}

	AActor* ScannedActor = nullptr;
		
	if (ScanComponent->InfiniteActors.IsValidIndex(InfiniteActorsIndex))
	{
		ScannedActor = ScanComponent->InfiniteActors[InfiniteActorsIndex];
	}
	else if (ScanComponent->ActorWithinRadius.IsValidIndex(ActorWithinRadiusIndex))
	{
		ScannedActor = ScanComponent->ActorWithinRadius[ActorWithinRadiusIndex];
	}

	if (ScannedActor == nullptr || ScannedActor->IsPendingKillPending())
	{
		return Result;
	}

	UIconComponent* IconComponent = nullptr;
	
	if (IIconInterface* IconInterface = Cast<IIconInterface>(ScannedActor))
	{
		IconComponent = IconInterface->GetIconComponent();
	}

	if (IconComponent == nullptr || !IconComponent->IsActive())
	{
		if (!ScannedActor->IsA<ADCharacterBase>())
		{
			return Result;
		}

		if (FGenericTeamId::GetAttitude(Character, ScannedActor) == ETeamAttitude::Hostile)
		{
			FRotator Rot;
			float DistanceNormalize;
			CalcDistanceAndDirection(Character, ScannedActor, ScanComponent->RadarScanRadius, Rot, DistanceNormalize);
			
			Result = MakeShared<FMiniMapWarning>(Rot.Yaw, DistanceNormalize);
			return Result;
		}
	}
	else
	{
		FRotator Rot;
		float Distance;
		CalcDistanceAndDirection(Character, ScannedActor, ScanComponent->RadarScanRadius, Rot, Distance);

		FVector2D Position = FVector2D(Rot.Vector().GetSafeNormal());
		float Direction = IconComponent->bFixedDirection ? 0.f : Rot.Yaw;
	
		Result = MakeShared<FMiniMapIcon>(Position, Direction, Distance, &IconComponent->Icon);
	}

	return Result;
}
