#include "GravitySweep.hpp"
#include <Geode/binding/GravityEffectSprite.hpp>

using namespace geode::prelude;

static cocos2d::ccColor3B s_portalColor = {255, 255, 255};
static bool s_portalColorFresh = false;

void setPortalColor(cocos2d::ccColor3B color) {
    s_portalColor = color;
    s_portalColorFresh = true;
}

void resetPortalColor() {
    s_portalColorFresh = false;
}

cocos2d::ccColor3B gravitySweepColor(bool flip) {
    if (s_portalColorFresh) {
        return s_portalColor;
    }
    return flip ? cocos2d::ccColor3B{255, 200, 0} : cocos2d::ccColor3B{0, 150, 255};
}

void triggerGravitySweep(bool flip, bool sideways, cocos2d::ccColor3B color) {
    auto director = CCDirector::sharedDirector();
    auto scene = director->getRunningScene();
    if (!scene) {
        return;
    }
    if (auto old = scene->getChildByID("gravity-effect"_spr)) {
        old->removeFromParent();
    }
    auto sprite = GravityEffectSprite::create();
    if (!sprite) {
        return;
    }
    sprite->setID("gravity-effect"_spr);
    sprite->updateSpritesColor(color);
    auto win = director->getWinSize();
    CCPoint from;
    CCPoint to;
    if (!sideways) {
        sprite->setFlipY(!flip);
        if (flip) {
            from = CCPoint(win.width * 0.5f, -95.f);
            to = CCPoint(win.width * 0.5f, win.height + 95.f);
        } else {
            from = CCPoint(win.width * 0.5f, win.height + 95.f);
            to = CCPoint(win.width * 0.5f, -95.f);
        }
    } else {
        sprite->setRotation(90.f);
        if (flip) {
            from = CCPoint(-95.f, win.height * 0.5f);
            to = CCPoint(win.width + 95.f, win.height * 0.5f);
        } else {
            from = CCPoint(win.width + 95.f, win.height * 0.5f);
            to = CCPoint(-95.f, win.height * 0.5f);
        }
    }
    sprite->setPosition(from);
    scene->addChild(sprite, 100);
    auto move = CCMoveTo::create(0.4f, to);
    auto clean = CCCallFunc::create(sprite, callfunc_selector(CCNode::removeFromParent));
    sprite->runAction(CCSequence::create(move, clean, nullptr));
}
