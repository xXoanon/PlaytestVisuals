#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

struct SpeedPortalRing {
    float radius;
    cocos2d::ccColor3B color;
};

struct SpeedData {
    std::vector<Ref<cocos2d::CCSprite>> glows;
    std::vector<Ref<CCCircleWave>> rings;
    std::unordered_set<GameObject*> touching;
    std::unordered_set<GameObject*> fired;
};

int speedPortalVariant(int objectID);
bool speedPortalRingData(int objectID, SpeedPortalRing& out);
const char* speedPortalPlist(int objectID);
const char* speedStreakPlist(int objectID);

void playtestSpeedPortalRig(LevelEditorLayer* self, SpeedData& data, GameObject* obj);
void updateSpeedStreak(LevelEditorLayer* self, SpeedData& data);
void syncSpeedGlows(LevelEditorLayer* self, SpeedData& data);
void dropSpeedVisuals(LevelEditorLayer* self, SpeedData& data);
void removeSpeedPortals(LevelEditorLayer* self, SpeedData& data);
void resetSpeedStreak();
