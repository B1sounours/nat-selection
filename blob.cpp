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

int Blob::getName()
{
	return m_name;
}

void Blob::setName(int name)
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

int Blob::getMapSize()
{
	return m_mapSize;
}

int Blob::getFoodEaten()
{
	return m_foodEaten;
}

void Blob::setFoodEaten(int x)
{
	m_foodEaten = x;
}

void Blob::sleep()
{
	m_energy = m_nativeEnergy;
}

/*Using convention that North and East are +ve and 
South and West are -ve (cartesian)*/
void Blob::stepNorth()
{
	double y{ m_yPosition + 1 };
	setStepTarget(m_xPosition, y);
}

void Blob::stepSouth()
{
	double y{ m_yPosition - 1 };
	setStepTarget(m_xPosition, y);
}

void Blob::stepEast()
{
	double x{ m_xPosition + 1 };
	setStepTarget(x, m_yPosition);
}

void Blob::stepWest()
{
	double x{ m_xPosition - 1 };
	setStepTarget(x, m_yPosition);
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
	// if the blob isn't against the West boundary
	 if (m_xPosition > 1)
	{
		compass.push_back(&Blob::stepWest);
	}
	// if the blob isn't against the North boundary
	if (m_yPosition < m_mapSize)
	{
		compass.push_back(&Blob::stepNorth);
	}
	// if the blob isn't against the South boundary
	 if (m_yPosition > 1)
	{
		compass.push_back(&Blob::stepSouth);
	}
	//Randomly choose direction and take one step
	int num{ getRandomNumber(1, (static_cast<int>(compass.size()))) };
	(this->*compass[num - 1])();
}

void Blob::stepEastOrWest()
{
	std::vector<void(Blob::*)()> compass;

	if (m_xPosition < m_mapSize)
	{
		compass.push_back(&Blob::stepEast);
	}
	if (m_xPosition > 1)
	{
		compass.push_back(&Blob::stepWest);
	}
	int num{ getRandomNumber(1, (static_cast<int>(compass.size()))) };
	(this->*compass[num - 1])();
}

void Blob::stepNorthOrSouth()
{
	int num{ getRandomNumber(0, 1) };
	if (num)
	{
		++m_yPosition;
	}
	else
	{
		--m_yPosition;
	}
}

void Blob::stepTowards(Thing &thing)
{
	double xdif = thing.getXPosition() - m_xPosition;
	double ydif = thing.getYPosition() - m_yPosition;
	double xTarget{ m_xPosition };
	double yTarget{ m_yPosition };
	
	(std::abs(xdif) > std::abs(ydif)) ?
		(xTarget += xdif / std::abs(xdif)) : 
		(yTarget += ydif / std::abs(ydif));
	
	setStepTarget( xTarget, yTarget );
}

void Blob::stepAway(Thing &thing)
{
	double xdif = thing.getXPosition() - m_xPosition;
	double ydif = thing.getYPosition() - m_yPosition;

	double xTarget{ m_xPosition };
	double yTarget{ m_yPosition };
	/* Blobs at Home that want to run from predators, 
	can run but only within the boundaries.*/
	if (atHome())
	{
		if (m_xPosition == 0 || m_xPosition >= m_mapSize + 1)
		{
			if (m_yPosition == 0 || m_yPosition >= m_mapSize + 1)
			{
				//if in one of the corners of home area
				randomStep();
			}
			else
			{
				/*If ydiff = 0 then Thing is on same row, so step North 
				or South is equally good. Otherwise increase distance to 
				pred, staying within column*/
				(ydif == 0) ? (stepNorthOrSouth()) :
					(yTarget -= ydif / std::abs(ydif));
			}
		}
		else if (m_yPosition == 0 || m_yPosition >= m_mapSize + 1)
		{
			/*If xdiff = 0 then Thing is on same column, so step West
			or East is equally good. Otherwise increase distance to 
			pred, staying within row*/
			(xdif == 0) ? (stepEastOrWest()) :
				(xTarget -= xdif / std::abs(xdif));
		}
	}
	else
	{
		if (ydif == 0 && xdif == 0)
		{
			randomStep();
		}
		else
		{
			/*Blobs want to run from predator in general, movement not as restricted*/
			(std::abs(xdif) > std::abs(ydif)) ?
				(xTarget -= xdif / std::abs(xdif)) :
				(yTarget -= ydif / std::abs(ydif));
		}
	}
	if (xTarget != m_xPosition || yTarget != m_yPosition)
	{
		setStepTarget(xTarget, yTarget);
	}
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

void Blob::chooseHuntOrRun(std::vector<Blob> &blobArray, std::vector<Food> &foodArray)
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
	else if (!foodOpt.has_value() && !preyOpt.has_value())
	{
		searchPattern();
		return;
	}

	if (foodDist == 0) //if ontop of food
	{
		//erase that food element
		auto it = foodArray.begin();
		foodArray.erase(it + foodOpt.value());
		setFoodEaten(getFoodEaten() + 1);
		return;
	}

	if (predDist < ((foodDist < preyDist) ? foodDist : preyDist))
	{
		stepAway(blobArray[predOpt.value()]);
		return;
	}

	if (predDist > ((foodDist < preyDist) ? foodDist : preyDist))
	{
		(foodDist < preyDist) ?
			stepTowards(foodArray[foodOpt.value()]) :
			stepTowards(blobArray[preyOpt.value()]);
		return;
	}

	if (predDist == ((foodDist < preyDist) ? foodDist : preyDist))
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
		return;
	}
	std::cout << "\nShould be impossible to reach here\n";
	return;
}

std::optional<int> Blob::huntOrRun(std::vector<Blob> &blobArray, std::vector<Food> &foodArray)
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
	//if (preyOpt.has_value())
	//{
	//	int preyElement{ preyOpt.value() };
	//	preyDist = distToObject(blobArray[preyElement]);
	//}
	if (predOpt.has_value())
	{
		int predElement{ predOpt.value() };
		predDist = distToObject(blobArray[predElement]);
	}
	// if none of the above have value take random step
	else if (!foodOpt.has_value() && !preyOpt.has_value())
	{
		searchPattern();
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
	
	if (preyDist == 0) //if ontop of prey
	{
		auto it = blobArray.begin();
		//erase that blob element
		blobArray.erase(it + preyOpt.value());
		setFoodEaten(getFoodEaten() + 1);
		return preyOpt.value();
	}

	if (predDist < ((foodDist < preyDist) ? foodDist : preyDist))
	{
		stepAway(blobArray[predOpt.value()]);
		return std::nullopt;
	}

	if (predDist > ((foodDist < preyDist) ? foodDist : preyDist))
	{
		(foodDist < preyDist) ?
			stepTowards(foodArray[foodOpt.value()]) :
			stepTowards(blobArray[preyOpt.value()]);
		return std::nullopt;
	}

	if (predDist == ((foodDist < preyDist) ? foodDist : preyDist))
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
	std::cout << "\nShould be impossible to reach here\n";
	return std::nullopt;
}

double Blob::distToObject(Thing &object)
{
	double xdif = object.getXPosition() - m_xPosition;
	double ydif = object.getYPosition() - m_yPosition;
	double distAway{ std::abs(xdif) + std::abs(ydif) };
	return distAway;
}

/* Lots of repetition here. Bound to be a smarter way.*/
std::optional<int> Blob::lookForFood(std::vector<Food> &foodArray)
{
	double dist = 10 * m_mapSize;
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
		to stay int because the tile spaces are in integers.*/
		double xPos{ foodArray[i].getXPosition() };
		double yPos{ foodArray[i].getYPosition() };
		if ((m_xPosition - m_sense) <= xPos && xPos <= (m_xPosition + m_sense))
		{
			if ((m_yPosition - m_sense) <= yPos && yPos <= (m_yPosition + m_sense))
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
	double dist = 10 * m_mapSize;
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
			double xPos{ blobArray[i].getXPosition() };
			double yPos{ blobArray[i].getYPosition() };
			if ((m_xPosition - m_sense) <= xPos && xPos <= (m_xPosition + m_sense))
			{
				if ((m_yPosition - m_sense) <= yPos && yPos <= (m_yPosition + m_sense))
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
	double dist = 10 * m_mapSize;
	int element;
	bool foundPred = false;

	//for every piece of food
	int length{ static_cast<int>(blobArray.size()) };
	for (int i = 0; i < length; ++i)
	{
		if ((0.8*blobArray[i].getSize()) >= m_size && !blobArray[i].atHome())
		{
			/*search the sense area around the blobs position to see if that food
			is within it. With sense as a double, the rounding down of int x means
			there is increased energetic cost (cost is a double) for every increase in sense
			but only an evolutionary advantage when sense increases by a whole unit. int x has
			to stay int because the tile spaces are in integers.*/
			double xPos{ blobArray[i].getXPosition() };
			double yPos{ blobArray[i].getYPosition() };
			if ((m_xPosition - m_sense) <= xPos && xPos <= (m_xPosition + m_sense))
			{
				if ((m_yPosition - m_sense) <= yPos && yPos <= (m_yPosition + m_sense))
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
	if (m_xPosition == 0 || m_xPosition >= m_mapSize + 1)
	{
		return true;
	}
	if (m_yPosition == 0 || m_yPosition >= m_mapSize + 1)
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
		/*find which coord is furthest from the midpoint
		and thus closest to edge, and step in that direction*/
		double xdif = m_xPosition - (m_mapSize / 2);
		double ydif = m_yPosition - (m_mapSize / 2);
		double xTarget{ m_xPosition };
		double yTarget{ m_yPosition };
		if (xdif == ydif) //centre diagonal
		{
			(yTarget >= m_mapSize / 2) ?
				(++yTarget) : (--yTarget);
		}
		else
		{
			(std::abs(xdif) > std::abs(ydif)) ?
				(xTarget += xdif / std::abs(xdif)) : 
				(yTarget += ydif / std::abs(ydif));
		}
		setStepTarget(xTarget, yTarget);
	}
}

void Blob::searchPattern()
{
	if (atHome())
	{
		randomStep();
		return;
	}
	int x = m_xPosition;
	int y = m_yPosition;
	if (x % 2 == 0)
	{
		if (m_yPosition <= 1 + static_cast<int>(m_sense))
		{
			stepEastOrWest();
		} 
		else
		{
			if (m_name % 2 == 0)
			{
				stepSouth();
			}
			else
			{
				stepNorth();
			}
		}
	}
	else
	{
		if (m_yPosition >= m_mapSize - static_cast<int>(m_sense))
		{
			stepEastOrWest();
		}
		else
		{
			if (m_name % 2 == 0)
			{
				stepNorth();
			}
			else
			{
				stepSouth();
			}
		}
	}
}

double Blob::distToEdge()
{
	double mid( (m_mapSize + 1.0) / 2.0 );
	double xSteps, ySteps;
	double dist;
	xSteps = mid - std::abs(m_xPosition - mid);
	ySteps = mid - std::abs(m_yPosition - mid);
	(xSteps >= ySteps) ? dist = ySteps : dist = xSteps;
	return dist;
}

bool Blob::hasSurplusStamina()
{
	if (atHome())
	{
		//stop blob movement after returning home with 1 food
		return false; 
	}

	double dist{ distToEdge() };
	/*if remaining stamina is greater than that needed to get to
	closest edge then blob has spare stamina. The +1 is for case
	where blob has spare stamina then steps	one away, reducing its
	stamina by one and increasing the needed stamina by one.
	This effective +2 means without +1 error margin blob would
	make the wrong choice, and be unable to get home
	*/
	if ((m_energy / getCost()) <= dist + 1.0)
	{
		return false;
	}
	return true;
}

void Blob::mutate()
{
	int prob{ 200/ g_mutationProb};
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
		child.mutate();
		return child;
	}
	else
	{
		return std::nullopt;
	}
}

void Blob::setStepTarget(int x, int y)
{
	m_stepTarget = { x, y };
}

std::array<int, 2>& Blob::getStepTarget()
{
	return m_stepTarget;
}

bool Blob::finishedStep()
{
	double x = static_cast<double>(m_stepTarget[0]);
	double y = static_cast<double>(m_stepTarget[1]);
	if (m_xPosition - 0.0001 <= x && x <= m_xPosition + 0.0001)
	{
		if (m_yPosition - 0.0001 <= y && y <= m_yPosition + 0.0001)
		{
			setPosition(m_stepTarget[0], m_stepTarget[1]);
			return true;
		}
	}
	else
	{
		return false;
	}
}

void Blob::continueStep()
{
	double xdiff = m_stepTarget[0]- m_xPosition;
	double ydiff = m_stepTarget[1] - m_yPosition;
	int speedMax = 20;
	int increments{ static_cast<int>(-m_speed + speedMax + 1.0) };
	double distPerTime = ( 1.0 / static_cast<int>(increments) );
	if (xdiff != 0)
	{
		(xdiff > 0) ? (m_xPosition += distPerTime) :
			(m_xPosition -= distPerTime);
	}
	if (ydiff != 0)
	{
		(ydiff > 0) ? (m_yPosition = (m_yPosition + distPerTime)) :
			(m_yPosition -= distPerTime);
	}

	if (finishedStep())
	{
		reduceEnergy();
	}
}