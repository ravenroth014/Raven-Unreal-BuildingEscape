// Copyright to Atid Puwatnuttasit 2017

#include "Grabber.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"

#define OUT

// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UGrabber::BeginPlay()
{
	Super::BeginPlay();
	FindPhysicsHandleComponent();
	SetupInputComponent();
}

void UGrabber::FindPhysicsHandleComponent()
{
	/// Look for attached Physics Handle
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (PhysicsHandle == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s physics handle is not found."), *GetWorld()->GetName());
	}
}

void UGrabber::SetupInputComponent()
{
	/// Look for attached Input Component (only appear at run time)
	InputComponent = GetOwner()->FindComponentByClass<UInputComponent>();
	if (InputComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Input component is found."));
		/// Bind the input axis
		InputComponent->BindAction("Grab", IE_Pressed, this, &UGrabber::Grab);
		InputComponent->BindAction("Grab", IE_Released, this, &UGrabber::Release);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s input component is not found."), *GetWorld()->GetName());
	}
}

const FHitResult UGrabber::GetFirstPhysicsBodyInReach()
{
	/// Draw a red trace in the world to visualize
	//DrawDebugLine(
	//	GetWorld(),
	//	PlayerViewPointLocation,
	//	LineTraceEnd,
	//	FColor(255, 0, 0),
	//	false,
	//	0.0f,
	//	0,
	//	5
	//);

	/// Setup query parameters
	FCollisionQueryParams TraceParameters(FName(TEXT("")), false, GetOwner());
	/// Line-trace (AKA ray-cast) out to reach distance
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByObjectType(
		OUT HitResult,
		GetReachLineStart(),
		GetReachLineEnd(),
		FCollisionObjectQueryParams(ECC_PhysicsBody),
		TraceParameters
	);

	return HitResult;
}

void UGrabber::Grab()
{
	UE_LOG(LogTemp, Warning, TEXT("Grab pressed"));

	/// LINE TRACE and see if we reach any actors with physics body collision channel set
	auto HitResult = GetFirstPhysicsBodyInReach();
	auto ComponentToGrab = HitResult.GetComponent();
	auto ActorHit = HitResult.GetActor();

	if(!ActorHit) {return;}

	// If we hit something then attach a physics handle
	if(ActorHit)
	{
		// attach physics handle
		PhysicsHandle->GrabComponentAtLocation(ComponentToGrab, NAME_None, ComponentToGrab->GetOwner()->GetActorLocation());
	}	
}

void UGrabber::Release()
{
	UE_LOG(LogTemp, Warning, TEXT("Grab released"));
	
	if (!PhysicsHandle) { return; }
	PhysicsHandle->ReleaseComponent();
}

// Called every frame
void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(!PhysicsHandle) {return;}

	// if the physics handle is attached
	if(PhysicsHandle->GrabbedComponent)
	{
		// move the object that we're holding
		PhysicsHandle->SetTargetLocation(GetReachLineEnd());
	}

}

FVector UGrabber::GetReachLineEnd()
{
	/// Get the player view point this tick
	FVector PlayerViewPointLocation;
	FRotator PlayerViewPointRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewPointLocation,
		OUT PlayerViewPointRotation
	);

	return PlayerViewPointLocation + (PlayerViewPointRotation.Vector() * Reach);
}

FVector UGrabber::GetReachLineStart()
{
	/// Get the player view point this tick
	FVector PlayerViewPointLocation;
	FRotator PlayerViewPointRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewPointLocation,
		OUT PlayerViewPointRotation
	);

	return PlayerViewPointLocation;
}
