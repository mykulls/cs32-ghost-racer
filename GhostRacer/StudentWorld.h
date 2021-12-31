#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"

#include <string>
#include <list>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

// constants
const int LEFT_BOUND = ROAD_CENTER - ROAD_WIDTH / 2;       // pos of left boundary
const int RIGHT_BOUND = ROAD_CENTER + ROAD_WIDTH / 2;      // pos of right boundary
const int LEFT_MID_BOUND = LEFT_BOUND + ROAD_WIDTH / 3;    // pos of left middle boundary
const int RIGHT_MID_BOUND = RIGHT_BOUND - ROAD_WIDTH / 3;  // pos of right middle boundary

const int ROAD_LEFT = ROAD_CENTER - ROAD_WIDTH / 3.0;
const int ROAD_RIGHT = ROAD_CENTER + ROAD_WIDTH / 3.0;

class Actor;
class GhostRacer;

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetPath);
    virtual ~StudentWorld();
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    void addActor(Actor* a);
    void savedSoul();

    // getters
    GhostRacer* getRacer() const;

    // returns -1 if neither, 0 if collidable actor in front of cab within 96 pixels, 1 if behind cab within 96 pixels
    int checkCabFrontOrBack(int lane, const Actor* a) const;

    // returns true if spray is activated, attempts to damage other actor by 1, and kills spray
    bool activatedSpray(Actor* a);

private:
    bool inLane(int lane, const Actor* a) const;

    GhostRacer* m_racer;
    std::list<Actor*> m_actors;

    int m_yellow;   // N, number of yellow borders
    int m_white;    // M, number of white borders

    double m_lastWhiteY;

    int m_souls;
    int m_bonus;
};

#endif // STUDENTWORLD_H_
