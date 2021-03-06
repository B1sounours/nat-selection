#include <vector>
#include <optional>
#include <cassert>
#include <string>
#include <iostream>
#include "food.h"
#include "blob.h"
#include "simulation.h"
#include "simulationResults.h"

extern int g_nameHolder{ 0 };

void walkAndEat(std::vector<Blob> &blobArray, std::vector<Food> &foodArray, simulationResults &stats)
{
	bool hasStamina{ true };
	stats.recordBlobFrame(blobArray);
	for (int timeStep{ 0 }; hasStamina; ++timeStep)
	{
		hasStamina = false;
		int blobCount{ static_cast<int>(blobArray.size()) };
		for (int i{ 0 }; i < blobCount; ++i)
		{
			if (blobArray[i].getEnergy() >= blobArray[i].getCost())
			{
				hasStamina = true;
				if (blobArray[i].finishedStep())
				{
					int foodEaten{ blobArray[i].getFoodEaten() };
					if (foodEaten == 0)
					{
						blobArray[i].huntOrRun(blobArray, foodArray);
					}
					else if (foodEaten == 1 && blobArray[i].hasSurplusStamina())
					{
						blobArray[i].huntOrRun(blobArray, foodArray);
					}
					else
					{
						blobArray[i].goHome();
					}
				}
				blobArray[i].continueStep();
				std::optional<int> blobEaten{ blobArray[i].tryToEat(blobArray, foodArray) };

				//if a prey blob was eaten
				if (blobEaten.has_value())
				{
					//adjust the loop size
					--blobCount;
					if (i > blobEaten.value())
					{
						/*if eaten blob was before hunting blob in the array
						then hunting blob element value has been reduced by one,
						as all blobs after the eaten have shifted down one in stack,
						filling the	gap*/
						--i;
					}
				}
			}
		}
	stats.recordBlobFrame(blobArray);
	}
}
	
/*
	do
	{
		bool staminaCheck = false;
		int length{ static_cast<int>(blobArray.size()) };
		//Before any steps are taken, capture the beginning frame
		stats.recordBlobFrame(blobArray);
		for (int i{ 0 }; i < length; ++i)
		{
			std::array<int, 2> positionOne{ blobArray[i].getXPosition(), blobArray[i].getYPosition() };
			
			//Start at one so blobs with speed < 1 don't take a step
			for (int step{ 1 }; step < blobArray[i].getSpeed(); ++step)
			{
				if (blobArray[i].getEnergy() >= blobArray[i].getCost())
				{
					int foodEaten{ blobArray[i].getFoodEaten() };
					std::optional<int> blobEaten{ std::nullopt };
					staminaCheck = true;
					if (foodEaten == 0)
					{
						blobEaten = blobArray[i].huntOrRun(blobArray, foodArray);
					}
					else if (foodEaten == 1 && blobArray[i].hasSurplusStamina())
					{
						blobEaten = blobArray[i].huntOrRun(blobArray, foodArray);
					}
					else
					{
						blobArray[i].goHome();
					}
					//if a prey blob was eaten
					if (blobEaten.has_value())
					{
						//adjust the loop size
						--length;
						if (i > blobEaten.value())
						{
							/*if the eaten blob was before the hunting blob in the array
							then the hunting blob element value has been reduced by one,
							as all blobs after the eaten have shifted down one in stack,
							filling the	gap
							--i;
						}
					}
					/*If food was not eaten, reduce energy and record step.
					otherwise the huntOrRun action was to eat, no step taken,
					so go back through the loop
					else if (blobArray[i].getFoodEaten() == foodEaten)
					{
						blobArray[i].reduceEnergy();
						stats.recordBlobFrame(blobArray);
					}
				}
			}
		}
	} while (staminaCheck);
*/


void naturalSelection(std::vector<Blob> &blobArray)
{
	/*how to delete objects from a vector.
	"it" is the "iterator pointer": points to current element of vector.*/
	for (auto it = blobArray.begin(); it != blobArray.end();)
	{
		// remember A->function() is same as (*A).function()
		if (!it->atHome() || !it->getFoodEaten())
		{
			//erase blob
			it = blobArray.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void breed(std::vector<Blob> &blobArray)
{
	int length{ static_cast<int>(blobArray.size()) };
	for (int i{ 0 }; i < length; ++i)
	{
		std::optional<Blob> clone{ blobArray[i].tryToReplicate() };
		if (clone.has_value())
		{
			//To get correct number for new blob name
			clone.value().setName(g_nameHolder);
			++g_nameHolder;
			blobArray.push_back(clone.value());
		}
	}
}

void digestAndSleep(std::vector<Blob> &blobArray)
{
	int length{ static_cast<int>(blobArray.size()) };
	for (int i{ 0 }; i < length; ++i)
	{
		blobArray[i].setFoodEaten(0);
		blobArray[i].sleep();
	}
}
