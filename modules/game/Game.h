#pragma once
#include "../map/Map.h"
#include "../content/vehicles/Tank.h"
#include "../player/Player.h"
#include <vector>
#include <map>

class Player;

class Game {
public:
    int idx;
    Game() {};
    Game(int, json , json);
    void update(json);

    void updateTank(int id, int x, int y, int z, int health, int capture_points);
    void updateTank(int id, int x, int y, int z);



    //Return tanks that are under shoot from this tank
    std::vector<std::vector<std::shared_ptr<Tank> > > tanksUnderShoot(std::shared_ptr<Tank>);

    //Finds a position from which tank_1 can safely shoot tank_2
    std::vector<Hex *> findSafePositionsToShoot(std::shared_ptr<Tank>, std::shared_ptr<Tank>);

    //Finds safe positions with the minimum distance from the specified position
    std::vector<Hex *> findNearestSafePositions(Hex *);

    std::vector<std::shared_ptr<Tank>> GuaranteedKill(std::shared_ptr<Tank>);

    Map map = Map();

    std::vector<std::shared_ptr<Tank>> all_vehicles;
    std::vector<std::shared_ptr<Tank>> player_vehicles;
    std::vector<std::shared_ptr<Tank>> opponent_vehicles;

    std::map<int, Player> players;
private:
    void addTank(json, int);
    void addPlayer(json);


    void updateDanger();
    std::map<std::vector<int>, int> map_danger;
};

