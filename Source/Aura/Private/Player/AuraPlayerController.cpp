// Copyright Cieran Nicholas

#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Interaction/EnemyInterface.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	check(Subsystem);
	Subsystem->AddMappingContext(AuraContext, 0);

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2d InputAxisVector = InputActionValue.Get<FVector2d>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if(APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if(!CursorHit.bBlockingHit) return;

	LastHoveredActor = CurrentHoveredActor;
	CurrentHoveredActor = CursorHit.GetActor();

	/**
	 * Line trace from cursor. There are several scenarios to consider:
	 *	A. LastHoveredActor is null && CurrentHoveredActor is null
	 *		- Do nothing
	 *	B. LastHoveredActor is null && CurrentHoveredActor is valid
	 *		- Call CurrentHoveredActor->HighlightActor()
	 *	C. LastHoveredActor is valid && CurrentHoveredActor is null
	 *		- Call LastHoveredActor->UnhighlightActor()
	 *	D. LastHoveredActor is valid && LastHoveredActor != CurrentHoveredActor
	 *		- Call LastHoveredActor->UnhighlightActor() & CurrentHoveredActor->HighlightActor()
	 *	C. LastHoveredActor is valid && LastHoveredActor == CurrentHoveredActor
	 *		- Do nothing
	*/

	if (LastHoveredActor == nullptr)
	{
		if(CurrentHoveredActor != nullptr)
		{
			// Case B
			CurrentHoveredActor->HighlightActor();
		}
	}
	else // LastHoveredActor is valid
	{
		if(CurrentHoveredActor == nullptr)
		{
			// Case C
			LastHoveredActor->UnHighlightActor();
		}
		else // both actors are valid
		{
			if(LastHoveredActor != CurrentHoveredActor)
			{
				// Case D
				LastHoveredActor->UnHighlightActor();
				CurrentHoveredActor->HighlightActor();
			}
		}
	}
}
