#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

// constants
const int LEFT_LANE = 0;
const int MIDDLE_LANE = 1;
const int RIGHT_LANE = 2;

class StudentWorld;

// Actor base class, derived from GraphObject
class Actor : public GraphObject {
public:
	Actor(int imageID, double startX, double startY, int dir, double size, unsigned int depth, 
		double speedX, double speedY, int hp, bool alive, bool collidable, StudentWorld* world);
	virtual ~Actor();

	// by default does the movement algorithm that all but GhostRacer and Spray use
	virtual void doSomething();
	
	// public setters
	// damages current actor, and plays appropriate hurt/death sounds if necessary
	// also functions as the reaction to sprays in derived classes
	virtual void damage(int dmg);

	// sets alive to false
	void kill();
	
	// public getters
	double getSpeedX() const;
	double getSpeedY() const;
	int getHP() const;
	bool alive() const;
	bool collidable() const;

	// sprayable is false by default
	virtual bool sprayable() const;

protected:
	void setSpeedX(double speed);
	void setSpeedY(double speed);
	StudentWorld* getWorld() const;

	// check if actor is in bounds, and kill() if it is not
	void checkInBounds();

private:
	// both default to -1, indicating no sound to be played
	// otherwise returns the appropriate sound to be played when hurt or killed via damage()
	virtual int hurtSound() const;
	virtual int dieSound() const;

	double m_speedX;
	double m_speedY;
	int m_hp;
	bool m_alive;
	bool m_collidable;

	StudentWorld* m_world;
};

// GhostRacer, derived from Actor
class GhostRacer : public Actor {
public:
	GhostRacer(StudentWorld* world);
	virtual ~GhostRacer();

	// checks if it hits the borders, else if you hit a key, else move accordingly
	virtual void doSomething();

	int getSprays();
	void setSprays(int sprays);

private:
	virtual int dieSound() const;

	int m_sprays;
};

// BorderLine, derived from Actor
class BorderLine : public Actor {
public:
	BorderLine(int imageID, double startX, double startY, StudentWorld* world);
	virtual ~BorderLine();
};

// Agent, derived from Actor, base for Pedestrians/Zombie Cabs
class Agent : public Actor {
public:
	Agent(int imageID, double startX, double startY, int dir, double size, unsigned int depth, 
		double speedY, int hp, StudentWorld* world);
	virtual ~Agent();

	// runs the commonality between all Agents, namely decrementing plan length if necessary
	virtual void doSomething();

	// all Agents are sprayable, so this returns true
	virtual bool sprayable() const;

protected:
	// setters
	// sets plan length to random integer between 4 and 32, inclusive
	void resetPlanLength();

	// getters
	int getPlanLength() const;

private:
	int m_plan;
};

// Pedestrian, derived from Agent, base for Human/Zombie
class Pedestrian : public Agent {
public:
	Pedestrian(int imageID, double startX, double startY, double size, StudentWorld* world);
	virtual ~Pedestrian();

	// calls Actor's doSomething() for movement, then Agent's doSomething() to decrement plan
	// then runs common function between Human and Zombie, namely resetting plan length and changing
	// horizontal speed and direction accordingly
	virtual void doSomething();

private:
	virtual int hurtSound() const;
	virtual int dieSound() const;
};

// Human, derived from Pedestrian
class Human : public Pedestrian {
public:
	Human(double startX, double startY, StudentWorld* world);
	virtual ~Human();

	// kill racer if overlap
	// then calls Pedestrian's doSomething(), for the common functionality between Humans and Zombies
	virtual void doSomething();

	// damage is used ONLY by spray, as spray functionality works in the same way
	// Human CANNOT be damaged, and thus calls Actor's damage with a damage of 0 every time
	// and then does its spray reaction, namely changing direction and horizontal speed
	virtual void damage(int dmg);
};

// Zombie, derived from Pedestrian
class Zombie : public Pedestrian {
public:
	Zombie(double startX, double startY, StudentWorld* world);
	virtual ~Zombie();

	// does appropriate damage if overlaps with racer, otherwise sets the proper movement
	// and plays SOUND_ZOMBIE_ATTACK if honing in on racer
	// then calls Pedestrian's doSomething(), for the common functionality between Humans and Zombies
	virtual void doSomething();

	// damage is used by spray as spray functionality works in the same way, AS WELL as when Racer hits Zombie,
	// calls Actor's damage for the damage dealt by a spray or collision, and then has a 1 in 5 chance to drop
	// a healing goodie if killed by a spray (not overlapping with GhostRacer)
	// then increases score by 150
	virtual void damage(int dmg);

private:
	int m_gruntTicks;

	// racer member variable kept here and taken from StudentWorld in order to make
	// calls to racer in both damage() and doSomething() less verbose
	// m_racer is NOT destructed in Zombie, as it will be destructed in StudentWorld instead
	GhostRacer* m_racer;
};

// Zombie Cab, derived from Agent
class Cab : public Agent {
public:
	Cab(double startX, double startY, double speedY, int lane, StudentWorld* world);
	virtual ~Cab();

	// damages GhostRacer if overlapping and plays sound accordingly, then no longer damages racer and flies off screen accordingly
	// if not overlapping, it calls Actor's doSomething() to move accordingly
	// then checks for collision avoidance-worthy actors in front or behind by 96 pixels in its lane and changes vertical speed accordingly
	// otherwise call Agent's doSomething to decrement plan distance and resets accordingly if necessary
	virtual void doSomething();

	// damage is used ONLY by spray, as spray functionality works in the same way
	// gets damaged by calling Actor's damage, then if killed by spray has 1 in 5 chance of dropping Oil Slick
	// then increases score by 200
	virtual void damage(int dmg);

private:
	virtual int hurtSound() const;
	virtual int dieSound() const;

	bool m_damagedRacer;
	int m_lane;
};

// Goodie, derived from Actor, base for Oil Slick/Goodies
class Goodie : public Actor {
public:
	Goodie(int imageID, double startX, double startY, int dir, double size, StudentWorld* world);
	virtual ~Goodie();

	// calls Actor's doSomething to move accordingly
	// then runs common functionality of all Goodies, by playing the appropriate sound if overlapping with racer,
	// doing the Goodie's common activity, destroying the Goodie, and increasing score accordingly
	virtual void doSomething();
	
	// goodies except Oil Slick and Lost Souls are sprayable by default, so this is set to true
	virtual bool sprayable() const;

private:
	// runs each Goodie's unique activity
	virtual void doActivity() = 0;

	// returns each Goodie's score increase
	virtual int getScoreIncrease() const = 0;

	// returns each Goodie's collision sound
	virtual int getSound() const;

	// all goodies except Oil Slick are destructible by default, so this is set to true
	virtual bool destructible() const;
};

// Oil Slick, derived from Goodie
class Oil : public Goodie {
public:
	Oil(double startX, double startY, StudentWorld* world);
	virtual ~Oil();

	// returns false to override Goodie
	virtual bool sprayable() const;

private:
	// spins racer accordingly
	virtual void doActivity();

	// Oil Slick doesn't increase score, so this returns 0
	virtual int getScoreIncrease() const;
	virtual int getSound() const;

	// returns false to override Goodie
	virtual bool destructible() const;
};

// Healing Goodie, derived from Goodie
class Heal : public Goodie {
public:
	Heal(double startX, double startY, StudentWorld* world);
	virtual ~Heal();

private:
	// heals racer by damaging it by -10 hp, equivalent to a heal of 10 hp
	virtual void doActivity();

	// returns 250
	virtual int getScoreIncrease() const;
};

// Holy Water Goodie, derived from Goodie
class HolyWater : public Goodie {
public:
	HolyWater(double startX, double startY, StudentWorld* world);
	virtual ~HolyWater();

private:
	// increases racer's sprays by 10
	virtual void doActivity();

	// returns 50
	virtual int getScoreIncrease() const;
};

// Lost Soul Goodie, derived from Goodie
class Soul : public Goodie {
public:
	Soul(double startX, double startY, StudentWorld* world);
	virtual ~Soul();

	// calls Goodie's doSomething() for common functionality, and also rotates clockwise by 10 as specified
	virtual void doSomething();

	// returns false to override Goodie
	virtual bool sprayable() const;

private:
	// increases souls saved in StudentWorld
	virtual void doActivity();
	virtual int getSound() const;

	// returns 100
	virtual int getScoreIncrease() const;
};

// Holy Water Projectile, derived from Actor
class Spray : public Actor {
public:
	Spray(double startX, double startY, int dir, StudentWorld* world);
	virtual ~Spray();

	// if activated, do nothing
	// if not activated, move accordingly, check if in bounds, and dissipate if moved its full distance
	virtual void doSomething();

private:
	int m_travelDist;
};

// Non-member functions
bool overlap(const Actor* a, const Actor* b);

#endif // ACTOR_H_
