// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SubSaveGame.h"
#include "SaveableInterface.generated.h"

// This class does not need to be modified.

// implement in class if you want to use the save susbsystem

UINTERFACE()
class USaveableInterface : public UInterface {
	GENERATED_BODY()
};

/**
 * 
 */
class SAVESUBSYSTEM_API ISaveableInterface {
	GENERATED_BODY()
public:
	// called before actor serialization - if you have custom class setup that need to be done before saving use this
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable,Category= "Saving")
	bool BeforeActorSaved(FActorSaveData& PartData);

	// called after loading actor - if you have custom class setup that need to be done after actor loading use this
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Saving")
	bool AfterActorLoaded(const FActorSaveData& ActorSaveData);

	// called before component serialization - if you have really really custom setup that need to be done before saving use this
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable,Category = "Saving")
	bool BeforeComponentSaved(FComponentSaveData ParentData);
	
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Saving")
	bool AfterComponentLoaded(const FComponentSaveData& ComponentSaveData);
	

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
};
