// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SubSaveGame.generated.h"

/**
 * 
 */

// derived save game class
//store all actors in FActorSaveData struct and seriazable actor components data in FComponentSaveData


// for "global" objects like player state
// and for objects that require custom serialization
// i recommend expand this class by additional structures 



USTRUCT(BlueprintType)
struct FComponentSaveData {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FTransform ComponentTransform;

	UPROPERTY(SaveGame)
	FName ComponentName;

	UPROPERTY(SaveGame)
	TArray<uint8> ComponentByteData;
	
};

USTRUCT(BlueprintType)
struct FActorSaveData {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	UClass* ActorClass; 

	UPROPERTY(SaveGame)
	FName ActorName;

	UPROPERTY(SaveGame)
	FTransform ActorTransform;

	UPROPERTY(SaveGame)
	FVector ActorCentroid; // https://en.wikipedia.org/wiki/Centroid

	// was created with though of saving static mesh components of struct assigned with FActorSaveData
	// help if you have a custom static mesh component classes with lot of functionality 
	UPROPERTY(SaveGame)
	TArray<FComponentSaveData> ComponentsData; 

	UPROPERTY(SaveGame)
	TArray<uint8> ActorByteData;

	
	// for sorting purposes
	bool operator>(const FActorSaveData& Other)const
	{
		return (ActorTransform.GetLocation().Size() > Other.ActorTransform.GetLocation().Size());
	}
};

USTRUCT(BlueprintType)
struct FSaveFileInfo {
	GENERATED_BODY()

	UPROPERTY()
	FName ProfileName;// player profile name

	UPROPERTY()
	FDateTime Date;// no girls present

	UPROPERTY()
	int TimePlayed; // in seconds

	UPROPERTY()
	FName LevelName;// current level name
	
};


UCLASS()
class SAVESUBSYSTEM_API USubSaveGame : public USaveGame {
	GENERATED_BODY()
	
public:
	UPROPERTY(SaveGame)
	TArray<FActorSaveData> SavedActors;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame) //TODO should expose?
	FSaveFileInfo SaveID;
	
	//UFUNCTION()
	//const FSaveFileInfo& GetSaveId()const  { return SaveID;} 
	
	UFUNCTION(BlueprintCallable)
	void SetSaveID(FName IDName, FDateTime IDDate, int IDTime , FName IDLevelName)
	{
		SaveID.ProfileName = IDName;
		SaveID.Date = IDDate;
		SaveID.TimePlayed = IDTime;
		SaveID.LevelName = IDLevelName;
	}

	UPROPERTY(BlueprintReadWrite,EditAnywhere)
	uint8 SaveNumber;

	UPROPERTY(BlueprintReadWrite,EditAnywhere)
	FName SlotName;


};
