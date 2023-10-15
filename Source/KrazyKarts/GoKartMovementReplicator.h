// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GoKartMovementComponent.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementReplicator.generated.h"


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

private:
	
	TArray<FGoKartMove> UnacknowledgedMoves;

	float ClientTimeSinceUpdate;

	float ClientTimeBetweenLastUpdates;

	FTransform ClientStartTransform;

	FVector ClientStartVelocity;
	
	UGoKartMovementComponent* MovementComponent;

	UFUNCTION(Server, Reliable, WithValidation)
	virtual void Server_SendMove(FGoKartMove Move);

	virtual bool Server_SendMove_Validate(FGoKartMove Move);

	void ClearAcknowledgedMoves(FGoKartMove LastMove);
	
	UFUNCTION()
	void OnRep_ServerState();

	void SimulatedProxy_OnRep_ServerState();
	void AutonomousProxy_OnRep_ServerState();

	void ClientTick(float DeltaTime);

		
};
