#include "Particles.hpp"
#include "ObjectLayer.hpp"
#include "PortalParticles.hpp"

using namespace geode::prelude;

static cocos2d::ccColor3B padColorFor(GameObjectType type) {
    switch (type) {
        case GameObjectType::PinkJumpPad:
        case GameObjectType::SpiderPad:
            return {255, 0, 255};
        case GameObjectType::GravityPad:
            return {0, 255, 255};
        case GameObjectType::RedJumpPad:
            return {255, 50, 0};
        default:
            return {255, 200, 0};
    }
}

cocos2d::ccColor3B getPadColor(GameObjectType type, GameObject* obj) {
    if (obj) {
        auto c = obj->getColor();
        if (c.r != 255 || c.g != 255 || c.b != 255) {
            return c;
        }
    }
    return padColorFor(type);
}

cocos2d::ccColor3B getRingColor(GameObjectType type, GameObject* obj) {
    if (obj) {
        if (auto sprite = obj->m_colorSprite) {
            auto c = sprite->getColor();
            if (c.r != 255 || c.g != 255 || c.b != 255) {
                return c;
            }
        }
        auto c = obj->getColor();
        if (c.r != 255 || c.g != 255 || c.b != 255) {
            return c;
        }
    }
    switch (type) {
        case GameObjectType::PinkJumpRing:
        case GameObjectType::SpiderOrb:
            return {255, 105, 255};
        case GameObjectType::GravityRing:
            return {100, 255, 255};
        case GameObjectType::GreenRing:
            return {100, 255, 85};
        case GameObjectType::DropRing:
            return {25, 25, 25};
        case GameObjectType::RedJumpRing:
            return {255, 95, 40};
        case GameObjectType::DashRing:
        case GameObjectType::GravityDashRing:
            return {0, 255, 0};
        case GameObjectType::YellowJumpRing:
            return {255, 255, 100};
        default:
            return {200, 200, 255};
    }
}

void attachParticle(LevelEditorLayer* self, ParticleData& data, GameObject* obj,
                    const char* plist, bool tint, cocos2d::ccColor3B color,
                    const PortalParticleTint* portalTint) {
    if (!obj || !self->m_objectLayer) {
        return;
    }
    auto p = CCParticleSystemQuad::create(plist, false);
    if (!p) {
        return;
    }
    p->setAutoRemoveOnFinish(false);
    p->setPosition(layerPosition(self->m_objectLayer, obj));
    p->setRotation(obj->getRotation());
    p->setScale(obj->getScale());
    p->setPositionType(kCCPositionTypeGrouped);
    if (obj->m_particle) {
        obj->m_particle->removeFromParent();
        obj->m_particle = nullptr;
    }
    obj->m_particle = p;
    if (portalTint) {
        if (portalTint->hasStart) {
            p->setStartColor(portalTint->start);
        }
        if (portalTint->hasEnd) {
            p->setEndColor(portalTint->end);
        }
    }
    if (tint) {
        obj->updateParticleColor(color);
    }
    self->m_objectLayer->addChild(p, obj->getZOrder() - 1);
    data.owners.push_back(obj);
    data.particles.push_back(p);
}

void unclaimParticles(LevelEditorLayer* self, ParticleData& data) {
    for (auto obj : data.claimed) {
        if (obj && obj->m_particle) {
            obj->unclaimParticle();
        }
    }
    data.claimed.clear();
}

void releaseOwnedParticles(LevelEditorLayer* self, ParticleData& data) {
    for (auto p : data.particles) {
        if (p) {
            p->removeFromParent();
        }
    }
    data.particles.clear();
    for (auto obj : data.owners) {
        if (obj && obj->m_particle) {
            obj->m_particle->removeFromParent();
            obj->m_particle = nullptr;
        }
    }
    data.owners.clear();
}

void syncOwnedParticles(LevelEditorLayer* self, ParticleData& data) {
    if (!self->m_objectLayer) {
        return;
    }
    auto& owners = data.owners;
    auto& parts = data.particles;
    for (size_t i = 0; i < owners.size() && i < parts.size(); i++) {
        auto obj = owners[i].data();
        auto p = parts[i].data();
        if (!obj || !p) {
            continue;
        }
        if (!p->getParent()) {
            self->m_objectLayer->addChild(p, obj->getZOrder() - 1);
        }
        if (obj->m_particle != p) {
            obj->m_particle = p;
        }
        p->setPosition(layerPosition(self->m_objectLayer, obj));
        p->setVisible(obj->isVisible() && obj->getParent() != nullptr);
    }
}
