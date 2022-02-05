#include "Map.h"


Map::Map(json map_json, int radius) : radius_(radius) {
    hexes = generateEmptyMap(radius);
    setBase(map_json);
    setSpawnPoints(map_json);
}

void Map::setBase(json map_json) {
    for (auto hex: map_json["content"]["base"])
        base.push_back(
                getHex(
                        Hex(
                                hex["x"].get<std::int32_t>(),
                                hex["y"].get<std::int32_t>(),
                                hex["z"].get<std::int32_t>()
                        )
                )
        );

    for (auto hex: base) {
        hex->setHex(ContentType::BASE);
    }
}

void Map::setSpawnPoints(json map_json) {
    json spawn_points = map_json["spawn_points"];
    for (auto it = spawn_points.begin(); it != spawn_points.end(); ++it) {
        json player = it.value();
        for (auto it = player.begin(); it != player.end(); ++it) {
            json vehicle = it.value();
            for (auto pos: vehicle) {
                Hex hex = Hex(pos["x"].get<std::int32_t>(),
                              pos["y"].get<std::int32_t>(),
                              pos["z"].get<std::int32_t>());
                getHex(hex)->setHex(ContentType::SPAWN_POINT);
            }
        }
    }
}


void Map::changeOccupied(Hex hex, bool is_occupied) {
    getHex(hex)->is_occupied = is_occupied;
}

void Map::clearPath() {
    for (auto hex: hexes) {
        hex->visited = false;
        hex->prev = nullptr;
    }
}

std::vector<std::shared_ptr<Hex>> Map::generateEmptyMap(int radius) {
    std::vector<Hex> deltas = {Hex(1, 0, -1), Hex(0, 1, -1), Hex(-1, 1, 0), Hex(-1, 0, 1), Hex(0, -1, 1),
                               Hex(1, -1, 0)};
    hexes_map.clear();
    hexes.clear();
    std::vector<int> v;
    for (int r = 0; r < radius + 1; r++) {
        int x = 0;
        int y = -1 * r;
        int z = r;
        v = {x, y, z};
        std::shared_ptr<Hex> hex = std::make_shared<Hex>(x, y, z);
        hexes_map.insert(std::pair<std::vector<int>, std::shared_ptr<Hex>>(v, hex));
        hexes.push_back(hex);

        for (int j = 0; j < 6; j++) {
            int num_of_hexes_in_edge = 0;
            if (j == 5)
                num_of_hexes_in_edge = r - 1;
            else
                num_of_hexes_in_edge = r;

            for (int i = 0; i < num_of_hexes_in_edge; i++) {
                x = x + deltas[j].x;
                y = y + deltas[j].y;
                z = z + deltas[j].z;
                v = {x, y, z};
                hex = std::make_shared<Hex>(x, y, z);
                hexes_map.insert(std::pair<std::vector<int>, std::shared_ptr<Hex>>(v, hex));
                hexes.push_back(hex);
            }
        }
    }
    for (auto itr = hexes_map.begin(); itr != hexes_map.end(); ++itr) {
        std::shared_ptr<Hex> hex = itr->second;
        for (int i = 0; i < 6; i++) {
            Hex neighbor_temp = hex->getNeighbor(i);
            if (neighbor_temp.getLength() <= 10) {
                v = {neighbor_temp.x, neighbor_temp.y, neighbor_temp.z};
                std::shared_ptr<Hex> neighbor = hexes_map.find(v)->second;
                hex->addNeighbour(neighbor);
            }
        }
    }

    return hexes;
}

bool Map::belongs(const Hex &h) const {
    return h.getLength() <= radius_;
}

std::shared_ptr<Hex> Map::getHex(const Hex &hex) const {
    std::vector<int> v1 = {hex.x, hex.y, hex.z};
    if (this->hexes_map.find(v1) == this->hexes_map.end()) {
        return nullptr;
    } else {
        return this->hexes_map.find(v1)->second;
    }
}

std::vector<std::shared_ptr<Hex>> Map::findPath(Hex start, std::vector<Hex> ends, std::shared_ptr<Tank> tank) {
    // If path does not exist, will be returned list only with the "END" Hex
    std::vector<std::shared_ptr<Hex>> ends_vector;
    for (auto hex: ends)
        ends_vector.push_back(this->getHex(hex));
    return this->findPath(this->getHex(start), ends_vector, tank);
}

std::vector<std::shared_ptr<Hex>>
Map::findPath(std::shared_ptr<Hex> start, std::vector<std::shared_ptr<Hex>> ends, std::shared_ptr<Tank> tank) {
    std::shared_ptr<Hex> end = nullptr;
    // If path does not exist, will be returned list only with the "END" Hex
    std::queue<std::shared_ptr<Hex>> Queue;
    bool reached_end = false;
    start->visited = true;

    Hex pos_tank = tank->getPosition();

    Queue.push(start);
    while (!Queue.empty() && !reached_end) {
        std::shared_ptr<Hex> current_node = Queue.front();
        Queue.pop();


        tank->update(current_node);
        std::vector<std::shared_ptr<Hex>> achievable_hexes = tank->getAchievableHexes(*this);


        for (std::shared_ptr<Hex> node: achievable_hexes) {
            if (!node->is_occupied && !node->visited) {
                node->visited = true;
                Queue.push(node);

                node->prev = current_node;
                if (std::find(ends.begin(), ends.end(), node) != ends.end()) {
                    reached_end = true;
                    end = node;
                    break;
                }
            }
        }
    }

    tank->update(pos_tank);

    std::vector<std::shared_ptr<Hex>> route = traceRoute(end);
    this->clearPath();
    return route;
}

std::vector<std::shared_ptr<Hex>> Map::findPath(Hex start, Hex end, std::shared_ptr<Tank> tank) {
    // If path does not exist, will be returned list only with the "END" Hex
    return this->findPath(this->getHex(start), this->getHex(end), tank);
}

std::vector<std::shared_ptr<Hex>>
Map::findPath(std::shared_ptr<Hex> start, std::shared_ptr<Hex> end, std::shared_ptr<Tank> tank) {
    // If path does not exist, will be returned list only with the "END" Hex

    std::vector<std::shared_ptr<Hex>> ends = {end};
    return this->findPath(start, ends, tank);
}

std::vector<std::shared_ptr<Hex>> Map::traceRoute(std::shared_ptr<Hex> end) {
    std::list<std::shared_ptr<Hex>> route;
    std::shared_ptr<Hex> node = end;
    while (node != nullptr) {
        route.push_front(node);
        node = node->prev;
    }
    std::vector<std::shared_ptr<Hex>> v{std::begin(route), std::end(route)};
    return v;
}

std::map<std::shared_ptr<Hex>, int> Map::getShootingMap(std::vector<std::shared_ptr<Tank>> opponent_vehicles) {
    std::map<std::shared_ptr<Hex>, int> shooting_map;
    for (const auto &opponent_vehicle: opponent_vehicles) {
        for (auto list: opponent_vehicle->getShootingHexesAreas(*this)) {
            for (auto hex: list) {
                auto value = shooting_map.find(hex);
                if (value == shooting_map.end()) {
                    shooting_map.insert(std::pair<std::shared_ptr<Hex>, int>(hex, 1));
                } else {
                    value->second++;
                }
            }
        }
    }
    return shooting_map;
}