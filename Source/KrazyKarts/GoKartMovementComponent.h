// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.generated.h"

USTRUCT()
struct FGoKartMove
{
	GENERATED_BODY()

	UPROPERTY()
	float Throttle = 0;

	UPROPERTY()
	float SteeringThrow = 0;

	UPROPERTY()
	float DeltaTime = 0;

	UPROPERTY()
	float Time = 0;


};

USTRUCT()
struct FGoKartState
{
	GENERATED_BODY()

	UPROPERTY()
	FGoKartMove LastMove;

	UPROPERTY()
	FVector Velocity = FVector::Zero();

	UPROPERTY()
	FTransform Transform;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UGoKartMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGoKartMovementComponent();


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	FVector GetAirResistance();

	FVector GetRollingResistance();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FGoKartMove CreateMove(float DeltaTime, float ServerTime) const;

	void SimulateMove(const FGoKartMove& Move);
	
	float GetRollingResistanceCoefficient() const;

	void SetVelocity(const FVector& NewVelocity);

	float GetMass() const;

	float GetMaxDrivingForce() const;

	float GetMinTurningRadius() const;

	float GetDragCoefficient() const;

	FVector GetVelocity() const;

	TArray<FGoKartMove> GetUnacknowledgedMoves() const;

	float GetThrottle() const;

	void SetThrottle(const float& Value);

	float GetSteeringThrow() const;

	void SetSteeringThrow(const float & Value);

	FGoKartMove GetLastMove()
	{
		return LastMove;
	}

private:
	void UpdateLocationFromVelocity(float DeltaTime);

	void UpdateRotation(float DeltaTime, float Steering);

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

	FVector Velocity;
	
	float Throttle;
	float SteeringThrow;
	
	FGoKartMove LastMove;

		
};
