#include "PortalShine.hpp"
#include "ObjectLayer.hpp"

using namespace geode::prelude;

int portalShineVariant(int objectID) {
    switch (objectID) {
        case 10:
        case 11:
        case 2926: return 1;
        case 12:
        case 13:
        case 47:
        case 111:
        case 660:
        case 745:
        case 1331:
        case 1933: return 2;
        case 45:
        case 46: return 3;
        case 99:
        case 101: return 4;
        case 286:
        case 287: return 5;
        case 747:
        case 749:
        case 2064:
        case 2902: return 6;
        default: return 0;
    }
}

void spawnPortalShine(GameObject* obj, cocos2d::CCNode* layer,
                      std::vector<Ref<cocos2d::CCSprite>>& glows) {
    if (!obj || !layer) {
        return;
    }
    auto variant = portalShineVariant(obj->m_objectID);
    if (!variant) {
        return;
    }
    ccBlendFunc additive;
    additive.src = GL_SRC_ALPHA;
    additive.dst = GL_ONE;
    auto parent = obj->getParent();
    auto pos = layerPosition(layer, obj);
    std::vector<Ref<cocos2d::CCSprite>> live;
    for (auto glow : glows) {
        if (glow && glow->getParent()) {
            live.push_back(glow);
        }
    }
    glows.swap(live);
    for (auto glow : glows) {
        if (glow->getParent() == layer && glow->getPosition() == pos) {
            return;
        }
    }
    int z = (parent ? parent->getZOrder() : obj->getZOrder()) + 1;
    for (auto part : {"front", "back"}) {
        auto sprite = cocos2d::CCSprite::createWithSpriteFrameName(
            fmt::format("portalshine_{:02}_{}_001.png", variant, part).c_str());
        if (!sprite) {
            continue;
        }
        sprite->setBlendFunc(additive);
        sprite->setOpacity(0);
        sprite->setPosition(pos);
        sprite->setRotation(obj->getRotation());
        sprite->setScaleX(obj->getScaleX());
        sprite->setScaleY(obj->getScaleY());
        sprite->setFlipX(obj->isFlipX());
        sprite->setFlipY(obj->isFlipY());
        sprite->setUserObject(obj);
        sprite->runAction(CCSequence::create(
            CCFadeIn::create(0.05f),
            CCFadeOut::create(0.4f),
            CCCallFunc::create(sprite, callfunc_selector(CCNode::removeFromParent)),
            nullptr));
        layer->addChild(sprite, z);
        glows.push_back(sprite);
    }
    if (auto child = obj->getChildByTag(91)) {
        if (auto extra = cocos2d::CCSprite::createWithSpriteFrameName("portal_extra_shine_001.png")) {
            auto size = child->getContentSize();
            extra->setPosition(cocos2d::CCPoint(size.width * 0.5f, size.height * 0.5f));
            child->addChild(extra, 1);
        }
    }
}

void syncPortalShine(std::vector<Ref<cocos2d::CCSprite>>& glows, cocos2d::CCNode* layer) {
    std::vector<Ref<cocos2d::CCSprite>> live;
    for (auto glow : glows) {
        if (glow && glow->getParent()) {
            live.push_back(glow);
        }
    }
    glows.swap(live);
    if (!layer) {
        return;
    }
    for (auto glow : glows) {
        auto owner = typeinfo_cast<GameObject*>(glow->getUserObject());
        if (!owner) {
            continue;
        }
        glow->setPosition(layerPosition(layer, owner));
    }
}
