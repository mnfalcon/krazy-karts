// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

USTRUCT()
struct FGoKartMove
{
	GENERATED_BODY()

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float SteeringThrow;

	UPROPERTY()
	float DeltaTime;

	UPROPERTY()
	float Time;


};

USTRUCT()
struct FGoKartState
{
	GENERATED_BODY()

	UPROPERTY()
	FGoKartMove LastMove;

	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FTransform Transform;
};

UCLASS()
class KRAZYKARTS_API AGoKart : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGoKart();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	
	TArray<FGoKartMove> UnacknowledgedMoves;

	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FGoKartState ServerState;

	FVector Velocity;

	//UPROPERTY(ReplicatedUsing=OnRep_ReplicatedTransform)
	//FTransform ReplicatedTransform;

	float Throttle;
	float SteeringThrow;

	UPROPERTY(EditAnywhere)
	float RollingResistanceCoefficient = 0.015f;

	// mass of the car (kg)
	UPROPERTY(EditAnywhere)
	float Mass = 1000; 

	// the force applied to the car when the throttle is fully down (N)
	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 10000;

	// minimum radius of the car turning circle at full lock
	UPROPERTY(EditAnywhere)
	float MinTurningRadius = 10;

	/* Higher means more drag (kg per meter). Air resistance needs to equal Force(MaxDrivingForce) at some point.
	If we take AirResistance = Speed^2 * DragCoefficient, we can assume AirResistance = MaxDrivingForce and we get 
	the DragCoefficient we need given a Speed we choose.
	*/
	UPROPERTY(EditAnywhere)
	float DragCoefficient = 16;

	UFUNCTION(Server, Reliable, WithValidation)
	virtual void Server_SendMove(FGoKartMove Move);

	virtual bool Server_SendMove_Validate(FGoKartMove Move);

	//UFUNCTION(Server, Reliable, WithValidation)
	//virtual void Server_MoveRight(float Value);

	//virtual bool Server_MoveRight_Validate(float Value);

	void MoveForward(float Value);

	void MoveRight(float Value);

	void SimulateMove(const FGoKartMove& Move);

	//UFUNCTION()
	//void OnRep_ReplicatedTransform();

	UFUNCTION()
	void OnRep_ServerState();

	FVector GetAirResistance();

	FVector GetRollingResistance();

	void UpdateLocationFromVelocity(float DeltaTime);

	FGoKartMove CreateMove(float DeltaTime) const;

	void ClearAcknowledgedMoves(FGoKartMove LastMove);


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void UpdateRotation(float DeltaTime, float Steering);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;



};
