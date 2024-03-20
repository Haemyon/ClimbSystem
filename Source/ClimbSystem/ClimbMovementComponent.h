// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ClimbMovementComponent.generated.h"

/**
 *
 */
UCLASS()
class CLIMBSYSTEM_API UClimbMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

private:
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SweepAndStoreWallHits();

	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere)
	int CollisionCapsuleRadius = 50;

	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere)
	int CollisionCapsuleHalfHeight = 72;

	TArray<FHitResult> CurrentWallHits;

	FCollisionQueryParams ClimbQueryParams;
};
