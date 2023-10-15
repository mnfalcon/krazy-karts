// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKartMovementComponent.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"

// Sets default values for this component's properties
UGoKartMovementComponent::UGoKartMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	UE_LOG(LogTemp, Warning, TEXT("Initialized Movement Component"));
}


// Called when the game starts
void UGoKartMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UGoKartMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	float ServerTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	if (GetOwnerRole() == ROLE_AutonomousProxy || GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		LastMove = CreateMove(DeltaTime, ServerTime);
		SimulateMove(LastMove);
	}
}

void UGoKartMovementComponent::SimulateMove(const FGoKartMove& Move)
{
	FVector Force = MaxDrivingForce * Move.Throttle * GetOwner()->GetActorForwardVector();
	Force += GetAirResistance();
	Force += GetRollingResistance();
	FVector Acceleration = Force / Mass;

	Velocity = Velocity + Acceleration * Move.DeltaTime;

	UpdateRotation(Move.DeltaTime, Move.SteeringThrow);
	UpdateLocationFromVelocity(Move.DeltaTime);
}


void UGoKartMovementComponent::UpdateRotation(float DeltaTime, float Steering)
{
	float deltaLocation =  FVector::DotProduct(GetOwner()->GetActorForwardVector(), Velocity) * DeltaTime;
	float RotationAngle = deltaLocation / MinTurningRadius * Steering;
	FQuat RotationDelta(GetOwner()->GetActorUpVector(), RotationAngle);
	Velocity = RotationDelta.RotateVector(Velocity);
	GetOwner()->AddActorWorldRotation(RotationDelta);
}

void UGoKartMovementComponent::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector DeltaTranslation = Velocity * 100 * DeltaTime;

	FHitResult hit;
	GetOwner()->AddActorWorldOffset(DeltaTranslation, true, &hit);
	if (hit.IsValidBlockingHit())
	{
		Velocity = FVector::Zero();
	}
}

FVector UGoKartMovementComponent::GetAirResistance()
{
	return -1 * Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

FVector UGoKartMovementComponent::GetRollingResistance()
{
	float GravityAcceleration = -1 * GetWorld()->GetGravityZ() / 100;
	float RollingResistanceForce = RollingResistanceCoefficient * GravityAcceleration;
	return -1 * Velocity.GetSafeNormal() * RollingResistanceForce;
}

FGoKartMove UGoKartMovementComponent::CreateMove(float DeltaTime, float ServerTime) const
{
	FGoKartMove Move;
	Move.DeltaTime = DeltaTime;
	Move.SteeringThrow = SteeringThrow;
	Move.Throttle = Throttle;
	Move.Time = ServerTime;
	return Move;
}

float UGoKartMovementComponent::GetRollingResistanceCoefficient() const
{
	return RollingResistanceCoefficient;
}

void UGoKartMovementComponent::SetVelocity(const FVector& NewVelocity)
{
	this->Velocity = NewVelocity;
}

float UGoKartMovementComponent::GetMass() const
{
	return Mass;
}

float UGoKartMovementComponent::GetMaxDrivingForce() const
{
	return MaxDrivingForce;
}

float UGoKartMovementComponent::GetMinTurningRadius() const
{
	return MinTurningRadius;
}

float UGoKartMovementComponent::GetDragCoefficient() const
{
	return DragCoefficient;
}

FVector UGoKartMovementComponent::GetVelocity() const
{
	return Velocity;
}

float UGoKartMovementComponent::GetThrottle() const
{
	return Throttle;
}

void UGoKartMovementComponent::SetThrottle(const float& Value)
{
	Throttle = Value;
}

float UGoKartMovementComponent::GetSteeringThrow() const
{
	return SteeringThrow;
}

void UGoKartMovementComponent::SetSteeringThrow(const float& Value)
{
	SteeringThrow = Value;
}



