// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SaveGISubsystem.h"
#include "SaveSubsystemBFL.generated.h"

/**
 * 
 */
UCLASS()
class SAVESUBSYSTEM_API USaveSubsystemBFL : public UBlueprintFunctionLibrary {
	GENERATED_BODY()

public:

	
	//delete save game
	UFUNCTION(BlueprintCallable)
	static void DeleteSavedGame(const FString& InSlotName,const int SlotIndex);
	
	// get last saved game ex. for "continue" button
	UFUNCTION(BlueprintCallable)
	static USubSaveGame* GetLastSavedGame(const int SlotIndex);
	

	// return all save game objects marked as autosaves- ex. you want to show all avalible saves in widget
	UFUNCTION(BlueprintCallable)
	static void GetAllAutoSaveGames(TMap<FString,USubSaveGame*>& OutAutoSaveGameSaves, const int SlotIndex);
	
	// return all save game objects - ex. you want to show all avalible saves in widget
	UFUNCTION(BlueprintCallable)
	static TArray<USubSaveGame*> GetAllSaveGames(const int SlotIndex);

	
	UFUNCTION(BlueprintCallable)
	static USubSaveGame* LoadSaveObjectIfExist(const FString& Name, const int Index);
	
	//return all save game slot names 
	UFUNCTION(BlueprintCallable)
	static void GetAllSaveGameNames(TArray<FString>& OutSlotNames,const int SlotIndex);
	
};
