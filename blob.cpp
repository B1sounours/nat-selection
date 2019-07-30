#include <string>
#include <optional>
#include <vector>
#include "thing.h"
#include "food.h"
#include "randNum.h"
#include "blob.h"

extern int g_mutationProb{ 10 };

Blob::Blob()
	: m_nativeEnergy{ 1800.0 }, m_size{ 3.0 }, m_speed{ 3.0 }, m_sense{ 3.0 }
{
}


Blob::Blob(double nativeEnergy, double size, double speed, double sense)
	: m_nativeEnergy{ nativeEnergy }, m_size{ size }, m_speed{ speed }, m_sense{ sense }
{
}

std::string Blob::getName()
{
	return m_name;
}

void Blob::setName(std::string name)
{
	m_name = name;
}

double Blob::getSense()
{
	return m_sense;
}

void Blob::setSense(double x)
{
	m_sense = x;
}

double Blob::getSpeed()
{
	return m_speed;
}

void Blob::setSpeed(double x)
{
	m_speed = x;
}

double Blob::getCost()
{
	double cost = (m_size*m_size*m_size) + (m_speed*m_speed) + m_sense;
	return cost;
}

double Blob::getEnergy()
{
	return m_energy;
}

void Blob::reduceEnergy()
{
	m_energy -= getCost();
}

void Blob::setEnergy(double x)
{
	m_energy = x;
}

double Blob::getSize()
{
	return m_size;
}

void Blob::setSize(double x)
{
	m_size = x;
}

void Blob::setMapSize(int x)
{
	m_mapSize = x;
}

int Blob::getFoodEaten()
{
	return m_foodEaten;
}

void Blob::setFoodEaten(int x)
{
	m_foodEaten = x;
}

int Blob::getAge()
{
	return m_age;
}

void Blob::setAge(int x)
{
	m_age = x;
}

void Blob::sleep()
{
	m_energy = m_nativeEnergy;
}

/*Using convention that North and East are +ve and 
South and West are -ve (cartesian)*/
void Blob::stepNorth()
{
	++m_yPosition;
}

void Blob::stepSouth()
{
	--m_yPosition;
}

void Blob::stepEast()
{
	++m_xPosition;
}

void Blob::stepWest()
{
	--m_xPosition;
}

void Blob::randomStep()
{
	/*create an array of function pointers, 
	specifically pointers to functions inside Blob class, 
	that take no inputs and return void*/
	std::vector<void(Blob::*)()> compass;

	// if the blob isn't against the east boundary
	if (m_xPosition < m_mapSize)
	{
		//add a function pointer to the array
		//pointing to address of stepEast
		compass.push_back(&Blob::stepEast);
	}
	else if (m_xPosition > 1)
	{
		compass.push_back(&Blob::stepWest);
	}
	if (m_yPosition < m_mapSize)
	{
		compass.push_back(&Blob::stepNorth);
	}
	else if (m_yPosition > 1)
	{
		compass.push_back(&Blob::stepSouth);
	}
	//Randomly choose direction and take one step
	int num{ getRandomNumber(1, (compass.size())) };
	(this->*compass[num - 1])();
}

void Blob::stepTowards(Thing &thing)
{
	int xdif = thing.getXPosition() - m_xPosition;
	int ydif = thing.getYPosition() - m_yPosition;
	(std::abs(xdif) > std::abs(ydif)) ?
		(m_xPosition += xdif / std::abs(xdif)) : 
		(m_yPosition += ydif / std::abs(ydif));
}

void Blob::stepAway(Thing &thing)
{
	int xdif = thing.getXPosition() - m_xPosition;
	int ydif = thing.getYPosition() - m_yPosition;
	(std::abs(xdif) > std::abs(ydif)) ?
		(m_xPosition -= xdif / std::abs(xdif)) : 
		(m_yPosition -= ydif / std::abs(ydif));
}

bool Blob::atFood(Food &food)
{
	if (m_xPosition == food.getXPosition() &&
		m_yPosition == food.getYPosition())
	{
		return true;
	}
	return false;
}

std::optional<int> Blob::huntOrRun(std::vector<Food> &foodArray, std::vector<Blob> &blobArray)
{
	std::optional<int> predOpt = lookForPredator(blobArray);
	std::optional<int> foodOpt = lookForFood(foodArray);
	std::optional<int> preyOpt = lookForPrey(blobArray);
	//guaranteed larger than any dist to object on map
	int foodDist{ m_mapSize * 10 }; 
	int preyDist{ m_mapSize * 10 };
	int predDist{ m_mapSize * 10 };

	if (foodOpt.has_value())
	{
		int foodElement{ foodOpt.value() };
		foodDist = distToObject(foodArray[foodElement]);
	}
	if (preyOpt.has_value())
	{
		int preyElement{ preyOpt.value() };
		preyDist = distToObject(blobArray[preyElement]);
	}
	if (predOpt.has_value())
	{
		int predElement{ predOpt.value() };
		predDist = distToObject(blobArray[predElement]);
	}
	// if none of the above have value take random step
	else if (foodDist == preyDist && preyDist == predDist)
	{
		randomStep();
		return std::nullopt;
	}

	if (foodDist == 0) //if ontop of food
	{
		//erase that food element
		auto it = foodArray.begin();
		foodArray.erase(it + foodOpt.value());
		setFoodEaten(getFoodEaten() + 1);
		return std::nullopt;
	}
	else if (preyDist == 0) //if ontop of prey
	{
		//erase that blob element
		auto it = blobArray.begin();
		blobArray.erase(it + preyOpt.value());
		setFoodEaten(getFoodEaten() + 1);
		return preyOpt.value();
	}
	/*if you start day sharing space with predator, i.e your
	child is larger than you*/
	if (predDist == 0)
	{
		randomStep();
		return std::nullopt;
	}
	if (predDist < ((foodDist < preyDist) ? foodDist : preyDist))
	{
		stepAway(blobArray[predOpt.value()]);
		return std::nullopt;
		}
	else if (predDist > ((foodDist < preyDist) ? foodDist : preyDist))
	{
		(foodDist < preyDist) ?
			stepTowards(foodArray[foodOpt.value()]) :
			stepTowards(blobArray[preyOpt.value()]);
		return std::nullopt;
	}
	else if (predDist = ((foodDist < preyDist) ? foodDist : preyDist))
	{
		if (getFoodEaten() == 0)
		{
			(foodDist < preyDist) ?
				stepTowards(foodArray[foodOpt.value()]) :
				stepTowards(blobArray[preyOpt.value()]);
		}
		else
		{
			stepAway(blobArray[predOpt.value()]);
		}
		return std::nullopt;
	}
	std::cout << "\nShould be possible to reach here\n";
	return std::nullopt;
}

int Blob::distToObject(Thing &object)
{
	int xdif = object.getXPosition() - m_xPosition;
	int ydif = object.getYPosition() - m_yPosition;
	int distAway{ std::abs(xdif) + std::abs(ydif) };
	return distAway;
}

	/* Lots of repetition here. Bound to be a smarter way.*/
std::optional<int> Blob::lookForFood(std::vector<Food> &foodArray)
{
	int dist = 10 * m_mapSize;
	int element;
	bool foundFood = false;

	//for every piece of food
	int length{ static_cast<int>(foodArray.size()) };
	for (int i = 0; i < length; ++i)
		{
		/*search the sense area around the blobs position to see if that food
		is within it. With sense as a double, the rounding down of int x means
		there is increased energetic cost (cost is a double) for every increase in sense
		but only an evolutionary advantage when sense increases by a whole unit. int x has
		to stay int because the tile spaces are in integers. */
		for (int x(m_xPosition - m_sense); x <= (m_xPosition + m_sense); ++x)
		{
			if (x == foodArray[i].getXPosition())
			{
				for (int y(m_yPosition - m_sense); y <= (m_yPosition + m_sense); ++y)
				{
					if (y == foodArray[i].getYPosition())
					{
						if (distToObject(foodArray[i]) < dist)
						{
							foundFood = true;
							dist = distToObject(foodArray[i]);
							element = i;
						}
					}
				}
			}
		}
	}
	if (foundFood)
	{
		return element;
	}
	else
	{
		return std::nullopt;
	}
}

std::optional<int> Blob::lookForPrey(std::vector<Blob> &blobArray)
{
	int dist = 10 * m_mapSize;
	int element;
	bool foundPrey = false;

	int length{ static_cast<int>(blobArray.size()) };
	for (int i = 0; i < length; ++i)
	{
		/*is blob is 80% or less of my size and isn't at home then it is legitimate prey.
		Let us see if it falls within my sense radius (i.e. can I see it)*/
		if (blobArray[i].getSize() <= (0.8*m_size) && !(blobArray[i].atHome()))
		{
			/*search the sense area around the blobs position to see if that food
			is within it. With sense as a double, the rounding down of int x means
			there is increased energetic cost (cost is a double) for every increase in sense
			but only an evolutionary advantage when sense increases by a whole unit. int x has
			to stay int because the tile spaces are in integers.*/
			for (int x(m_xPosition - m_sense); x <= (m_xPosition + m_sense); ++x)
			{
				if (x == blobArray[i].getXPosition())
				{
					for (int y(m_yPosition - m_sense); y <= (m_yPosition + m_sense); ++y)
						{
						if (y == blobArray[i].getYPosition())
						{
							if (distToObject(blobArray[i]) < dist)
							{
								foundPrey = true;
								dist = distToObject(blobArray[i]);
								element = i;
							}
						}
					}
				}
			}
		}
	}

	if (foundPrey)
	{
		return element;
	}
	else
	{
		return std::nullopt;
	}
}

std::optional<int> Blob::lookForPredator(std::vector<Blob> &blobArray)
{
	int dist = 10 * m_mapSize;
	int element;
	bool foundPred = false;

	//for every piece of food
	int length{ static_cast<int>(blobArray.size()) };
	for (int i = 0; i < length; ++i)
	{
		if ((0.8*blobArray[i].getSize()) >= m_size)
		{
			/*search the sense area around the blobs position to see if that food
			is within it. With sense as a double, the rounding down of int x means
			there is increased energetic cost (cost is a double) for every increase in sense
			but only an evolutionary advantage when sense increases by a whole unit. int x has
			to stay int because the tile spaces are in integers.*/
			for (int x(m_xPosition - m_sense); x <= (m_xPosition + m_sense); ++x)
			{
				if (x == blobArray[i].getXPosition())
				{
					for (int y(m_yPosition - m_sense); y <= (m_yPosition + m_sense); ++y)
					{
						if (y == blobArray[i].getYPosition())
						{
							if (distToObject(blobArray[i]) < dist)
							{
								foundPred = true;
								dist = distToObject(blobArray[i]);
								element = i;
							}
						}
					}
				}
			}
		}
	}
	if (foundPred)
	{
		return element;
	}
	else
	{
		return std::nullopt;
	}
}

bool Blob::atHome()
{
	if (m_xPosition == 0 || m_xPosition == m_mapSize + 1)
	{
		return true;
	}
	else if (m_yPosition == 0 || m_yPosition == m_mapSize + 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Blob::goHome()
{
	if (atHome())
	{
		m_energy = 0.0;
		return;
	}
	else
	{
		//find which coord is furthest from the midpoint
		//and thus closest to edge, and step in that direction
		int xdif = m_xPosition - (m_mapSize / 2);
		int ydif = m_yPosition - (m_mapSize / 2);
		if (xdif == ydif) //special case if blob is at centre
		{
			randomStep();
		}
		else
		{
			(std::abs(xdif) > std::abs(ydif)) ?
				(m_xPosition += xdif / std::abs(xdif)) : 
				(m_yPosition += ydif / std::abs(ydif));
		}
	}
}

int Blob::distToEdge()
{
	int xdif = m_xPosition - (m_mapSize / 2);
	int ydif = m_yPosition - (m_mapSize / 2);
	int edgeDistance;
	/*find which coord is furthest from midpoint
	Midpoint minus the modulus of (distance between coord and midpoint) 
	will give distance between coord and edge*/
	(std::abs(xdif) > std::abs(ydif)) ?
		edgeDistance = ((m_mapSize / 2) - std::abs(xdif)) :
		edgeDistance = ((m_mapSize / 2) - std::abs(ydif));

	return edgeDistance;
}

bool Blob::hasSurplusStamina()
{
	int dist{ distToEdge() };
	/*if remaining stamina is greater than that needed to get to
	closest edge then blob has spare stamina. The +1 is for case
	where blob has spare stamina then steps	one away, reducing its
	stamina by one and increasing the needed stamina by one.
	This effective +2 means without +1 error margin blob would
	make the wrong choice, and be unable to get home
	*/

	if ((m_energy / getCost()) <= dist + 1)
	{
		return false;
	}
	return true;
}

void Blob::mutate()
	{
	int prob{ 50 / g_mutationProb};
	int num{ getRandomNumber(1, prob) };
	if (num == 1) 
	{
		int mult = getRandomNumber(1, 10);
		double x{ (getSpeed()) + (mult / 10.0) };
		setSpeed(x);
	}
	else if (num == 2)
	{
		if (getSpeed() > 0)
		{
			int mult = getRandomNumber(1, 10);
			double x{ (getSpeed()) - (mult / 10.0) };
			setSpeed(x);
		}
	}

	num = getRandomNumber(1, prob);
	if (num == 1) 
	{
		int mult = getRandomNumber(1, 10);
		double x{ (getSense()) + (mult / 10.0) };
		setSense(x);
	}
	else if (num == 2)
	{
		if (getSense() > 0)
		{
			int mult = getRandomNumber(1, 10);
			double x{ (getSense()) - (mult / 10.0) };
			setSense(x);
		}
	}

	num = getRandomNumber(1, prob);
	if (num == 1) 
	{
		int mult = getRandomNumber(1, 10);
		double x{ (getSize()) + (mult / 10.0) };
		setSize(x);
	}
	else if (num == 2)
	{
		if (getSize() > 0)
		{
			int mult = getRandomNumber(1, 10);
			double x{ (getSize()) - (mult / 10.0) };
			setSize(x);
		}
	}
}

std::optional<Blob> Blob::tryToReplicate()
{
	if (getFoodEaten() > 1)
	{
		setFoodEaten(0);
		Blob child = *this;
		/*would prefer a mutated child was born rather than a
		clone made and then mutated.*/
		child.setAge(0);
		child.mutate();
		return child;
	}
	else
	{
		return std::nullopt;
	}
}