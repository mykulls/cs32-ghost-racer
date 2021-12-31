#include "Actor.h"
#include "StudentWorld.h"
#include "GameConstants.h"
#include <cmath>

// Students:  Add code to this file, Actor.h, StudentWorld.h, and StudentWorld.cpp

// Non-member functions
bool overlap(const Actor* a, const Actor* b)
{
	double delX = abs(a->getX() - b->getX());
	double delY = abs(a->getY() - b->getY());
	double radSum = a->getRadius() + b->getRadius();

	if (delX < radSum * 0.25 && delY < radSum * 0.6)
		return true;

	return false;
}


// Actor definitions
Actor::Actor(int imageID, double startX, double startY, int dir, double size, unsigned int depth, 
	double speedX, double speedY, int hp, bool alive, bool collidable, StudentWorld* world)
	: GraphObject(imageID, startX, startY, dir, size, depth), m_speedX(speedX), m_speedY(speedY), 
	m_hp(hp), m_alive(alive), m_collidable(collidable), m_world(world) {}

Actor::~Actor() {}

void Actor::doSomething() {
	double newX, newY;
	newY = getY() + m_speedY - getWorld()->getRacer()->getSpeedY();
	newX = getX() + m_speedX;
	GraphObject::moveTo(newX, newY);

	checkInBounds();
}

// setters
void Actor::damage(int dmg) {
	m_hp -= dmg;

	if (m_hp <= 0) {
		kill();
		if (dieSound() != -1) {
			getWorld()->playSound(dieSound());
		}
	}
	else {
		if (hurtSound() != -1) {
			getWorld()->playSound(hurtSound());
		}
	}
}

void Actor::kill() {
	m_alive = false;
}

void Actor::setSpeedX(double speed) {
	m_speedX = speed;
}

void Actor::setSpeedY(double speed) {
	m_speedY = speed;
}

void Actor::checkInBounds() {
	if (getX() < 0 || getY() < 0 || getX() > VIEW_WIDTH || getY() > VIEW_HEIGHT)
		kill();
}

// getters
double Actor::getSpeedX() const {
	return m_speedX;
}

double Actor::getSpeedY() const {
	return m_speedY;
}

int Actor::getHP() const {
	return m_hp;
}

bool Actor::alive() const {
	return m_alive;
}

bool Actor::collidable() const {
	return m_collidable;
}

StudentWorld* Actor::getWorld() const {
	return m_world;
}

int Actor::hurtSound() const {
	return -1;
}

int Actor::dieSound() const {
	return -1;
}

bool Actor::sprayable() const {
	return false;
}


// GhostRacer definitions
GhostRacer::GhostRacer(StudentWorld* world)
	: Actor(IID_GHOST_RACER, 128, 32, 90, 4.0, 0, 0, 0, 100, true, true, world), m_sprays(10) {}

GhostRacer::~GhostRacer() {}

void GhostRacer::doSomething() {
	int ch;
	if (getX() <= LEFT_BOUND) {			// left
		if (getDirection() > 90) {
			damage(10);
		}

		setDirection(82);
		getWorld()->playSound(SOUND_VEHICLE_CRASH);
	}
	else if (getX() >= RIGHT_BOUND) {	// right
		if (getDirection() < 90) {
			damage(10);
		}

		setDirection(98);
		getWorld()->playSound(SOUND_VEHICLE_CRASH);
	}
	else if (getWorld()->getKey(ch))
	{
		// user hit a key during this tick!
		switch (ch)
		{
		case KEY_PRESS_LEFT:
			if (getDirection() < 114)
				setDirection(getDirection() + 8);
			break;
		case KEY_PRESS_RIGHT:
			if (getDirection() > 66)
				setDirection(getDirection() - 8);
			break;
		case KEY_PRESS_UP:
			if (getSpeedY() < 5)
				setSpeedY(getSpeedY() + 1);
			break;
		case KEY_PRESS_DOWN:
			if (getSpeedY() > -1)
				setSpeedY(getSpeedY() - 1);
			break;
		case KEY_PRESS_SPACE:
			if (m_sprays > 0) {
				double currDir = getDirection() * 1.0 / 180 * 4 * atan(1.0);
				getWorld()->addActor(new Spray(getX() + SPRITE_HEIGHT * cos(currDir), 
					getY() + SPRITE_HEIGHT * sin(currDir), getDirection(), getWorld()));
				getWorld()->playSound(SOUND_PLAYER_SPRAY);
				--m_sprays;
			}
			break;
		}
	}

	double maxShift = 4.0;
	double dir = getDirection() * 1.0 / 180 * 4 * atan(1.0);
	double delX = cos(dir) * maxShift;
	moveTo(getX() + delX, getY());
}

int GhostRacer::dieSound() const {
	return SOUND_PLAYER_DIE;
}

int GhostRacer::getSprays()
{
	return m_sprays;
}

void GhostRacer::setSprays(int sprays) {
	m_sprays = sprays;
}


// BorderLine definitions
BorderLine::BorderLine(int imageID, double startX, double startY, StudentWorld* world)
	: Actor(imageID, startX, startY, 0, 2.0, 2, 0, -4, 0, true, false, world) {}

BorderLine::~BorderLine() {}


// Agent definitions
Agent::Agent(int imageID, double startX, double startY, int dir, double size, unsigned int depth, 
	double speedY, int hp, StudentWorld* world)
	: Actor(imageID, startX, startY, dir, size, depth, 0, speedY, hp, true, true, world), m_plan(0) {}

Agent::~Agent() {}

void Agent::doSomething() {
	if (m_plan > 0) {
		--m_plan;
	}
}

// setters
void Agent::resetPlanLength() {
	m_plan = randInt(4, 32);
}

// getters
int Agent::getPlanLength() const {
	return m_plan;
}

bool Agent::sprayable() const {
	return true;
}


// Pedestrian definitions
Pedestrian::Pedestrian(int imageID, double startX, double startY, double size, StudentWorld* world)
	: Agent(imageID, startX, startY, 0, size, 0, -4, 2, world) {}

Pedestrian::~Pedestrian() {}

void Pedestrian::doSomething() {
	Actor::doSomething();
	Agent::doSomething();

	if (getPlanLength() == 0) {
		resetPlanLength();

		// reset horiz. speed between 3, -3 inclusive, 0 exclusive
		do
			setSpeedX(randInt(-3, 3));
		while (getSpeedX() == 0);

		if (getSpeedX() < 0)
			setDirection(180);
		else
			setDirection(0);
	}
}

int Pedestrian::hurtSound() const {
	return SOUND_PED_HURT;
}

int Pedestrian::dieSound() const {
	return SOUND_PED_DIE;
}


// Human definitions
Human::Human(double startX, double startY, StudentWorld* world)
	: Pedestrian(IID_HUMAN_PED, startX, startY, 2.0, world) {}

Human::~Human() {}

void Human::doSomething() {
	if (overlap(this, getWorld()->getRacer())) {
		getWorld()->getRacer()->kill();
		return;
	}

	Pedestrian::doSomething();
}

void Human::damage(int dmg) {
	Actor::damage(0);	// human can't be damaged/killed

	setSpeedX(getSpeedX() * -1);
	setDirection(getDirection() - 180); // don't have to worry about negative, auto set in GraphObject.h
}


// Zombie definitions
Zombie::Zombie(double startX, double startY, StudentWorld* world)
	: Pedestrian(IID_ZOMBIE_PED, startX, startY, 3.0, world), m_gruntTicks(0), m_racer(world->getRacer()) {}

Zombie::~Zombie() {} // don't need to delete m_racer since StudentWorld does that

void Zombie::doSomething() {
	if (overlap(this, m_racer)) {
		m_racer->damage(5);
		damage(2);
	}

	if (abs(getX() - m_racer->getX()) <= 30 && getY() > m_racer->getY()) {
		setDirection(270);
		if (getX() < m_racer->getX())		// to the left of racer
			setSpeedX(1);
		else if (getX() > m_racer->getX())	// to the right of racer
			setSpeedX(-1);
		else								// same x coordinate
			setSpeedX(0);

		--m_gruntTicks;
		if (m_gruntTicks <= 0) {
			getWorld()->playSound(SOUND_ZOMBIE_ATTACK);
			m_gruntTicks = 20;
		}
	}

	Pedestrian::doSomething();
}

void Zombie::damage(int dmg) {
	Actor::damage(dmg);

	if (getHP() <= 0) {
		if (!overlap(this, m_racer)) {
			if (randInt(1, 5) == 1) {
				getWorld()->addActor(new Heal(getX(), getY(), getWorld()));
			}
		}

		getWorld()->increaseScore(150);
	}
}

// Zombie Cab definitions
Cab::Cab(double startX, double startY, double speedY, int lane, StudentWorld* world)
	: Agent(IID_ZOMBIE_CAB, startX, startY, 90, 4.0, 0, speedY, 3, world), m_damagedRacer(false), m_lane(lane) {}

Cab::~Cab() {}

void Cab::doSomething() {
	GhostRacer* racer = getWorld()->getRacer();
	if (overlap(this, racer) && !m_damagedRacer) {
		getWorld()->playSound(SOUND_VEHICLE_CRASH);
		racer->damage(20);

		if (getX() - racer->getX() <= 0) {
			setSpeedX(-5);
			setDirection(120 + randInt(0, 19));
		}
		else {
			setSpeedX(5);
			setDirection(60 - randInt(0, 19));
		}

		m_damagedRacer = true;
	}

	Actor::doSomething();

	// check actors if in front or behind cab
	if (getSpeedY() > racer->getSpeedY() && getWorld()->checkCabFrontOrBack(m_lane, this) == 0) {
		setSpeedY(getSpeedY() - 0.5);
		return;
	}
	if (getSpeedY() <= racer->getSpeedY() && getWorld()->checkCabFrontOrBack(m_lane, this) == 1) {
		setSpeedY(getSpeedY() + 0.5);
		return;
	}

	// decrement plan distance
	Agent::doSomething();

	if (getPlanLength() == 0) {
		resetPlanLength();
		// set vert speed to its vert speed + random int [-2, 2]
		setSpeedY(getSpeedY() + randInt(-2, 2));
	}
}

void Cab::damage(int dmg) {
	Actor::damage(dmg);

	if (getHP() <= 0) {
		if (randInt(1, 5) == 1) {
			getWorld()->addActor(new Oil(getX(), getY(), getWorld()));
		}

		getWorld()->increaseScore(200);
	}
}

int Cab::hurtSound() const {
	return SOUND_VEHICLE_HURT;
}

int Cab::dieSound() const {
	return SOUND_VEHICLE_DIE;
}


// Goodie definitions
Goodie::Goodie(int imageID, double startX, double startY, int dir, double size, StudentWorld* world)
	: Actor(imageID, startX, startY, dir, size, 2, 0, -4, 0, true, false, world) {}

Goodie::~Goodie() {}

void Goodie::doSomething() {
	Actor::doSomething();

	if (overlap(this, getWorld()->getRacer())) {
		getWorld()->playSound(getSound());
		doActivity();
		if (destructible()) {
			kill();
		}
		getWorld()->increaseScore(getScoreIncrease());
	}
}

int Goodie::getSound() const {
	return SOUND_GOT_GOODIE;
}

bool Goodie::destructible() const {
	return true;
}

bool Goodie::sprayable() const {
	return true;
}


// Oil Slick definitions
Oil::Oil(double startX, double startY, StudentWorld* world)
	: Goodie(IID_OIL_SLICK, startX, startY, 0, randInt(2, 5), world) {}

Oil::~Oil() {}

void Oil::doActivity() {
	// spin Ghost Racer
	GhostRacer* racer = getWorld()->getRacer();
	int dir = racer->getDirection();
	int cw = randInt(0, 1);
	if (dir <= 100 && dir >= 80) {
		if (cw == 0) {
			// counter clockwise
			racer->setDirection(dir + randInt(5, 20));
		}
		else {
			// clockwise
			racer->setDirection(dir - randInt(5, 20));
		}
	}
	else if (dir <= 100) {
		racer->setDirection(dir + randInt(5, 20));
	}
	else {
		racer->setDirection(dir - randInt(5, 20));
	}
}

int Oil::getScoreIncrease() const {
	return 0;
}

int Oil::getSound() const {
	return SOUND_OIL_SLICK;
}

bool Oil::destructible() const {
	return false;
}

bool Oil::sprayable() const {
	return false;
}


// Healing Goodie definitions
Heal::Heal(double startX, double startY, StudentWorld* world) 
	: Goodie(IID_HEAL_GOODIE, startX, startY, 0, 1.0, world) {}

Heal::~Heal() {}

void Heal::doActivity() {
	if (getWorld()->getRacer()->getHP() < 90)
		getWorld()->getRacer()->damage(-10); // damage by -10 = heal by 10
	else
		getWorld()->getRacer()->damage(getWorld()->getRacer()->getHP() - 100);
		// damage by HP - 100 so that it always ends up at 100 when HP >= 90
}

int Heal::getScoreIncrease() const {
	return 250;
}


// Holy Water Goodie definitions
HolyWater::HolyWater(double startX, double startY, StudentWorld* world)
	: Goodie(IID_HOLY_WATER_GOODIE, startX, startY, 90, 2.0, world) {}

HolyWater::~HolyWater() {}

void HolyWater::doActivity() {
	getWorld()->getRacer()->setSprays(getWorld()->getRacer()->getSprays() + 10);
}

int HolyWater::getScoreIncrease() const {
	return 50;
}


// Lost Soul Goodie definitions
Soul::Soul(double startX, double startY, StudentWorld* world)
	: Goodie(IID_SOUL_GOODIE, startX, startY, 0, 4.0, world) {}

Soul::~Soul() {}

void Soul::doSomething() {
	Goodie::doSomething();

	setDirection(getDirection() - 10);
}

void Soul::doActivity() {
	getWorld()->savedSoul();
}

int Soul::getSound() const {
	return SOUND_GOT_SOUL;
}

int Soul::getScoreIncrease() const {
	return 100;
}

bool Soul::sprayable() const {
	return false;
}


// Holy Water Projectile definitions
Spray::Spray(double startX, double startY, int dir, StudentWorld* world)
	: Actor(IID_HOLY_WATER_PROJECTILE, startX, startY, dir, 1.0, 1, 0, 0, 0, true, false, world), m_travelDist(160) {}

Spray::~Spray() {}

void Spray::doSomething() {
	// check if activated
	if (!getWorld()->activatedSpray(this)) {
		moveForward(SPRITE_HEIGHT);
		m_travelDist -= SPRITE_HEIGHT;
		checkInBounds();

		// dissipate if moved a total of 160 pixels
		if (m_travelDist <= 0)
			kill();
	}
}
