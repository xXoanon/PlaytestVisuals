#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

void triggerGravitySweep(bool flip, bool sideways, cocos2d::ccColor3B color);

void setPortalColor(cocos2d::ccColor3B color);
void resetPortalColor();
cocos2d::ccColor3B gravitySweepColor(bool flip);
