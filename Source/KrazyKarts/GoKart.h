// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

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

	FVector Velocity;

	float Throttle;
	float SteeringThrow;

	// mass of the car (kg)
	UPROPERTY(EditAnywhere)
	float Mass = 1000; 

	// the force applied to the car when the throttle is fully down (N)
	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 10000;

	// max degrees rotated per second
	UPROPERTY(EditAnywhere)
	float MaxRotationDegreesPerSecond = 90;

	/* Higher means more drag (kg per meter). Air resistance needs to equal Force(MaxDrivingForce) at some point.
	If we take AirResistance = Speed^2 * DragCoefficient, we can assume AirResistance = MaxDrivingForce and we get 
	the DragCoefficient we need given a Speed we choose.
	*/
	UPROPERTY(EditAnywhere)
	float DragCoefficient = 16;

	void MoveForward(float Value);

	void MoveRight(float Value);

	FVector GetAirResistance();

	void UpdateLocationFromVelocity(float DeltaTime);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void UpdateRotation(float DeltaTime);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;



};
