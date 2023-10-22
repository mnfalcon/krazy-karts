// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKartMovementReplicator.h"
#include "GoKart.h"
#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UGoKartMovementReplicator::UGoKartMovementReplicator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	SetIsReplicated(true);
	UE_LOG(LogTemp, Warning, TEXT("Initialized Replicator Component"));
}


// Called when the game starts
void UGoKartMovementReplicator::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent = GetOwner()->FindComponentByClass<UGoKartMovementComponent>();
}

void UGoKartMovementReplicator::BeginDestroy()
{
	Super::BeginDestroy();
}

// Called every frame
void UGoKartMovementReplicator::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (MovementComponent == nullptr) return;

	FGoKartMove Move = MovementComponent->GetLastMove();

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		AddUnacknowledgedMove(Move);
		Server_SendMove(Move);
	}
	// server in control of the pawn
	else if (GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		UpdateServerState(Move);
	}
	else if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		// MovementComponent->SimulateMove(ServerState.LastMove);
		ClientTick(DeltaTime);
	}
}

bool UGoKartMovementReplicator::Server_SendMove_Validate(FGoKartMove Move)
{
	float ProposedTime = ClientSimulatedTime + Move.DeltaTime;

	bool ClientNotRunningAhead = ProposedTime < GetWorld()->TimeSeconds;
	if (ClientNotRunningAhead)
	{
		UE_LOG(LogTemp, Error, TEXT("Client is running too fast"));
		return false;
	}

	if (!Move.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid move"));
		return false;
	}
	
	return true;
}

void UGoKartMovementReplicator::Server_SendMove_Implementation(FGoKartMove Move)
{
	if (MovementComponent == nullptr) return;

	ClientSimulatedTime += Move.DeltaTime;
	MovementComponent->SimulateMove(Move);

	UpdateServerState(Move);
}

void UGoKartMovementReplicator::OnRep_ServerState()
{
	switch (GetOwnerRole())
	{
	case ROLE_AutonomousProxy:
		AutonomousProxy_OnRep_ServerState();
		break;
	case ROLE_SimulatedProxy:
		SimulatedProxy_OnRep_ServerState();
		break;
	default:
		break;
	}
}

void UGoKartMovementReplicator::SimulatedProxy_OnRep_ServerState()
{
	ClientTimeBetweenLastUpdates = ClientTimeSinceUpdate;
	ClientTimeSinceUpdate = 0;

	if (MeshOffsetRoot != nullptr)
	{
		ClientStartTransform.SetLocation(MeshOffsetRoot->GetComponentLocation());
		ClientStartTransform.SetRotation(MeshOffsetRoot->GetComponentQuat());
	}
	else
	{
		ClientStartTransform = GetOwner()->GetActorTransform();
	}
	if (MovementComponent == nullptr) return;
	ClientStartVelocity = MovementComponent->GetVelocity();

	GetOwner()->SetActorTransform(ServerState.Transform);
}

void UGoKartMovementReplicator::AutonomousProxy_OnRep_ServerState()
{
	if (MovementComponent == nullptr) return;
	GetOwner()->SetActorTransform(ServerState.Transform);
	MovementComponent->GetVelocity() = ServerState.Velocity;
	ClearAcknowledgedMoves(ServerState.LastMove);

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		MovementComponent->SimulateMove(Move);
	}
}

FHermiteCubicSpline UGoKartMovementReplicator::CreateSpline()
{
	FHermiteCubicSpline Spline;
	Spline.TargetLocation = ServerState.Transform.GetLocation();
	Spline.StartLocation = ClientStartTransform.GetLocation();
	Spline.StartDerivative = ClientStartVelocity * GetVelocityToDerivative();
	Spline.TargetDerivative = ServerState.Velocity * GetVelocityToDerivative();
	return Spline;
}

float UGoKartMovementReplicator::GetVelocityToDerivative()
{
	return ClientTimeBetweenLastUpdates * 100; // multiplied by 100 because of UE conversion of m to cm
}

FVector UGoKartMovementReplicator::GetNewVelocity(FHermiteCubicSpline Spline, float LerpRatio)
{
	FVector NewDerivative = Spline.InterpolateDerivative(LerpRatio);
	return NewDerivative / GetVelocityToDerivative();
}

void UGoKartMovementReplicator::InterpolateLocation(const float& LerpRatio, const FHermiteCubicSpline& Spline) const
{
	FVector NextLocation = Spline.InterpolateLocation(LerpRatio);
	if (MeshOffsetRoot != nullptr)
	{
		MeshOffsetRoot->SetWorldLocation(NextLocation);
	}
	else
	{
		GetOwner()->SetActorLocation(NextLocation);
	}
}

void UGoKartMovementReplicator::InterpolateDerivative(const float& LerpRatio, const FHermiteCubicSpline& Spline)
{
	FVector NewVelocity = GetNewVelocity(Spline, LerpRatio);
	MovementComponent->SetVelocity(NewVelocity);
}

void UGoKartMovementReplicator::InterpolateRotation(const float& LerpRatio) const
{
	FQuat StartRotation = ClientStartTransform.GetRotation();
	FQuat TargetRotation = ServerState.Transform.GetRotation();
	FQuat NextRotation = FQuat::Slerp(StartRotation, TargetRotation, LerpRatio);

	if (MeshOffsetRoot != nullptr)
	{
		MeshOffsetRoot->SetWorldRotation(NextRotation);
	}
	else
	{
		GetOwner()->SetActorRotation(NextRotation);
	}
}

void UGoKartMovementReplicator::ClientTick(float DeltaTime)
{
	ClientTimeSinceUpdate += DeltaTime;

	if (ClientTimeBetweenLastUpdates < KINDA_SMALL_NUMBER) return;
	if (MovementComponent == nullptr) return;

	float LerpRatio = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdates;

	FHermiteCubicSpline Spline = CreateSpline();

	InterpolateLocation(LerpRatio, Spline);

	InterpolateDerivative(LerpRatio, Spline);

	InterpolateRotation(LerpRatio);
}

void UGoKartMovementReplicator::UpdateServerState(const FGoKartMove& Move)
{
	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}

void UGoKartMovementReplicator::ClearAcknowledgedMoves(FGoKartMove LastMove)
{
	TArray<FGoKartMove> NewMoves;

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		if (Move.Time > LastMove.Time)
		{
			NewMoves.Add(Move);
		}
	}

	UnacknowledgedMoves = NewMoves;
}

void UGoKartMovementReplicator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGoKartMovementReplicator, ServerState);
}

void UGoKartMovementReplicator::AddUnacknowledgedMove(FGoKartMove& Move)
{
	UnacknowledgedMoves.Add(Move);
}
