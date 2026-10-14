#include "Portals.hpp"

using namespace geode::prelude;

bool isTeleportPortalID(int objectID) {
    switch (objectID) {
        case 747:
        case 749:
        case 2064:
        case 2902:
            return true;
        default:
            return false;
    }
}

bool isPortalType(GameObjectType type) {
    switch (type) {
        case GameObjectType::NormalGravityPortal:
        case GameObjectType::InverseGravityPortal:
        case GameObjectType::ShipPortal:
        case GameObjectType::CubePortal:
        case GameObjectType::InverseMirrorPortal:
        case GameObjectType::NormalMirrorPortal:
        case GameObjectType::BallPortal:
        case GameObjectType::RegularSizePortal:
        case GameObjectType::MiniSizePortal:
        case GameObjectType::UfoPortal:
        case GameObjectType::DualPortal:
        case GameObjectType::SoloPortal:
        case GameObjectType::WavePortal:
        case GameObjectType::RobotPortal:
        case GameObjectType::SpiderPortal:
        case GameObjectType::SwingPortal:
        case GameObjectType::GravityTogglePortal:
        case GameObjectType::TeleportPortal:
            return true;
        default:
            return false;
    }
}

int portalFrontVariantFromID(int objectID) {
    switch (objectID) {
        case 10: return 1;
        case 11: return 2;
        case 12: return 3;
        case 13: return 4;
        case 45: return 5;
        case 46: return 6;
        case 47: return 7;
        case 99: return 8;
        case 101: return 9;
        case 111: return 10;
        case 287: return 11;
        case 286: return 12;
        case 660: return 13;
        case 745: return 14;
        case 747: return 15;
        case 2902: return 15;
        case 749: return 16;
        case 2064: return 16;
        case 1331: return 17;
        case 1933: return 18;
        case 2926: return 19;
        default: return 0;
    }
}

int portalBackVariantFromID(int objectID) {
    int variant = portalFrontVariantFromID(objectID);
    if (objectID == 286) {
        return 11;
    }
    if (objectID == 287) {
        return 12;
    }
    return variant;
}

int portalVariantFromFrame(cocos2d::CCSprite* sprite) {
    if (!sprite) {
        return 0;
    }
    auto frame = sprite->displayFrame();
    if (!frame) {
        return 0;
    }
    auto rect = frame->getRect();
    auto cache = cocos2d::CCSpriteFrameCache::sharedSpriteFrameCache();
    for (int i = 1; i <= 19; i++) {
        auto candidate = cache->spriteFrameByName(fmt::format("portal_{:02}_front_001.png", i).c_str());
        if (!candidate) {
            continue;
        }
        auto other = candidate->getRect();
        if (other.origin.x == rect.origin.x && other.origin.y == rect.origin.y) {
            return i;
        }
    }
    return 0;
}

int portalVariantForObject(GameObject* obj) {
    if (!obj) {
        return 0;
    }
    int variant = portalBackVariantFromID(obj->m_objectID);
    if (variant) {
        return variant;
    }
    return portalVariantFromFrame(static_cast<cocos2d::CCSprite*>(obj));
}

bool isPortalObject(GameObject* obj) {
    if (!obj) {
        return false;
    }
    return portalVariantForObject(obj) != 0 || isPortalType(obj->m_objectType);
}
