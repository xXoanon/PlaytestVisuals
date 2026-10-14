#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

bool isEnterTrigger(GameObject* obj);
void fireEnterTrigger(LevelEditorLayer* self, EffectGameObject* obj);
void resetEnterState(LevelEditorLayer* self);
void applyPlaytestEnter(LevelEditorLayer* self);
void resetEnterObjects(LevelEditorLayer* self);
unsigned char playtestObjectOpacity(GameObject* obj);
