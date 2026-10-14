#include "PortalBacks.hpp"
#include "EnterExitEffects.hpp"
#include "ObjectLayer.hpp"
#include "Portals.hpp"

using namespace geode::prelude;

static void syncPortalBack(LevelEditorLayer* self, cocos2d::CCSprite* back, GameObject* obj) {
    if (!back || !obj || !self->m_objectLayer) {
        return;
    }
    back->setPosition(layerPosition(self->m_objectLayer, obj));
    back->setRotation(obj->getRotation());
    back->setScaleX(obj->getScaleX());
    back->setScaleY(obj->getScaleY());
    back->setFlipY(obj->isFlipY());
    back->setOpacity(playtestObjectOpacity(obj));
    back->setVisible(obj->isVisible() && obj->getParent() != nullptr);
}

void playtestPortalBacks(LevelEditorLayer* self, PortalBackData& data) {
    if (!self->m_objects || !self->m_objectLayer) {
        return;
    }
    for (auto obj : CCArrayExt<GameObject*>(self->m_objects)) {
        if (!obj || !isPortalObject(obj)) {
            continue;
        }
        int variant = portalVariantForObject(obj);
        if (!variant) {
            continue;
        }
        auto name = fmt::format("portal_{:02}_back_001.png", variant);
        auto back = GameObject::createWithFrame(name.c_str());
        if (!back) {
            continue;
        }
        back->m_objectID = 0x26;
        back->copyGroups(obj);
        back->setColor(obj->getColor());
        syncPortalBack(self, back, obj);
        self->m_objectLayer->addChild(back, obj->getZOrder() - 100);
        data.backs.push_back(back);
        data.owners.push_back(obj);
    }
}

void syncPortalBacks(LevelEditorLayer* self, PortalBackData& data) {
    auto& backs = data.backs;
    auto& owners = data.owners;
    for (size_t i = 0; i < backs.size() && i < owners.size(); i++) {
        syncPortalBack(self, backs[i], owners[i]);
    }
}

void removePortalBacks(LevelEditorLayer* self, PortalBackData& data) {
    for (auto sprite : data.backs) {
        if (sprite) {
            sprite->removeFromParent();
        }
    }
    data.backs.clear();
    data.owners.clear();
}
