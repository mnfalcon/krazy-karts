// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GoKartMovementComponent.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementReplicator.generated.h"

USTRUCT()
struct FHermiteCubicSpline
{
	GENERATED_BODY()

	FVector StartLocation, StartDerivative, TargetLocation, TargetDerivative;

	FVector InterpolateLocation(float LerpRatio) const
	{
		return FMath::CubicInterp(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}
	FVector InterpolateDerivative(float LerpRatio) const
	{
		return FMath::CubicInterpDerivative(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UGoKartMovementReplicator : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGoKartMovementReplicator();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void AddUnacknowledgedMove(::FGoKartMove& Move);

	FGoKartState& GetServerState()
	{
		return ServerState;
	}

	TArray<FGoKartMove>& GetUnacknowledgedMoves()
	{
		return UnacknowledgedMoves;
	}

	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FGoKartState ServerState;

	void UpdateServerState(const FGoKartMove& Move);

	UFUNCTION(BlueprintCallable)
	void SetMeshOffsetRoot(USceneComponent* Root)
	{
		MeshOffsetRoot = Root;
	}

	USceneComponent* GetMeshOffsetRoot()
	{
		return MeshOffsetRoot;
	}

	virtual void BeginDestroy() override;

private:

	UPROPERTY()
	USceneComponent* MeshOffsetRoot;
	
	TArray<FGoKartMove> UnacknowledgedMoves;

	float ClientTimeSinceUpdate;

	float ClientTimeBetweenLastUpdates;

	FTransform ClientStartTransform;

	FVector ClientStartVelocity;

	float ClientSimulatedTime;
	
	UGoKartMovementComponent* MovementComponent;

	UFUNCTION(Server, Reliable, WithValidation)
	virtual void Server_SendMove(FGoKartMove Move);

	virtual bool Server_SendMove_Validate(FGoKartMove Move);

	void ClearAcknowledgedMoves(FGoKartMove LastMove);
	
	UFUNCTION()
	void OnRep_ServerState();

	void SimulatedProxy_OnRep_ServerState();
	void AutonomousProxy_OnRep_ServerState();
	FHermiteCubicSpline CreateSpline();
	float GetVelocityToDerivative();
	FVector GetNewVelocity(FHermiteCubicSpline Spline, float LerpRatio);
	void InterpolateLocation(const float& LerpRatio, const FHermiteCubicSpline& Spline) const;
	void InterpolateDerivative(const float& LerpRatio, const FHermiteCubicSpline& Spline);
	void InterpolateRotation(const float& LerpRatio) const;

	void ClientTick(float DeltaTime);

		
};
