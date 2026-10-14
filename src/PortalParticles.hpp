#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

struct PortalParticleTint {
    cocos2d::ccColor4F start;
    cocos2d::ccColor4F end;
    bool hasStart;
    bool hasEnd;
};

PortalParticleTint portalParticleTint(GameObjectType type, int objectID);
const char* portalParticlePlist(GameObjectType type, int objectID);
