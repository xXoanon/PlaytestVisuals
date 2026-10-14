#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

struct MirrorOrig {
    cocos2d::CCPoint pos;
    bool flipX;
    bool flipY;
    float rot;
};

struct MirrorData {
    std::unordered_map<GameObject*, MirrorOrig> orig;
    bool wasFlipping = false;
};

void resetMirrorState(LevelEditorLayer* self);
void unmirrorObjectLayer(LevelEditorLayer* self);
void pauseMirror(LevelEditorLayer* self, MirrorData& data);
bool resumeMirror(LevelEditorLayer* self, MirrorData& data);
void updateMirrorObjects(LevelEditorLayer* self, MirrorData& data);
void restoreMirrorObjects(LevelEditorLayer* self, MirrorData& data);
void remirrorObjects(LevelEditorLayer* self, MirrorData& data);
