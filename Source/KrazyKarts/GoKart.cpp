// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"
#include "Components/InputComponent.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Force = MaxDrivingForce * Throttle * GetActorForwardVector(); 
	Force += GetAirResistance();
	FVector Acceleration = Force / Mass;

	//UpdateRotation(DeltaTime);
	float RotationAngle = MaxRotationDegreesPerSecond * DeltaTime * SteeringThrow;
	FQuat RotationDelta(GetActorUpVector(), FMath::DegreesToRadians(RotationAngle));
	AddActorWorldRotation(RotationDelta);
	// this is my take on rotating the velocity vector, and I think it works better this way
	Velocity = RotationDelta.RotateVector(Velocity) + Acceleration * DeltaTime;

	UpdateLocationFromVelocity(DeltaTime);
}

void AGoKart::UpdateRotation(float DeltaTime)
{
	// this is the course take on rotating the velocity vector
	float RotationAngle = MaxRotationDegreesPerSecond * DeltaTime * SteeringThrow;
	FQuat RotationDelta(GetActorUpVector(), FMath::DegreesToRadians(RotationAngle));
	AddActorWorldRotation(RotationDelta);
	RotationDelta.RotateVector(Velocity);
}

void AGoKart::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector DeltaTranslation = Velocity * 100 * DeltaTime;

	FHitResult hit;
	this->AddActorWorldOffset(DeltaTranslation, true, &hit);
	if (hit.IsValidBlockingHit())
	{
		Velocity = FVector::Zero();
	}
}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGoKart::MoveRight);
}

void AGoKart::MoveForward(float Value)
{
	Throttle = Value;
}

void AGoKart::MoveRight(float Value)
{
	SteeringThrow = Value;
}

FVector AGoKart::GetAirResistance()
{
	return -1 * Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}
