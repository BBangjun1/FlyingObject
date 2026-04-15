#include "FlightPawn.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

AFlightPawn::AFlightPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// 캡슐 컴포넌트 생성 및 루트로 설정
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	RootComponent = CapsuleComp;
	CapsuleComp->SetSimulatePhysics(false);

	// 메쉬 컴포넌트 생성 및 캡슐에 부착
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);

	// 스프링암 생성 및 캡슐에 부착
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 400.f; // 비행체와 카메라 사이의 거리
	SpringArmComp->bUsePawnControlRotation = false; 

	// 카메라 생성 및 스프링암 끝에 부착
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

}

void AFlightPawn::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultIMC)
			{
				Subsystem->AddMappingContext(DefaultIMC, 0);
			}
		}
	}
}

void AFlightPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 마우스 상하 이동(Y)은 Pitch, 좌우 이동(X)은 Yaw에 적용합니다.
	FRotator NewRotation = FRotator(-LookInput.Y, -LookInput.X, RollInput) * RotationSpeed * DeltaTime;

	// 현재 로컬 회전값을 기준으로 회전을 더해줍니다.
	AddActorLocalRotation(NewRotation);
	LookInput = FVector2D::ZeroVector;
	RollInput = 0.f;

	FVector TraceStart = GetActorLocation();
	// 캡슐의 절반 높이 + 여유분 10cm 
	FVector TraceEnd = TraceStart + (FVector::DownVector * (CapsuleComp->GetScaledCapsuleHalfHeight() + 10.f));

	FHitResult HitResult;
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);

	// 월드에 레이저를 쏴서 뭔가 맞았는지 확인합니다.
	bIsGrounded = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, TraceParams);

	float CurrentMaxSpeed = MoveSpeed;

	if (!bIsGrounded)
	{
		if (LiftInput > 0.f)
		{
			Velocity.Z = 0.f; 
		}
		else
		{
			Velocity.Z += Gravity * DeltaTime;
		}

		CurrentMaxSpeed = MoveSpeed * 0.4f;
	}
	else
	{
		if (Velocity.Z < 0) Velocity.Z = 0.f;
	}

	FVector LocalInput = FVector(MoveInput.Y, MoveInput.X, LiftInput).GetSafeNormal();

	FVector TargetLocalVelocity = LocalInput * CurrentMaxSpeed;

	CurrentLocalVelocity = FMath::VInterpTo(CurrentLocalVelocity, TargetLocalVelocity, DeltaTime, 5.0f);

	FVector ForwardMove = GetActorForwardVector() * CurrentLocalVelocity.X;
	FVector RightMove = GetActorRightVector() * CurrentLocalVelocity.Y;
	FVector UpMove = GetActorUpVector() * CurrentLocalVelocity.Z;

	FVector PlayerMovement = (ForwardMove + RightMove + UpMove) * DeltaTime;

	if (bIsGrounded && PlayerMovement.Z < 0.f)
	{
		PlayerMovement.Z = 0.f;
	}

	FVector FinalMovement = PlayerMovement + (FVector(0.f, 0.f, Velocity.Z) * DeltaTime);
	AddActorWorldOffset(FinalMovement, true);

	MoveInput = FVector2D::ZeroVector;
	LiftInput = 0.f;
}

void AFlightPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AFlightPawn::Move);
		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AFlightPawn::Look);
		EnhancedInputComponent->BindAction(IA_Lift, ETriggerEvent::Triggered, this, &AFlightPawn::Lift);
		EnhancedInputComponent->BindAction(IA_Roll, ETriggerEvent::Triggered, this, &AFlightPawn::Roll);
	}
}

void AFlightPawn::Move(const FInputActionValue& Value)
{
	// 키보드 입력을 2D 벡터로 받아 저장합니다.
	MoveInput = Value.Get<FVector2D>();
}

void AFlightPawn::Look(const FInputActionValue& Value)
{
	// 마우스 입력을 2D 벡터로 받아 저장합니다.
	LookInput = Value.Get<FVector2D>();
}

void AFlightPawn::Lift(const FInputActionValue& Value)
{
	LiftInput = Value.Get<float>(); // 1D 값이므로 float으로 받습니다.
}

void AFlightPawn::Roll(const FInputActionValue& Value)
{
	RollInput = Value.Get<float>();
}
