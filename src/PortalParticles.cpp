#include "PortalParticles.hpp"
#include "Portals.hpp"

using namespace geode::prelude;

PortalParticleTint portalParticleTint(GameObjectType type, int objectID) {
    auto none = [] { return PortalParticleTint{{0, 0, 0, 0}, {0, 0, 0, 0}, false, false}; };
    auto only = [](int r, int g, int b) {
        return PortalParticleTint{{(float)r / 255.f, (float)g / 255.f, (float)b / 255.f, 1.f}, {0, 0, 0, 0}, true, false};
    };
    auto both = [](int r1, int g1, int b1, int r2, int g2, int b2) {
        return PortalParticleTint{
            {(float)r1 / 255.f, (float)g1 / 255.f, (float)b1 / 255.f, 1.f},
            {(float)r2 / 255.f, (float)g2 / 255.f, (float)b2 / 255.f, 1.f},
            true, true};
    };
    if (isPortalType(type)) {
        switch (type) {
            case GameObjectType::InverseMirrorPortal: return only(255, 150, 0);
            case GameObjectType::BallPortal: return both(255, 100, 0, 255, 100, 0);
            case GameObjectType::UfoPortal: return both(255, 200, 0, 255, 100, 0);
            case GameObjectType::DualPortal: return both(255, 200, 0, 255, 100, 0);
            case GameObjectType::SoloPortal: return both(0, 200, 255, 0, 100, 255);
            case GameObjectType::WavePortal: return both(0, 200, 255, 0, 100, 255);
            case GameObjectType::RobotPortal: return both(150, 150, 150, 50, 50, 75);
            case GameObjectType::TeleportPortal: return both(0, 255, 255, 0, 100, 150);
            case GameObjectType::SpiderPortal: return both(200, 0, 255, 200, 0, 255);
            case GameObjectType::SwingPortal: return both(255, 255, 0, 255, 200, 0);
            case GameObjectType::GravityTogglePortal: return both(0, 255, 0, 0, 255, 0);
            default: break;
        }
    }
    if (isTeleportPortalID(objectID)) {
        return both(255, 200, 0, 255, 100, 0);
    }
    return none();
}

const char* portalParticlePlist(GameObjectType type, int objectID) {
    if (!isPortalType(type) && isTeleportPortalID(objectID)) {
        return "portalEffect02.plist";
    }
    switch (type) {
        case GameObjectType::NormalGravityPortal: return "portalEffect01.plist";
        case GameObjectType::InverseGravityPortal: return "portalEffect02.plist";
        case GameObjectType::CubePortal: return "portalEffect03.plist";
        case GameObjectType::ShipPortal: return "portalEffect04.plist";
        case GameObjectType::InverseMirrorPortal: return "portalEffect02.plist";
        case GameObjectType::NormalMirrorPortal: return "portalEffect01.plist";
        case GameObjectType::BallPortal: return "portalEffect02.plist";
        case GameObjectType::RegularSizePortal: return "portalEffect08.plist";
        case GameObjectType::MiniSizePortal: return "portalEffect09.plist";
        case GameObjectType::UfoPortal: return "portalEffect02.plist";
        case GameObjectType::DualPortal: return "portalEffect09.plist";
        case GameObjectType::SoloPortal: return "portalEffect09.plist";
        case GameObjectType::WavePortal: return "portalEffect02.plist";
        case GameObjectType::RobotPortal: return "portalEffect02.plist";
        case GameObjectType::SpiderPortal: return "portalEffect02.plist";
        case GameObjectType::SwingPortal: return "portalEffect02.plist";
        case GameObjectType::GravityTogglePortal: return "portalEffect02.plist";
        case GameObjectType::TeleportPortal: return "portalEffect02.plist";
        default: return nullptr;
    }
}
