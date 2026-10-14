#include "Rings.hpp"

using namespace geode::prelude;

static int s_frame = 0;
static std::unordered_map<RingObject*, int> s_lastOutline;
static std::unordered_map<RingObject*, int> s_lastHit;
static std::unordered_set<RingObject*> s_armed;
static std::vector<Ref<RingObject>> s_scaleReset;
static float s_resetDelay = 0.f;

bool tryConsumeRingWave(RingObject* ring, bool click) {
    auto& map = click ? s_lastHit : s_lastOutline;
    auto it = map.find(ring);
    if (it != map.end() && s_frame - it->second < 12) {
        return false;
    }
    map[ring] = s_frame;
    return true;
}

void expireRingWave(cocos2d::CCNode* wave, float delay) {
    if (!wave) return;
    wave->runAction(CCSequence::create(
        CCDelayTime::create(delay),
        CCCallFunc::create(wave, callfunc_selector(CCCircleWave::removeMeAndCleanup)),
        nullptr));
}

bool ringIsArmed(RingObject* ring) {
    return s_armed.count(ring) > 0;
}

void armRing(RingObject* ring) {
    s_armed.insert(ring);
}

void resetRingWaves() {
    s_lastOutline.clear();
    s_lastHit.clear();
    s_armed.clear();
}

void resetRingScale() {
    s_scaleReset.clear();
    s_resetDelay = 0.f;
}


void playtestRings(LevelEditorLayer* self, RingsData& data) {
    data.rings.clear();
    if (!self->m_objects) {
        return;
    }
    for (auto obj : CCArrayExt<GameObject*>(self->m_objects)) {
        if (auto ring = typeinfo_cast<RingObject*>(obj)) {
            ring->m_isActivated = false;
            ring->m_isRingPoweredOn = false;
            data.rings.push_back(ring);
        }
    }
}

void updateRings(LevelEditorLayer* self, RingsData& data) {
    s_frame++;
    for (auto it = s_armed.begin(); it != s_armed.end();) {
        auto ring = *it;
        bool close = false;
        for (auto player : {self->m_player1, self->m_player2}) {
            if (!player || !ring) {
                continue;
            }
            auto a = player->getPosition();
            auto b = ring->getPosition();
            float dx = a.x - b.x;
            float dy = a.y - b.y;
            if (dx * dx + dy * dy < 6400.f) {
                close = true;
                break;
            }
        }
        if (close) {
            ++it;
        } else {
            if (ring) {
                ring->m_isRingPoweredOn = false;
            }
            it = s_armed.erase(it);
        }
    }
}

void updateRingScale(LevelEditorLayer* self, RingsData& data, float pulse) {
    for (auto const& ref : data.rings) {
        auto ring = ref.data();
        if (!ring) {
            continue;
        }
        if (ring->m_unk3F8) {
            continue;
        }
        if (!ring->m_usesAudioScale || ring->m_hasNoAudioScale) {
            continue;
        }
        float v = pulse;
        if (ring->m_customAudioScale) {
            v = ring->m_minAudioScale
                + (ring->m_maxAudioScale - ring->m_minAudioScale) * (pulse - 0.1f);
        }
        ring->setRScale(std::min(v + 0.3f, 1.2f));
    }
}

void updateRingReset(float dt) {
    if (s_resetDelay <= 0.f) {
        return;
    }
    s_resetDelay -= dt;
    if (s_resetDelay > 0.f) {
        return;
    }
    for (auto ring : s_scaleReset) {
        if (ring) {
            ring->m_isActivated = false;
            ring->m_isRingPoweredOn = false;
            ring->resetRScaleForced();
            ring->setRScale(1.f);
        }
    }
    s_scaleReset.clear();
}

void stopRings(LevelEditorLayer* self, RingsData& data) {
    for (auto ring : data.rings) {
        if (ring) {
            ring->m_isActivated = false;
            ring->m_isRingPoweredOn = false;
            ring->resetRScaleForced();
            ring->setRScale(1.f);
            s_scaleReset.push_back(ring);
        }
    }
    s_resetDelay = 0.5f;
    data.rings.clear();
}

void damageRings(RingsData& data) {
    for (auto ring : data.rings) {
        if (ring) {
            ring->m_isRingPoweredOn = false;
            ring->resetRScaleForced();
        }
    }
}
