// Fill out your copyright notice in the Description page of Project Settings.


#include "ClimbMovementComponent.h"

#include "ClimbSystemCharacter.h"
#include "ECustomMovementMode.h"

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
			//bool CanStartClimbing 함수 디버깅
			/*
			for (FHitResult& Hit : CurrentWallHits)
			{
				const FVector HorizontalNormal = Hit.Normal.GetSafeNormal2D();

				const float HorizontalDot = FVector::DotProduct(UpdatedComponent->GetForwardVector(), -HorizontalNormal);
				const float VerticalDot = FVector::DotProduct(Hit.Normal, HorizontalNormal);

				const float HorizontalDegrees = FMath::RadiansToDegrees(FMath::Acos(HorizontalDot));
				const float VerticalDegrees = FMath::RadiansToDegrees(FMath::Acos(VerticalDot));

				const bool bIsCeiling = FMath::IsNearlyZero(VerticalDot);

				UE_LOG(LogTemp, Log, TEXT("Wall Degrees: %f"), VerticalDot);

				if (HorizontalDegrees <= MinHorizontalDegressToStartClimbing && !bIsCeiling)
				{
					UE_LOG(LogTemp, Log, TEXT("!bIsCeiling"));

				}
			}
			*/
		}
	}
	else
	{
		CurrentWallHits.Reset();
	}
	//CanStartClimbing();
	//히트 범위 디버깅
	FVector Center = GetActorLocation() + StartOffset * 0.5f;
	DrawDebugCapsule(GetWorld(), Center, CollisionCapsuleHalfHeight, CollisionCapsuleRadius, FQuat::Identity, FColor::White, false, -1.0f);
}

bool UClimbMovementComponent::CanStartClimbing()
{
	//if (CurrentWallHits.Num() > 0)
	//{
		for (FHitResult& Hit : CurrentWallHits)
		{
			const FVector HorizontalNormal = Hit.Normal.GetSafeNormal2D();

			const float HorizontalDot = FVector::DotProduct(UpdatedComponent->GetForwardVector(), -HorizontalNormal);
			const float VerticalDot = FVector::DotProduct(Hit.Normal, HorizontalNormal);

			const float HorizontalDegrees = FMath::RadiansToDegrees(FMath::Acos(HorizontalDot));

			const bool bIsCeiling = FMath::IsNearlyZero(VerticalDot);

			if (HorizontalDegrees <= MinHorizontalDegressToStartClimbing &&
				!bIsCeiling && IsFacingSurface(VerticalDot))
			{
				//UE_LOG(LogTemp, Log, TEXT("Can Climb!!"));
				return true;
			}
		}
		//UE_LOG(LogTemp, Error, TEXT("Can't Climb!!"));
	//}
	return false;
}

bool UClimbMovementComponent::EyeHeightTrace(const float TraceDistance) const
{
	FHitResult UpperEdgeHit;

	const FVector Start = UpdatedComponent->GetComponentLocation() +
		(UpdatedComponent->GetUpVector() * GetCharacterOwner()->BaseEyeHeight);
	const FVector End = Start + (UpdatedComponent->GetForwardVector() * TraceDistance);

	//눈 높이 라인 트레이스 디버그
	DrawDebugLine(GetWorld(), Start, End, FColor::Orange, false, -1.0f, 0U, 1.0f);

	return GetWorld()->LineTraceSingleByChannel(UpperEdgeHit, Start, End,
		ECC_WorldStatic, ClimbQueryParams);
}

bool UClimbMovementComponent::IsFacingSurface(const float Steepness) const
{
	constexpr float BaseLength = 80;
	const float SteepnessMultiplier = 1 + (1 - Steepness) * 5;

	return EyeHeightTrace(BaseLength * SteepnessMultiplier);
}

void UClimbMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	if (CanStartClimbing())
	{
		if (bWantsToClimb)
		{
			SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_Climbing);
			UE_LOG(LogTemp, Log, TEXT("SET CMOVE_Climbing Mode"));
		}
	}

	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
}

void UClimbMovementComponent::TryClimbing()
{
	if (CanStartClimbing() == true)
	{
		bWantsToClimb = true;
		UE_LOG(LogTemp, Log, TEXT("TryClimbing"));
	}
}

void UClimbMovementComponent::CancelClimbing()
{
	//if (bWantsToClimb)
	//{
	//}
	bWantsToClimb = false;
	UE_LOG(LogTemp, Log, TEXT("CancelClimbing"));
}

bool UClimbMovementComponent::IsClimbing() const
{
	return MovementMode == EMovementMode::MOVE_Custom && CustomMovementMode == ECustomMovementMode::CMOVE_Climbing;
}

FVector UClimbMovementComponent::GetClimbSurfaceNormal() const
{
	return FVector();
}
