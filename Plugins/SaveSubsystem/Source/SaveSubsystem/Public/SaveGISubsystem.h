// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SubSaveGame.h"
#include "SaveGISubsystem.generated.h"

/**
 * 
 */


class USubSaveGame;

DECLARE_LOG_CATEGORY_EXTERN(SubloadingSubsystemLOG,Log,All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveGameSignature, class USubSaveGame*, SaveGameObject);

UCLASS()
class SAVESUBSYSTEM_API USaveGISubsystem : public UGameInstanceSubsystem {
	GENERATED_BODY()
	
	UPROPERTY()
	uint8 CurrentSlotIndex = 0;

	FTimerHandle LoadGameHandle;

	UPROPERTY()//EditAnywhere,BlueprintReadWrite
	FString CurrentSlotName; // save slot name

	UPROPERTY()
	USubSaveGame* CurrentSaveGame; // current save game object

	// current level name stored in save object - loading this level on is not implemented
	UPROPERTY()
	FName LevelName; 


protected:
	// load one actor
	void SimpleLoadActor(AActor* ActorToLoad,const FActorSaveData& Record);

	// save one actor and its components
	void CreateSingleActorData(AActor* ActorToSave);

	// save one actor  static mesh component
	FComponentSaveData CreateSingleComponentData(UStaticMeshComponent* ComponentToSave);

public:
	
	// delegate broadcasted after all actors are saved
	UPROPERTY(BlueprintAssignable, Category= "SaveSubsystem")
	FOnSaveGameSignature OnSaveGameWritten;

	// delegate broadcasted after all actors are loaded - if you need some logic that require for all actors to load
	// subscribing to this event its good idea
	UPROPERTY(BlueprintAssignable, Category= "SaveSubsystem")
	FOnSaveGameSignature OnSaveGameLoaded;

	/* Initialize Subsystem, good moment to load in SaveGameSettings variables */
	void Initialize(FSubsystemCollectionBase& Collection) override;
	

	
	// get current save game objects
	UFUNCTION(BlueprintCallable)
	FORCEINLINE USubSaveGame* GetCurrentSaveGame() const {return CurrentSaveGame;}

	// current level name is stored in save object
	UFUNCTION(BlueprintCallable)
	void SetCurrentLevel(FName InLevelName){LevelName = InLevelName;}
	
	// set current save object
	UFUNCTION(BlueprintCallable)
	void SetCurrentSave(const FString& InSaveName, USubSaveGame* InSaveGame,bool UpdatePlaytime);

	// update play time stored in save object
	UFUNCTION(BlueprintCallable)
	void UpdateSessionTime(const int Value);

	// create new save game object
	// created object will be named as quick save
	// this function does not save the game ,
	UFUNCTION(BlueprintCallable)
	USubSaveGame* CreateNewGameSave(FString& OutSlotName ,const int SlotIndex, TSubclassOf<USubSaveGame> SaveClass);

	// create new save game object
	// creted object will be named as auto save
	// this function does not save the game ,
	UFUNCTION(BlueprintCallable)
	USubSaveGame* CreateNewAutoSave(FString& OutSlotName,const int SlotIndex,TSubclassOf<USubSaveGame> SaveClass);

	// perform game save
	// get all actors that implementing saveableInterface and serialize their properties
	// template used to determine which actor is which can be easy changed depending on your needs
	UFUNCTION(BlueprintCallable, Category = "SaveSubsystem")
	void WriteSaveGame();

	// perform game load
	// handle destroying "not saved " actors
	//handle spawning actors that are missing in level ( spawned during gameplay etc.)
	UFUNCTION(BlueprintCallable, Category = "SaveSubsystem")
	void LoadSaveGame();
	
	UFUNCTION(BlueprintCallable,BlueprintPure)
	bool IsReadyToLoad();

	
	
};


//TODO player profiles: check any references. save player profile name in save file name?
//TODO save hours played to determine order of saves
//TODO store saved games associated with player profiles in different folders
//TODO save version backward compability gsdfghdfhsdfasdfgdfhg

