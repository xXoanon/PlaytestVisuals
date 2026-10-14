#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

struct PortalBackData {
    std::vector<Ref<cocos2d::CCSprite>> backs;
    std::vector<Ref<GameObject>> owners;
};

void playtestPortalBacks(LevelEditorLayer* self, PortalBackData& data);
void syncPortalBacks(LevelEditorLayer* self, PortalBackData& data);
void removePortalBacks(LevelEditorLayer* self, PortalBackData& data);
