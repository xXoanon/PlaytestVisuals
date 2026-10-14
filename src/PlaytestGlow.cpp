#include "PlaytestGlow.hpp"
#include "ObjectLayer.hpp"
#include "Particles.hpp"
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/ObjectToolbox.hpp>
#include <Geode/binding/RingObject.hpp>

using namespace geode::prelude;

static void* glowPtr(GameObject* obj) {
    return *reinterpret_cast<void**>(reinterpret_cast<char*>(obj) + 0x2f0);
}

static unsigned char* gateByte(GameObject* obj) {
    return reinterpret_cast<unsigned char*>(reinterpret_cast<char*>(obj) + 0x28d);
}

using SlopeFill = void (*)(GameObject*, bool);

static SlopeFill slopeFill(int id) {
    if (id < 1743 || id > 1750) return nullptr;
    auto off = ((id - 1743) & 1) ? 0x1a2f00 : 0x1a2c30;
    return reinterpret_cast<SlopeFill>(
        reinterpret_cast<char*>(geode::base::get()) + off);
}

static void syncGlowTransform(GameObject* obj) {
    auto glow = reinterpret_cast<cocos2d::CCSprite*>(glowPtr(obj));
    if (!glow) return;
    auto layer = glow->getParent();
    if (!layer) return;
    glow->setPosition(layerPosition(layer, obj));
    glow->setRotation(obj->getRotation());
    glow->setScaleX(obj->getScaleX());
    glow->setScaleY(obj->getScaleY());
}

void applyPlaytestGlow(LevelEditorLayer* self, bool enable) {
    if (!self || !self->m_objects) return;
    if (!enable) {
        for (auto obj : CCArrayExt<GameObject*>(self->m_objects)) {
            if (!obj) continue;
            if (auto glow = reinterpret_cast<cocos2d::CCSprite*>(glowPtr(obj))) {
                glow->removeFromParent();
                obj->removeGlow();
            }
        }
        return;
    }
    auto toolbox = ObjectToolbox::sharedState();
    if (!toolbox) return;
    for (auto obj : CCArrayExt<GameObject*>(self->m_objects)) {
        if (!obj || glowPtr(obj)) continue;
        const char* frame = toolbox->intKeyToFrame(obj->m_objectID);
        auto fill = slopeFill(obj->m_objectID);
        auto gate = gateByte(obj);
        unsigned char saved = *gate;
        *gate = 0;
        if (frame && *frame) obj->addGlow(gd::string(frame));
        if (!glowPtr(obj) && fill) {
            auto where = obj->getPosition();
            obj->setPosition(cocos2d::CCPoint(0.f, 0.f));
            obj->addEmptyGlow();
            fill(obj, false);
            obj->setPosition(where);
        }
        *gate = saved;
        if (auto ring = typeinfo_cast<RingObject*>(obj)) {
            if (auto glow = reinterpret_cast<cocos2d::CCSprite*>(glowPtr(obj))) {
                glow->setColor(getRingColor(ring->m_objectType, ring));
            }
        }
    }
    for (auto obj : CCArrayExt<GameObject*>(self->m_objects)) {
        if (obj) {
            obj->m_isActivated = false;
        }
    }
}

void updatePlaytestGlow(LevelEditorLayer* self) {
    if (!self || !self->m_objects) return;
    for (auto obj : CCArrayExt<GameObject*>(self->m_objects)) {
        if (!obj) continue;
        syncGlowTransform(obj);
    }
}
