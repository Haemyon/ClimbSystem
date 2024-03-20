// Fill out your copyright notice in the Description page of Project Settings.


#include "ClimbMovementComponent.h"

void UClimbMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	ClimbQueryParams.AddIgnoredActor(GetOwner());
}

void UClimbMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SweepAndStoreWallHits();
}

void UClimbMovementComponent::SweepAndStoreWallHits()
{
	const FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(CollisionCapsuleRadius, CollisionCapsuleHalfHeight);

	const FVector StartOffset = UpdatedComponent->GetForwardVector() * 20;

	const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
	const FVector End = Start + UpdatedComponent->GetForwardVector();

	TArray<FHitResult> Hits;
	const bool HitWall = GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, ECC_WorldStatic, CollisionShape, ClimbQueryParams);

	if (HitWall)
	{
		CurrentWallHits = Hits;
		//히트된 벽 디버깅
		for (int32 Index = 0; Index < Hits.Num(); Index++)
		{
			DrawDebugPoint(GetWorld(), Hits[Index].ImpactPoint, 20.0f, FColor::Cyan, false);

			for (FHitResult& Hit : CurrentWallHits)
			{
				const FVector HorizontalNormal = Hit.Normal.GetSafeNormal2D();

				const float VerticalDot = FVector::DotProduct(Hit.Normal, HorizontalNormal);

				const float HorizontalDegrees = FMath::RadiansToDegrees(FMath::Acos(VerticalDot));

				UE_LOG(LogTemp, Log, TEXT("Wall Degrees: %f"), HorizontalDegrees);
			}
		}
	}
	else
	{
		CurrentWallHits.Reset();
	}
	//히트 범위 디버깅
	FVector Center = GetActorLocation() + StartOffset * 0.5f;
	DrawDebugCapsule
	(
		GetWorld(),
		Center,
		CollisionCapsuleHalfHeight,
		CollisionCapsuleRadius,
		FQuat::Identity,
		FColor::White,
		false,
		-1.0f
	);
}

bool UClimbMovementComponent::CanStartClimbing()
{
	for (FHitResult& Hit : CurrentWallHits)
	{
		const FVector HorizontalNormal = Hit.Normal.GetSafeNormal2D();

		const float HorizontalDot = FVector::DotProduct(UpdatedComponent->GetForwardVector(), -HorizontalNormal);
		const float VerticalDot = FVector::DotProduct(Hit.Normal, HorizontalNormal);

		const float HorizontalDegrees = FMath::RadiansToDegrees(FMath::Acos(HorizontalDot));

		const bool bIsCeiling = FMath::IsNearlyZero(VerticalDot);

		if (HorizontalDegrees <= MinHorizontalDegressToStartClimbing && !bIsCeiling)
		{
			return true;
		}
	}
	return false;
}
