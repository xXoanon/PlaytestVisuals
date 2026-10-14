#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

int portalShineVariant(int objectID);
void spawnPortalShine(GameObject* obj, cocos2d::CCNode* layer,
                      std::vector<Ref<cocos2d::CCSprite>>& glows);
void syncPortalShine(std::vector<Ref<cocos2d::CCSprite>>& glows, cocos2d::CCNode* layer);
