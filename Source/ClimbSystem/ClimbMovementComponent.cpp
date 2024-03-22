// Fill out your copyright notice in the Description page of Project Settings.


#include "ClimbMovementComponent.h"

#include "ClimbSystemCharacter.h"
#include "Components/CapsuleComponent.h"

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

void UClimbMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	if (IsClimbing())
	{
		bOrientRotationToMovement = false;

		UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
		Capsule->SetCapsuleHalfHeight(Capsule->GetUnscaledCapsuleHalfHeight() - ClimbingCollisionShrinkAmount);
	}

	const bool bWasClimbing = PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Climbing;
	if (bWasClimbing)
	{
		bOrientRotationToMovement = true;

		const FRotator StandRotation = FRotator(0, UpdatedComponent->GetComponentRotation().Yaw, 0);
		UpdatedComponent->SetRelativeRotation(StandRotation);

		UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
		Capsule->SetCapsuleHalfHeight(Capsule->GetUnscaledCapsuleHalfHeight() + ClimbingCollisionShrinkAmount);

		StopMovementImmediately();
	}
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
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
	bWantsToClimb = false;
	UE_LOG(LogTemp, Log, TEXT("CancelClimbing"));
}

bool UClimbMovementComponent::IsClimbing() const
{
	return MovementMode == EMovementMode::MOVE_Custom && CustomMovementMode == ECustomMovementMode::CMOVE_Climbing;
}

FVector UClimbMovementComponent::GetClimbSurfaceNormal() const
{
	return CurrentClimbingNormal;
}

void UClimbMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	if (CustomMovementMode == ECustomMovementMode::CMOVE_Climbing)
	{
		PhysClimbing(deltaTime, Iterations);
	}
	Super::PhysCustom(deltaTime, Iterations);
}

void UClimbMovementComponent::PhysClimbing(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	ComputeSurfaceInfo();

	if (ShouldStopClimbing())
	{
		StopClimbing(deltaTime, Iterations);
		return;
	}

	ComputeClimbingVelocity(deltaTime);

	const FVector OldLocation = UpdatedComponent->GetComponentLocation();

	MoveAlongClimbingSurface(deltaTime);

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
	}
	SnapToClimbingSurface(deltaTime);
}

void UClimbMovementComponent::ComputeSurfaceInfo()
{
	CurrentClimbingNormal = FVector::ZeroVector;
	CurrentClimbingPosition = FVector::ZeroVector;

	if (CurrentWallHits.IsEmpty())
	{
		return;
	}
	
	const FVector Start = UpdatedComponent->GetComponentLocation();
	const FCollisionShape CollisionSphere = FCollisionShape::MakeSphere(6);

	for (const FHitResult& WallHit : CurrentWallHits)
	{
		const FVector End = Start + (WallHit.ImpactPoint - Start).GetSafeNormal() * 120;

		FHitResult AssistHit;
		GetWorld()->SweepSingleByChannel(AssistHit, Start, End, FQuat::Identity,
			ECC_WorldStatic, CollisionSphere, ClimbQueryParams);

		CurrentClimbingPosition += AssistHit.ImpactPoint;
		CurrentClimbingNormal += AssistHit.Normal;
	}

	CurrentClimbingPosition /= CurrentWallHits.Num();
	CurrentClimbingNormal = CurrentClimbingNormal.GetSafeNormal();
}

void UClimbMovementComponent::ComputeClimbingVelocity(float deltaTime)
{
	RestorePreAdditiveRootMotionVelocity();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		constexpr float Friction = 0.0f;
		constexpr bool bFluid = false;
		CalcVelocity(deltaTime, Friction, bFluid, BrakingDecelerationClimbing);
	}

	ApplyRootMotionToVelocity(deltaTime);
}

bool UClimbMovementComponent::ShouldStopClimbing()
{
	const bool bIsOnCeiling = FVector::Parallel(CurrentClimbingNormal, FVector::UpVector);

	return !bWantsToClimb || CurrentClimbingNormal.IsZero() || bIsOnCeiling;
}

void UClimbMovementComponent::StopClimbing(float deltaTime, int32 Iterations)
{
	bWantsToClimb = false;
	SetMovementMode(EMovementMode::MOVE_Falling);
	StartNewPhysics(deltaTime, Iterations);
}

void UClimbMovementComponent::MoveAlongClimbingSurface(float deltaTime)
{
	const FVector Adjusted = Velocity * deltaTime;

	FHitResult Hit(1.f);

	SafeMoveUpdatedComponent(Adjusted, GetClimbingRotation(deltaTime), true, Hit);

	if (Hit.Time < 1.f)
	{
		HandleImpact(Hit, deltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}
}

void UClimbMovementComponent::SnapToClimbingSurface(float deltaTime) const
{
	const FVector Forward = UpdatedComponent->GetForwardVector();
	const FVector Location = UpdatedComponent->GetComponentLocation();
	const FQuat Rotation = UpdatedComponent->GetComponentQuat();

	const FVector ForwardDifference = (CurrentClimbingPosition - Location).ProjectOnTo(Forward);
	const FVector Offset = -CurrentClimbingNormal * (ForwardDifference.Length() - DistanceFromSurface);

	constexpr bool bSweep = true;
	UpdatedComponent->MoveComponent(Offset * ClimbingSnapSpeed * deltaTime, Rotation, bSweep);
}

float UClimbMovementComponent::GetMaxSpeed() const
{
	return IsClimbing() ? MaxClimbingSpeed : Super::GetMaxSpeed();
}

float UClimbMovementComponent::GetMaxAcceleration() const
{
	return IsClimbing() ? MaxClimbingAcceleration : Super::GetMaxAcceleration();
}

FQuat UClimbMovementComponent::GetClimbingRotation(float deltaTime) const
{
	const FQuat Current = UpdatedComponent->GetComponentQuat();
	const FQuat Target = FRotationMatrix::MakeFromX(-CurrentClimbingNormal).ToQuat();

	return FMath::QInterpTo(Current, Target, deltaTime, ClimbingRotationSpeed);
}
