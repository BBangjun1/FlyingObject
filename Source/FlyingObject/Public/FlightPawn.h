

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "FlightPawn.generated.h"

UCLASS()
class FLYINGOBJECT_API AFlightPawn : public APawn
{
	GENERATED_BODY()

public:
	AFlightPawn();

protected:
	virtual void BeginPlay() override;

	// 현재 프레임의 입력값을 임시 저장할 변수
	FVector2D MoveInput;
	FVector2D LookInput;
	float LiftInput;
	float RollInput;

	// 입력 액션과 바인딩될 실제 함수
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Lift(const FInputActionValue& Value);
	void Roll(const FInputActionValue& Value);

	// 중력 및 상태 변수
	UPROPERTY(EditAnywhere, Category = "Movement")
	float Gravity = -980.f;
	FVector Velocity;
	FVector CurrentLocalVelocity;
	bool bIsGrounded;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 충돌 영역
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UCapsuleComponent* CapsuleComp;

	// 비행체 외형
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USkeletalMeshComponent* MeshComp;

	// 메라 지지대
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USpringArmComponent* SpringArmComp;

	// 카메라
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UCameraComponent* CameraComp;

	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputMappingContext* DefaultIMC;

	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* IA_Move;

	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* IA_Look;

	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* IA_Lift;

	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* IA_Roll; 

	// 이동 및 회전 속도
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MoveSpeed = 600.f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float RotationSpeed = 100.f;
};
