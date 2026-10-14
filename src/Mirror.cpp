#include "Mirror.hpp"
#include "EnterExitEffects.hpp"

using namespace geode::prelude;

void resetMirrorState(LevelEditorLayer* self) {
    self->m_gameState.m_isFlipped = false;
    self->m_gameState.m_levelFlipProgress = 0.f;
    self->m_gameState.m_inSecondHalfOfFlipAnimation = false;
    self->m_gameState.m_isFlipping = false;
    self->m_gameState.stopTweenAction(7);
    self->m_cameraFlip = 1.f;
    self->stopActionByTag(14);
}

void unmirrorObjectLayer(LevelEditorLayer* self) {
    if (!self->m_objectLayer) {
        return;
    }
    float sx = self->m_objectLayer->getScaleX();
    if (sx >= 0.f) {
        return;
    }
    float winW = cocos2d::CCDirector::sharedDirector()->getWinSize().width;
    self->m_objectLayer->setScaleX(-sx);
    self->m_objectLayer->setPositionX(winW - self->m_objectLayer->getPositionX());
}

void restoreMirrorObjects(LevelEditorLayer* self, MirrorData& data) {
    for (auto& entry : data.orig) {
        auto obj = entry.first;
        auto& orig = entry.second;
        if (!obj) {
            continue;
        }
        obj->setPosition(orig.pos);
        obj->setFlipX(orig.flipX);
        obj->setFlipY(orig.flipY);
        obj->setRotation(orig.rot);
    }
    data.orig.clear();
    data.wasFlipping = false;
}

void pauseMirror(LevelEditorLayer* self, MirrorData& data) {
    bool wasFlipped = self->m_gameState.m_isFlipped;
    resetEnterObjects(self);
    restoreMirrorObjects(self, data);
    resetMirrorState(self);
    if (wasFlipped) {
        self->m_gameState.m_isFlipped = true;
    }
    unmirrorObjectLayer(self);
}

static void mirrorObject(GameObject* obj, MirrorOrig& orig, float visW, float camX, float eff) {
    float x = orig.pos.x + (visW - 2.f * (orig.pos.x - camX)) * eff;
    obj->setPosition(cocos2d::CCPoint(x, orig.pos.y));
    bool useY = (fabsf(orig.rot) == 90.f || fabsf(orig.rot) == 270.f);
    bool flipped = (eff >= 0.5f);
    if (useY) {
        obj->setFlipY(orig.flipY != flipped);
    } else {
        obj->setFlipX(orig.flipX != flipped);
    }
    if (static_cast<int>(orig.rot) % 90 != 0) {
        obj->setRotation(flipped ? -orig.rot : orig.rot);
    }
}

bool resumeMirror(LevelEditorLayer* self, MirrorData& data) {
    if (self->m_playbackMode != PlaybackMode::Playing || !self->m_gameState.m_isFlipped
        || !data.orig.empty()) {
        return false;
    }
    float progress = self->m_gameState.m_levelFlipProgress;
    if ((progress != 0.f && progress != 1.f) || self->m_gameState.m_isFlipping) {
        return false;
    }
    remirrorObjects(self, data);
    return true;
}

void updateMirrorObjects(LevelEditorLayer* self, MirrorData& data) {
    float progress = self->m_gameState.m_levelFlipProgress;
    bool flipping = (progress != 0.f && progress != 1.f) || self->m_gameState.m_isFlipping;
    if (!flipping) {
        if (data.wasFlipping) {
            for (auto& entry : data.orig) {
                auto obj = entry.first;
                auto& orig = entry.second;
                if (!obj) {
                    continue;
                }
                obj->setFlipX(orig.flipX);
                obj->setFlipY(orig.flipY);
                obj->setRotation(orig.rot);
            }
            data.wasFlipping = false;
        } else if (progress == 0.f && self->m_cameraFlip != -1.f) {
            for (auto& entry : data.orig) {
                if (entry.first) {
                    entry.second.pos = entry.first->getPosition();
                }
            }
        }
        return;
    }
    data.wasFlipping = true;
    float eff = (self->m_cameraFlip == -1.f) ? 1.f - progress : progress;
    float zoom = self->m_gameState.m_cameraZoom;
    float camX = self->m_gameState.m_cameraPosition.x;
    float winW = CCDirector::sharedDirector()->getWinSize().width;
    float visW = winW / zoom;
    if (!self->m_objects) {
        return;
    }
    for (auto obj : CCArrayExt<GameObject*>(self->m_objects)) {
        if (!obj || !obj->isVisible()) {
            continue;
        }
        if (obj == self->m_player1 || obj == self->m_player2) {
            continue;
        }
        auto [found, fresh] = data.orig.try_emplace(obj);
        if (fresh) {
            found->second.pos = obj->getRealPosition();
            found->second.flipX = obj->isFlipX();
            found->second.flipY = obj->isFlipY();
            found->second.rot = obj->getRotation();
        }
        mirrorObject(obj, found->second, visW, camX, eff);
    }
}

void remirrorObjects(LevelEditorLayer* self, MirrorData& data) {
    if (!self->m_objects || !self->m_objectLayer) {
        return;
    }
    float zoom = self->m_gameState.m_cameraZoom;
    float camX = self->m_gameState.m_cameraPosition.x;
    float winW = CCDirector::sharedDirector()->getWinSize().width;
    float visW = winW / zoom;
    for (auto obj : CCArrayExt<GameObject*>(self->m_objects)) {
        if (!obj) {
            continue;
        }
        if (obj == self->m_player1 || obj == self->m_player2) {
            continue;
        }
        auto [found, fresh] = data.orig.try_emplace(obj);
        if (fresh) {
            found->second.pos = obj->getRealPosition();
            found->second.flipX = obj->isFlipX();
            found->second.flipY = obj->isFlipY();
            found->second.rot = obj->getRotation();
        }
        auto& orig = found->second;
        float x = orig.pos.x + (visW - 2.f * (orig.pos.x - camX));
        obj->setPosition(cocos2d::CCPoint(x, orig.pos.y));
    }
    self->m_gameState.m_isFlipped = true;
    self->m_gameState.m_levelFlipProgress = 1.f;
    self->m_gameState.m_inSecondHalfOfFlipAnimation = false;
    self->m_gameState.m_isFlipping = false;
    self->m_cameraFlip = -1.f;
    data.wasFlipping = false;
    if (self->m_objectLayer->getScaleX() > 0.f) {
        self->m_objectLayer->setScaleX(-self->m_objectLayer->getScaleX());
        self->m_objectLayer->setPositionX(winW - self->m_objectLayer->getPositionX());
    }
}
