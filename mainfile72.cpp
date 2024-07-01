#include <iostream>
#include <vector>
#include <deque>
#include <limits>
#include"greed_online.hpp"

using namespace std;




    static int findNearestShipToChase(std::deque<shipInfo>& shipList,ship& ob) {
        int minDistance = INT_MAX;
        int targetShipId = -1;
        Greed::coords myPos = ob.getCurrentTile();

        for (int i = 0; i < shipList.size(); ++i) {
            if (i != ob.getShipId() && shipList[i].getDiedStatus() == 0) {
                Greed::coords shipPos = shipList[i].getCurrentTile();
                int distance = std::abs(shipPos.r - myPos.r) + std::abs(shipPos.c - myPos.c);
                if (distance < minDistance) {
                    minDistance = distance;
                    targetShipId = i;
                }
            }
        }

        return targetShipId;
    }

    static int findNearestCannonToAttack(std::vector<Greed::Cannon>& cannonList, ship& ob) {
        int minDistance = INT_MAX;
        int targetCannonId = -1;
        Greed::coords myPos = ob.getCurrentTile();

        for (int i = 0; i < cannonList.size(); ++i) {
            Greed::coords cannonPos = cannonList[i].getCannonPosition();
            int distance = std::abs(cannonPos.c - myPos.c) + std::abs(cannonPos.r - myPos.r);
            if (distance < minDistance) {
                minDistance = distance;
                targetCannonId = i;
            }
        }

        return targetCannonId;
    }

     void GreedMain(ship& ob) 
     {
        ob.Greed_sail(Direction::NORTH_WEST);

        bool attackMode = false;
        int targetShipId = -1;
        int targetCannonId = -1;
        std::vector<Greed::coords> corners = {
            {0, 0},
            {0, 23},
            {10, 23},
            {10, 0}
        };
        size_t cornerIndex = 0;

        while (true) {
            ob.frame_rate_limiter();
            std::deque<shipInfo> shipList =ob.getShipList();
            std::vector<Greed::Cannon> cannonList =ob.getCannonList();
            Event e;
            ob.getNextCurrentEvent(e);

            if (targetShipId >= 0 && shipList[targetShipId].getDiedStatus() == 1) {
                targetShipId = findNearestShipToChase(shipList, ob);
                if (targetShipId >= 0) {
                    ob.Greed_chaseShip(targetShipId);
                    attackMode = true;
                }
                else {
                    cornerIndex = (static_cast<unsigned long long>(cornerIndex) + 1) % corners.size();
                    ob.Greed_setPath({ corners[cornerIndex] });
                    attackMode = false;
                }
            }

            if (targetCannonId >= 0 && cannonList[targetCannonId].isCannonDead()) {
                targetCannonId = findNearestCannonToAttack(cannonList, ob);
                if (targetCannonId >= 0) {
                    ob.Greed_fireAtCannon(targetCannonId, cannon::FRONT);
                    attackMode = true;
                }
            }

            if (e.eventType == Event::EventType::ShipCollision) {
                // Handle ship collision
            }

            if (ob.getCurrentHealth() <= 15) {
                ob.Greed_upgradeHealth(10);
            }

            if (ob.getCurrentAmmo() < 5) {
                ob.Greed_upgradeAmmo(10);
            }

            std::deque<Event> passiveEvents = ob.getPassiveEvent();
            for (auto& pe : passiveEvents) {
                if (pe.eventType == Event::EventType::ShipsInMyRadius && attackMode) {
                    for (auto& ship_id : pe.radiusShip.getShipId()) {
                        ob.Greed_fireCannon(cannon::FRONT, ship_id, ShipSide::FRONT);
                    }
                }
                if (pe.eventType == Event::EventType::IamInCannonRadius && attackMode) {
                    ob.Greed_fireAtCannon(pe.radiusCannon.getCannonId()[0], cannon::FRONT);
                }
            }
        }
    }
