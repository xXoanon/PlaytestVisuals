#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

bool isPortalType(GameObjectType type);
bool isTeleportPortalID(int objectID);
bool isPortalObject(GameObject* obj);

int portalFrontVariantFromID(int objectID);
int portalBackVariantFromID(int objectID);
int portalVariantFromFrame(cocos2d::CCSprite* sprite);
int portalVariantForObject(GameObject* obj);
