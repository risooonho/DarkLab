// Fill out your copyright notice in the Description page of Project Settings.

#include "DarknessController.h"
#include "Darkness.h"
#include "MainPlayerController.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/Character.h"

// Called on disabling a character
void ADarknessController::OnDisabling()
{
	StartRetreating();
}

// Called when player gets the black doorcard
void ADarknessController::OnPlayerFindsBlackCard()
{
	bIsPersistent = true;
	StartHunting();
}

// Teleports to some point closer to character
void ADarknessController::TeleportToCharacter()
{
	if (SinceLastTeleport < MinTimeBetweenTeleports || Darkness->TimeInDark < MinTimeInDark)
		return;

	APlayerController* controller = GetWorld()->GetFirstPlayerController();
	if (!controller)
		return;
	ACharacter* character = controller->GetCharacter();
	if (!character)
		return;

	FVector charLocation = character->GetActorLocation();

	if ((charLocation - Darkness->GetActorLocation()).Size2D() < MinTeleportDistance)
		return;

	// Making random direction
	FVector direction;
	float L;
	do
	{
		// Check random vectors in the unit circle so result is statistically uniform.
		direction.X = FMath::FRand() * 2.f - 1.f;
		direction.Y = FMath::FRand() * 2.f - 1.f;
		L = direction.SizeSquared();
	} 
	while (L > 1.0f || L < KINDA_SMALL_NUMBER);
	direction *= (1.0f / FMath::Sqrt(L));

	// TODO check if that place is not lit
	FVector destination = charLocation + direction * MinTeleportDistance;

	Darkness->TeleportToLocation(destination);
	SinceLastTeleport = 0.f;

	UE_LOG(LogTemp, Warning, TEXT("Teleported"));
}

// Stops everything, enters passive state
void ADarknessController::BecomePassive()
{
	State = EDarkStateEnum::VE_Passive;
	Darkness->Stop();

	CurrentMaxTimePassive = FMath::FRandRange(MinTimePassive, MaxTimePassive);
	SinceLastStateChange = 0.f;

	UE_LOG(LogTemp, Warning, TEXT("Entering passive state"));
}

// Teleports somewhere and starts following the player
void ADarknessController::StartHunting()
{	
	APlayerController* controller = GetWorld()->GetFirstPlayerController();
	if (!controller)
		return;

	AMainPlayerController* mainController = Cast<AMainPlayerController>(controller);
	if (!mainController)
		return;

	// Don't hunt anymore
	if (mainController->Lives <= 0)
		return;

	ACharacter* character = controller->GetCharacter();
	if (!character)
		return;
	
	State = EDarkStateEnum::VE_Hunting;
	CurrentMaxTimeHunting = FMath::FRandRange(MinTimeHunting, MaxTimeHunting);
	SinceLastStateChange = 0.f;
	Darkness->MoveToActor((AActor*)character);

	UE_LOG(LogTemp, Warning, TEXT("Starting the hunt"));
}

// Stops following the player and retreats into the darkness
void ADarknessController::StartRetreating()
{
	State = EDarkStateEnum::VE_Retreating;
	Darkness->Stop();

	SinceLastStateChange = 0.f;

	UE_LOG(LogTemp, Warning, TEXT("Retreating into darkness"));
}

// Called when the game starts or when spawned
void ADarknessController::BeginPlay()
{
	Super::BeginPlay();

	// We find the darkness
	Darkness = Cast<ADarkness>(GetPawn());	
	if (!Darkness)
		UE_LOG(LogTemp, Warning, TEXT("No Darkness"));

	// Then we set the state
	BecomePassive();
}

// Called every frame
void ADarknessController::Tick(const float deltaTime)
{
	Super::Tick(deltaTime);

	SinceLastStateChange += deltaTime;
	SinceLastTeleport += deltaTime;

	// Darkness retreats from powerful light sources
	bool isAfraid = Darkness->RetreatFromLight();

	// Do something based on state if it isn't afraid already
	if (!isAfraid)
	{
		switch (State)
		{
		case EDarkStateEnum::VE_Passive:
			// If its been some time, start the hunt
			if (SinceLastStateChange >= CurrentMaxTimePassive)
				StartHunting();
			// TODO else?
			break;
		case EDarkStateEnum::VE_Hunting:
			// If hunting for some time, start retreating
			if (!bIsPersistent && SinceLastStateChange >= CurrentMaxTimeHunting)
				StartRetreating();
			// Otherwise keep hunting
			else
			{
				// Tries to teleport if possible
				TeleportToCharacter();
				// Then just moves
				Darkness->Tracking();
			}
			break;
		case EDarkStateEnum::VE_Retreating:
			// If retreating for too long or if already escaped into darkness, become passive
			if (SinceLastStateChange >= MaxTimeRetreating || Darkness->TimeInDark >= MinTimeInDark)
				BecomePassive();
			// Otherwise keep retreating
			else
				Darkness->IntoDarkness();
			break;
		}
	}
}