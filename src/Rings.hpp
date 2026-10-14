#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

struct RingsData {
    std::vector<Ref<RingObject>> rings;
};

bool tryConsumeRingWave(RingObject* ring, bool click);
void expireRingWave(cocos2d::CCNode* wave, float delay);
bool ringIsArmed(RingObject* ring);
void armRing(RingObject* ring);
void resetRingWaves();
void resetRingScale();

void playtestRings(LevelEditorLayer* self, RingsData& data);
void updateRings(LevelEditorLayer* self, RingsData& data);
void updateRingScale(LevelEditorLayer* self, RingsData& data, float pulse);
void updateRingReset(float dt);
void stopRings(LevelEditorLayer* self, RingsData& data);
void damageRings(RingsData& data);
