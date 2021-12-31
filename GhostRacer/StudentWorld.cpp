#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <string>
#include <sstream>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h, and Actor.cpp

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath)
{
	m_yellow = VIEW_HEIGHT / SPRITE_HEIGHT;
	m_white = VIEW_HEIGHT / (4 * SPRITE_HEIGHT);
	m_souls = 0;
	m_bonus = 5000;
}

StudentWorld::~StudentWorld() {
	cleanUp();
}

int StudentWorld::init()
{
	m_souls = 0;
	m_bonus = 5000;
	m_racer = new GhostRacer(this);
	m_actors.resize(0);

	// yellow boundaries
	// set left boundary then right boundary
	for (int y = 0; y < m_yellow; ++y) {
		addActor(new BorderLine(IID_YELLOW_BORDER_LINE, LEFT_BOUND, y * SPRITE_HEIGHT, this));
		addActor(new BorderLine(IID_YELLOW_BORDER_LINE, RIGHT_BOUND, y * SPRITE_HEIGHT, this));
	}

	// white boundaries
	// set left-middle boundary then right-middle boundary
	for (int w = 0; w < m_white; ++w) {
		addActor(new BorderLine(IID_WHITE_BORDER_LINE, LEFT_MID_BOUND, w * 4 * SPRITE_HEIGHT, this));
		addActor(new BorderLine(IID_WHITE_BORDER_LINE, RIGHT_MID_BOUND, w * 4 * SPRITE_HEIGHT, this));
	}

	m_lastWhiteY = (m_white - 1) * 4 * SPRITE_HEIGHT;

	return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
	if (!m_racer->alive()) {
		decLives();
		return GWSTATUS_PLAYER_DIED;
	}

	m_racer->doSomething();

	for (auto i = m_actors.begin(); m_racer->alive() && i != m_actors.end();) {
		// if saved enough souls, return GWSTATUS_FINISHED_LEVEL
		if (m_souls == getLevel() * 2 + 5) {
			increaseScore(m_bonus);

			return GWSTATUS_FINISHED_LEVEL;
		}

		if ((*i)->alive()) {
			(*i)->doSomething();
		}
		if (!(*i)->alive()) {
			delete *i;
			i = m_actors.erase(i);
		}
		else {
			++i;
		}
	}

	if (!m_racer->alive()) {
		decLives();
		return GWSTATUS_PLAYER_DIED;
	}

	// add new Border Lines
	double newY = VIEW_HEIGHT - SPRITE_HEIGHT;
	m_lastWhiteY += (-4 - getRacer()->getSpeedY());
	double delY = newY - m_lastWhiteY;
	if (delY >= SPRITE_HEIGHT) {
		addActor(new BorderLine(IID_YELLOW_BORDER_LINE, LEFT_BOUND, newY, this));
		addActor(new BorderLine(IID_YELLOW_BORDER_LINE, RIGHT_BOUND, newY, this));
	}
	if (delY >= 4 * SPRITE_HEIGHT) {
		addActor(new BorderLine(IID_WHITE_BORDER_LINE, LEFT_MID_BOUND, newY, this));
		addActor(new BorderLine(IID_WHITE_BORDER_LINE, RIGHT_MID_BOUND, newY, this));
		m_lastWhiteY = newY;
	}

	// add Human Pedestrian for chance [0, humChance)
	int humChance = max(200 - getLevel() * 10, 30);
	if (randInt(0, humChance - 1) == 0) {
		addActor(new Human(randInt(0, VIEW_WIDTH), VIEW_HEIGHT, this));
	}

	// add Zombie Pedestrian for chance [0, zombChance)
	int zombChance = max(100 - getLevel() * 10, 30);
	if (randInt(0, zombChance - 1) == 0) {
		addActor(new Zombie(randInt(0, VIEW_WIDTH), VIEW_HEIGHT, this));
	}

	int cabChance = max(100 - getLevel() * 10, 20);
	int lane = randInt(0, 2);
	// possibly add Zombie Cab for chance [0, cabChance)
	if (randInt(0, cabChance - 1) == 0) {
		// repeat up to 3 times (once for each lane)
		for (int l = 0; l < 3; ++l) {
			double minY = -1, maxY = -1;;
			// parse actors for collision avoidance-worthy actors in lane
			for (auto a = m_actors.begin(); a != m_actors.end(); ++a) {
				if ((*a)->collidable()) {
					if (inLane(lane, *a)) {
						if (minY == -1 || (*a)->getY() < minY) {
							minY = (*a)->getY();
						}
						if ((*a)->getY() > maxY) {
							maxY = (*a)->getY();
						}
					}
				}
			}
			// check if racer is in lane
			if (inLane(lane, m_racer)) {
				if (minY == -1 || m_racer->getY() < minY) {
					minY = m_racer->getY();
				}
				// note: m_racer can't be that far towards the top...
			}
			// collision avoidance-worthy actor does not exist for lane/not too near bottom
			if (minY == -1 || minY > VIEW_HEIGHT / 3.0) {
				if (lane == LEFT_LANE) {
					addActor(new Cab(ROAD_LEFT, SPRITE_HEIGHT / 2, m_racer->getSpeedY() + randInt(2, 4), LEFT_LANE, this));
				}
				else if (lane == MIDDLE_LANE) {
					addActor(new Cab(ROAD_CENTER, SPRITE_HEIGHT / 2, m_racer->getSpeedY() + randInt(2, 4), MIDDLE_LANE, this));
				}
				else {	// lane == RIGHT_LANE
					addActor(new Cab(ROAD_RIGHT, SPRITE_HEIGHT / 2, m_racer->getSpeedY() + randInt(2, 4), RIGHT_LANE, this));
				}
				break;
			}
			// collision avoidance-worthy actor does not exist for lane/not too near top
			if (maxY == -1 || maxY < VIEW_HEIGHT * 2 / 3.0) {
				if (lane == LEFT_LANE) {
					addActor(new Cab(ROAD_LEFT, VIEW_HEIGHT - SPRITE_HEIGHT / 2, m_racer->getSpeedY() - randInt(2, 4), LEFT_LANE, this));
				}
				else if (lane == MIDDLE_LANE) {
					addActor(new Cab(ROAD_CENTER, VIEW_HEIGHT - SPRITE_HEIGHT / 2, m_racer->getSpeedY() - randInt(2, 4), MIDDLE_LANE, this));
				}
				else {	// lane == RIGHT_LANE
					addActor(new Cab(ROAD_RIGHT, VIEW_HEIGHT - SPRITE_HEIGHT / 2, m_racer->getSpeedY() - randInt(2, 4), RIGHT_LANE, this));
				}
				break;
			}

			if (lane == RIGHT_LANE)
				lane = LEFT_LANE;
			else
				++lane;
		}
	}
	
	// add Oil Slick for chance [0, oilChance)
	int oilChance = max(150 - getLevel() * 10, 40);
	if (randInt(0, oilChance - 1) == 0) {
		addActor(new Oil(randInt(LEFT_BOUND, RIGHT_BOUND), VIEW_HEIGHT, this));
	}

	// add Holy Water Goodie for chance [0, waterChance)
	int waterChance = 100 + 10 * getLevel();
	if (randInt(0, waterChance - 1) == 0) {
		addActor(new HolyWater(randInt(LEFT_BOUND, RIGHT_BOUND), VIEW_HEIGHT, this));
	}

	// add Lost Soul Goodie for chance [0, 100)
	if (randInt(0, 99) == 0) {
		addActor(new Soul(randInt(LEFT_BOUND, RIGHT_BOUND), VIEW_HEIGHT, this));
	}

	// decrement bonus if possible
	if(m_bonus > 0)
		--m_bonus;

	// update game status string
	ostringstream status;
	status << "Score: " << getScore() << "  Lvl: " << getLevel() << "  Souls2Save: " << getLevel() * 2 + 5 - m_souls
		<< "  Lives: " << getLives() << "  Health: " << m_racer->getHP() << "  Sprays: " << m_racer->getSprays() << "  Bonus: " << m_bonus;

	setGameStatText(status.str());

	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
	for (auto i = m_actors.begin(); i != m_actors.end(); ++i) {
		delete (*i);
	}
	delete m_racer;
}

void StudentWorld::addActor(Actor* a) {
	m_actors.push_back(a);
}

void StudentWorld::savedSoul() {
	++m_souls;
}

// getters
GhostRacer* StudentWorld::getRacer() const
{
	return m_racer;
}

int StudentWorld::checkCabFrontOrBack(int lane, const Actor* a) const {
	// lane is lane of cab, a is pointer to the cab

	for (auto i = m_actors.begin(); i != m_actors.end(); ++i) {
		if ((*i) != a && (*i)->collidable()) {
			if (inLane(lane, *i) && (*i)->getY() > a->getY() && (*i)->getY() - a->getY() < 96) {
				return 0;	// actor < 96 pixels in front of cab
			}
			if (inLane(lane, *i) && a->getY() > (*i)->getY() && a->getY() - (*i)->getY() < 96) {
				return 1;	// actor > 96 pixels behind cab
			}
		}
	}

	return -1;
}

bool StudentWorld::activatedSpray(Actor* a) {
	for (auto i = m_actors.begin(); i != m_actors.end(); ++i) {
		if (overlap(*i, a) && (*i)->sprayable()) {
			(*i)->damage(1);
			a->kill();
			return true;
		}
	}

	return false;
}

// private
bool StudentWorld::inLane(int lane, const Actor* a) const {
	return ((lane == LEFT_LANE && a->getX() >= LEFT_BOUND && a->getX() < LEFT_MID_BOUND)
		|| (lane == MIDDLE_LANE && a->getX() >= LEFT_MID_BOUND && a->getX() < RIGHT_MID_BOUND)
		|| (lane == RIGHT_LANE && a->getX() >= RIGHT_MID_BOUND && a->getX() < RIGHT_BOUND));
}