#include "SpeedPortals.hpp"
#include "ObjectLayer.hpp"

using namespace geode::prelude;

static float s_prevPlayerX = 0.f;

int speedPortalVariant(int objectID) {
    switch (objectID) {
        case 200: return 1;
        case 201: return 2;
        case 202: return 3;
        case 203: return 4;
        case 1334: return 5;
        default: return 0;
    }
}

bool speedPortalRingData(int objectID, SpeedPortalRing& out) {
    switch (objectID) {
        case 200: out = {60.f, {255, 255, 0}}; return true;
        case 201: out = {65.f, {0, 150, 255}}; return true;
        case 202: out = {70.f, {0, 255, 150}}; return true;
        case 203: out = {80.f, {255, 0, 255}}; return true;
        case 1334: out = {90.f, {255, 50, 50}}; return true;
        default: return false;
    }
}

const char* speedPortalPlist(int objectID) {
    switch (objectID) {
        case 200: return "boost_01_effect.plist";
        case 201: return "boost_02_effect.plist";
        case 202: return "boost_03_effect.plist";
        case 203: return "boost_04_effect.plist";
        case 1334: return "boost_04_effect.plist";
        default: return nullptr;
    }
}

const char* speedStreakPlist(int objectID) {
    switch (objectID) {
        case 200: return "speedEffect_slow.plist";
        case 201: return "speedEffect_normal.plist";
        case 202: return "speedEffect_fast.plist";
        case 203: return "speedEffect_vfast.plist";
        case 1334: return "speedEffect_vvfast.plist";
        default: return nullptr;
    }
}

static cocos2d::CCSize hitSize(GameObject* obj) {
    auto size = obj->getOuterObjectRect().size;
    if (size.width <= 0.f) {
        size.width = 30.f;
    }
    if (size.height <= 0.f) {
        size.height = 30.f;
    }
    return size;
}

static cocos2d::CCRect hitRect(LevelEditorLayer* self, GameObject* obj) {
    auto center = layerPosition(self->m_objectLayer, obj);
    auto size = hitSize(obj);
    return cocos2d::CCRect(center.x - size.width * 0.5f, center.y - size.height * 0.5f,
                          size.width, size.height);
}

static bool speedPortalGlowAlive(LevelEditorLayer* self, SpeedData& data, GameObject* obj) {
    auto pos = layerPosition(self->m_objectLayer, obj);
    for (auto glow : data.glows) {
        if (glow && glow->getParent() == self->m_objectLayer && glow->getPosition() == pos) {
            return true;
        }
    }
    return false;
}

void playtestSpeedPortalRig(LevelEditorLayer* self, SpeedData& data, GameObject* obj) {
    auto variant = speedPortalVariant(obj->m_objectID);
    if (!variant) {
        return;
    }
    auto shine = cocos2d::CCSprite::createWithSpriteFrameName(
        fmt::format("boost_{:02}_shine_001.png", variant).c_str());
    if (!shine) {
        return;
    }
    ccBlendFunc additive;
    additive.src = GL_SRC_ALPHA;
    additive.dst = GL_ONE;
    shine->setBlendFunc(additive);
    shine->setOpacity(0);
    shine->setPosition(layerPosition(self->m_objectLayer, obj));
    shine->setRotation(obj->getRotation());
    shine->setScaleX(obj->getScaleX());
    shine->setScaleY(obj->getScaleY());
    shine->setUserObject(obj);
    shine->runAction(CCSequence::create(
        CCFadeIn::create(0.05f),
        CCFadeOut::create(0.4f),
        CCCallFunc::create(shine, callfunc_selector(CCNode::removeFromParent)),
        nullptr));
    auto parent = obj->getParent();
    self->m_objectLayer->addChild(shine, (parent ? parent->getZOrder() : obj->getZOrder()) + 1);
    data.glows.push_back(shine);
}

static void spawnSpeedPortalRing(LevelEditorLayer* self, SpeedData& data, GameObject* obj) {
    SpeedPortalRing ring_data;
    if (!self->m_objectLayer || !speedPortalRingData(obj->m_objectID, ring_data)) {
        return;
    }
    auto ring = CCCircleWave::create(5.f, ring_data.radius, 0.3f, false, true);
    if (!ring) {
        return;
    }
    ring->setPosition(layerPosition(self->m_objectLayer, obj));
    ring->m_color = ring_data.color;
    ring->m_circleMode = CircleMode::Outline;
    ring->m_lineWidth = 6;
    ring->m_blendAdditive = true;
    ring->m_opacityMod = 1.f;
    auto parent = obj->getParent();
    ring->followObject(obj, false);
    self->m_objectLayer->addChild(ring, (parent ? parent->getZOrder() : obj->getZOrder()) - 1);
    data.rings.push_back(ring);
}

static void spawnSpeedStreak(LevelEditorLayer* self, GameObject* obj) {
    if (!speedStreakPlist(obj->m_objectID) || !self->m_objectLayer || !self->m_player1) {
        return;
    }
    auto p = CCParticleSystemQuad::create(speedStreakPlist(obj->m_objectID), false);
    if (!p) {
        return;
    }
    p->setAutoRemoveOnFinish(true);
    p->setPosition(layerPosition(self->m_objectLayer, self->m_player1));
    self->m_objectLayer->addChild(p, 100);
}

void updateSpeedStreak(LevelEditorLayer* self, SpeedData& data) {
    if (!self->m_player1 || !self->m_objects || !self->m_objectLayer) {
        return;
    }
    auto prev = s_prevPlayerX;
    auto playerRect = hitRect(self, self->m_player1);
    s_prevPlayerX = playerRect.origin.x + playerRect.size.width * 0.5f;
    bool forward = prev == prev && s_prevPlayerX > prev;
    auto& touching = data.touching;
    auto& fired = data.fired;
    for (auto obj : CCArrayExt<GameObject*>(self->m_objects)) {
        if (!obj || !speedStreakPlist(obj->m_objectID)) {
            continue;
        }
        if (!playerRect.intersectsRect(hitRect(self, obj))) {
            touching.erase(obj);
            continue;
        }
        if (!touching.insert(obj).second || !forward) {
            continue;
        }
        if (fired.find(obj) != fired.end()) {
            continue;
        }
        fired.insert(obj);
        if (!speedPortalGlowAlive(self, data, obj)) {
            playtestSpeedPortalRig(self, data, obj);
        }
        spawnSpeedStreak(self, obj);
        spawnSpeedPortalRing(self, data, obj);
    }
}

void syncSpeedGlows(LevelEditorLayer* self, SpeedData& data) {
    auto& glows = data.glows;
    std::vector<Ref<cocos2d::CCSprite>> live;
    for (auto glow : glows) {
        if (glow && glow->getParent()) {
            live.push_back(glow);
        }
    }
    glows.swap(live);
    for (auto glow : glows) {
        auto owner = typeinfo_cast<GameObject*>(glow->getUserObject());
        if (!owner) {
            continue;
        }
        auto layer = glow->getParent();
        if (!layer) {
            continue;
        }
        glow->setPosition(layerPosition(layer, owner));
        glow->setRotation(owner->getRotation());
        glow->setScaleX(owner->getScaleX());
        glow->setScaleY(owner->getScaleY());
    }
}

void dropSpeedVisuals(LevelEditorLayer* self, SpeedData& data) {
    for (auto glow : data.glows) {
        if (glow) {
            glow->removeFromParent();
        }
    }
    data.glows.clear();
    for (auto ring : data.rings) {
        if (ring) {
            ring->removeFromParent();
        }
    }
    data.rings.clear();
}

void removeSpeedPortals(LevelEditorLayer* self, SpeedData& data) {
    dropSpeedVisuals(self, data);
    data.touching.clear();
    data.fired.clear();
}

void resetSpeedStreak() {
    s_prevPlayerX = NAN;
}
