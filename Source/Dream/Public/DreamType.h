// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DreamType.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDream, Log, All);

#define DREAM_LOG(Verbosity, Format, ...) UE_LOG(LogDream, Verbosity, TEXT("%s: %s"), *UEnum::GetValueAsString(GetLocalRole()), *FString::Printf(Format, ##__VA_ARGS__));
#define DREAM_NLOG(Verbosity, Format, ...) UE_LOG(LogDream, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));

#define Collision_ObjectType_Projectile ECollisionChannel::ECC_GameTraceChannel1
#define Collision_ObjectType_Weapon ECollisionChannel::ECC_GameTraceChannel2

#define IfStandalone(Key) if (GetNetMode() == NM_Standalone) { Key; }

#define DNUMBER_ZERO (0)
#define DNUMBER_ONE (1)

#define PROBABILITY_UINT 100

namespace DreamActorTagName
{
	extern const FName Teammate;
	extern const FName Enemy;
	extern const FName Death;
}

UENUM(BlueprintType)
namespace EWidgetOrder
{
	enum Type
	{
		None,
		Player,
		PlayerPopup,
		PlayerOuter,
		PlayerCtrl,
		PlayerCtrlPopup,
		PlayerCtrlOuter,
		InteractiveUI,
		RewardPopup,
		LoadingPopup
	};
}

UENUM(BlueprintType)
enum class EGameType : uint8
{
	PVE,
	PVP,
	SalePlace
};

UENUM(BlueprintType)
enum class EGearType : uint8
{
	None,
	Head,
	Hand,
	Leg,
	Shoe
};

UENUM(BlueprintType)
namespace EMessageType
{
	enum Type
	{
		Kill_Message,
		Talk_Message,
		TeamApply_Message,
		All
	};
}

UENUM()
enum class EWeaponStatus : uint8
{
	Idle,
	Firing,
	Reloading,
	Equipping
};

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	Level1,
	Level2,
	Level3
};

UENUM(BlueprintType)
enum class EAbilityType : uint8
{
	Initiative,
	Passives
};

UENUM(BlueprintType)
enum class EMiniMapDrawType : uint8
{
	Warning,
	Sprite
};

UENUM(BlueprintType)
enum class EOpportunity : uint8
{
	Immediately UMETA(ToolTip="即时", DisplayName="立即"),
	Reloading UMETA(ToolTip="填装弹药时", DisplayName="装填弹药时"),
	Injured UMETA(ToolTip="受到伤害时", DisplayName="受到伤害时"),
	Firing UMETA(ToolTip="开火时", DisplayName="开火时"),
	KilledEnemy UMETA(ToolTip="击杀敌方目标时", DisplayName="击杀敌方目标时"),
};

UENUM(BlueprintType)
namespace ETalkType
{
	enum Type
	{
		Current,
		World,
		Team,
		Private,
		NONE
	};
}

USTRUCT(BlueprintType)
struct FWeaponTrailVFX
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailEffect;
	UPROPERTY(EditAnywhere)
	float MinimumSpawnDistance;
	UPROPERTY(EditAnywhere)
	float TrailFlyingSpeed;
	UPROPERTY(EditAnywhere)
	FVector SpawnPositionOffset;
	UPROPERTY(EditAnywhere)
	FName TrailEndLocationParamName;
	UPROPERTY(EditAnywhere)
	FName TrailLifeTimeParamName;


	FWeaponTrailVFX()
		: TrailEffect(nullptr),
		  MinimumSpawnDistance(200.f),
		  TrailFlyingSpeed(25000.f),
		  SpawnPositionOffset(FVector::ZeroVector),
		  TrailEndLocationParamName(TEXT("EndPosition")),
		  TrailLifeTimeParamName(TEXT("NCLifeTime"))
	{
	}
};

struct FMiniMapData
{
public:
	FMiniMapData() : PosNormalize(FVector2D::ZeroVector),
					 Yaw(0),
					 DistancePercentage(0),
	                 DrawType(EMiniMapDrawType::Warning),
	                 SpriteBrush(nullptr),
	                 OverflowSpriteBrush(nullptr)
	{
	}

	FMiniMapData(
		const FVector2D& InPosNormalize,
		float InYaw,
		float InDistancePercentage,
		EMiniMapDrawType InDrawType,
		FSlateBrush* InSprite,
		FSlateBrush* InOverflowSprite)
		:
		PosNormalize(InPosNormalize),
		Yaw(InYaw),
		DistancePercentage(InDistancePercentage),
		DrawType(InDrawType),
		SpriteBrush(InSprite),
		OverflowSpriteBrush(InOverflowSprite)
	{
	}

	FVector2D PosNormalize;
	float Yaw;
	float DistancePercentage;
	EMiniMapDrawType DrawType;
	FSlateBrush* SpriteBrush;
	FSlateBrush* OverflowSpriteBrush;
};

USTRUCT()
struct FDamageResult
{
	GENERATED_USTRUCT_BODY()

public:

	FDamageResult()
		: Damage(0), bCritical(false)
	{
	}

	FDamageResult(float InDamage, bool InbCritical)
		: Damage(InDamage), bCritical(InbCritical)
	{
	}

	float Damage;
	bool bCritical;
};

USTRUCT(BlueprintType)
struct FKillMessage
{
	GENERATED_USTRUCT_BODY()

public:

	FKillMessage() = default;

	FKillMessage(FString InKillerName, FString InSufferName)
		: KillerName(InKillerName), SufferName(InSufferName)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString KillerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SufferName;
};

USTRUCT(BlueprintType)
struct FTalkMessage
{
	GENERATED_USTRUCT_BODY()

public:

	FTalkMessage() = default;

	FTalkMessage(FString InPlayerName, FString InContent, TEnumAsByte<ETalkType::Type> InTalkType)
		: PlayerName(InPlayerName), Content(InContent), TalkType(InTalkType)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Content;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ETalkType::Type> TalkType;
};

USTRUCT(BlueprintType)
struct FPlayerAccountInfo
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AccountID;

	bool operator==(const FPlayerAccountInfo& Other)
	{
		return Other.AccountID == AccountID;
	}
};

USTRUCT()
struct FRadialDamageProjectileInfo
{
	GENERATED_USTRUCT_BODY()

public:

	FRadialDamageProjectileInfo() :
		Origin(FVector::ZeroVector),
		DamageRadius(0.f)
	{
	}


	FRadialDamageProjectileInfo(
		const FVector& InOrigin,
		float InDamageRadius)

		: Origin(InOrigin),
		  DamageRadius(InDamageRadius)
	{
	}

public:

	UPROPERTY()
	FVector_NetQuantize Origin;
	UPROPERTY()
	float DamageRadius;
};


USTRUCT(BlueprintType)
struct FSurfaceImpactEffect
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;
	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;
	UPROPERTY(EditAnywhere)
	class UMaterialInterface* ImpactDecal;
};


USTRUCT(BlueprintType)
struct FMatchmakingHandle
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY()
	class UMatchmakingCallProxy* MatchmakingCallProxy;

	bool IsValid() const
	{
		return MatchmakingCallProxy != nullptr;
	}
};


USTRUCT(BlueprintType)
struct FHttpRequestParameter
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Value;
};

USTRUCT(BlueprintType)
struct FMapInfo : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString FullName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* MapIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;
};

UCLASS()
class UCharacterMesh : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USkeletalMesh* MasterMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<USkeletalMesh*> SlaveMeshs;
};

/*
	按几率随机
*/
struct FRandomProbability
{
public:

	static int32 RandomProbability(const TArray<float>& Probability, int32 Unit = 100)
	{
		float RandomValue = FMath::FRandRange(0, Unit);

		int32 Index = INDEX_NONE;

		for (int I = 0; I < Probability.Num(); I++)
		{
			float Min = Sum(Probability, I);
			float Max = Sum(Probability, I + 1);

			if (RandomValue >= Min && RandomValue <= Max)
			{
				Index = I;
				break;
			}
		}

		return Index;
	}

	template <typename SourceType>
	static SourceType RandomProbabilityEx(const TMap<SourceType, float>& ProbabilitySource, int32 Unit = 100)
	{
		TArray<SourceType> Values;
		TArray<float> Probability;

		GetArrayPair(ProbabilitySource, Values, Probability);

		int32 Index = RandomProbability(Probability, Unit);

		if (Index == INDEX_NONE)
		{
			return SourceType();
		}

		return Values[Index];
	}

private:

	template <typename SourceType>
	static void GetArrayPair(const TMap<SourceType, float>& ProbabilitySource, TArray<SourceType>& Sources,
	                         TArray<float>& Probability)
	{
		for (TPair<SourceType, float> Pair : ProbabilitySource)
		{
			Sources.Add(Pair.Key);
			Probability.Add(Pair.Value);
		}
	}

	static float Sum(const TArray<float>& Array, int32 EndIndex, int32 StartIndex = 0)
	{
		float SumResult = 0.f;
		for (int I = StartIndex; I < EndIndex; I++)
		{
			SumResult += Array[I];
		}
		return SumResult;
	}
};

struct FHitResultKeyFuncs : DefaultKeyFuncs<FHitResult>
{
	static FORCEINLINE uint32 GetKeyHash(FHitResult const& Key)
	{
		return Key.Actor.IsValid() ? reinterpret_cast<uint64>(Key.Actor.Get()) : 0;
	}

	static FORCEINLINE bool Matches(FHitResult const& A, FHitResult const& B) { return (A.GetActor() == B.GetActor()); }
};

template <typename ElementType, typename SetKeyFunc = DefaultKeyFuncs<ElementType>>
struct FArrayDistinctIterator;

template <typename ElementType, typename SetKeyFunc>
struct FArrayDistinctIterator
{
	typedef typename TArray<ElementType>::SizeType SizeType;

	explicit FArrayDistinctIterator(const TArray<ElementType>& InArray)
		: Ptr(InArray.GetData()),
		  ArrayNum(InArray.Num()),
		  CurrentIndex(0)
	{
		if (Ptr)
		{
			Exist.Add(*Ptr);
		}
	};

	FORCEINLINE void operator++()
	{
		bool bIsAlreadyInSet = true;
		do
		{
			++Ptr;
			++CurrentIndex;

			Exist.Add(*Ptr, &bIsAlreadyInSet);
		}
		while (bIsAlreadyInSet && *this);
	}

	FORCEINLINE const ElementType& operator*() const
	{
		return *Ptr;
	}

	FORCEINLINE operator bool() const
	{
		return CurrentIndex < ArrayNum;
	}

	FORCEINLINE const ElementType& operator->() const
	{
		return *Ptr;
	}

	const ElementType* Ptr;

	SizeType ArrayNum;
	SizeType CurrentIndex;

	TSet<ElementType, SetKeyFunc> Exist;
};
