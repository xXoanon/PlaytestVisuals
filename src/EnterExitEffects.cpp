#include "EnterExitEffects.hpp"
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/binding/EnterEffectObject.hpp>
#include <Geode/binding/PlayLayer.hpp>

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

using namespace geode::prelude;

constexpr uintptr_t kEnterRightEdge = 0x6C10BC;

static std::unordered_map<GameObject*, unsigned char> s_playedOpacity;
static std::unordered_set<GameObject*> s_fired;

void setObjectOpacity(GameObject* obj, unsigned char opacity) {
    auto target = reinterpret_cast<void*>(reinterpret_cast<char*>(obj) + 0x140);
    auto vtable = *reinterpret_cast<void***>(target);
    reinterpret_cast<void(*)(void*, unsigned char)>(vtable[5])(target, opacity);
    s_playedOpacity[obj] = opacity;
}

unsigned char playtestObjectOpacity(GameObject* obj) {
    auto it = s_playedOpacity.find(obj);
    return it == s_playedOpacity.end() ? 255 : it->second;
}

bool isEnterTriggerID(int id) {
    if (id >= 22 && id <= 28) return true;
    if (id >= 55 && id <= 59) return true;
    if (id == 1915) return true;
    if (id >= 3006 && id <= 3024) return true;
    return false;
}

bool isEnterTrigger(GameObject* obj) {
    if (!obj) return false;
    if (typeinfo_cast<EnterEffectObject*>(obj)) return true;
    return isEnterTriggerID(obj->m_objectID);
}

void fireEnterTrigger(LevelEditorLayer* self, EffectGameObject* obj) {
    if (!self || !obj) return;
    auto gm = GameManager::sharedState();
    bool editor = self->m_isEditor;
    bool enabled = gm->m_editorEnabled;
    auto lel = gm->m_levelEditorLayer;
    auto play = gm->m_playLayer;
    self->m_isEditor = false;
    gm->m_editorEnabled = false;
    gm->m_levelEditorLayer = nullptr;
    if (!play) gm->m_playLayer = reinterpret_cast<PlayLayer*>(static_cast<GJBaseGameLayer*>(self));
    static const gd::vector<int> emptyKeys;
    obj->triggerObject(self, obj->m_uniqueID, &emptyKeys);
    self->m_isEditor = editor;
    gm->m_editorEnabled = enabled;
    gm->m_levelEditorLayer = lel;
    gm->m_playLayer = play;
}

void resetEnterState(LevelEditorLayer* self) {
    if (!self) return;
    self->resetActiveEnterEffects();
    self->resetStoppedAreaObjects();
    self->m_gameState.m_enterEffectInstanceVectors.clear();
    self->m_gameState.m_exitEffectInstanceVectors.clear();
    s_playedOpacity.clear();
    s_fired.clear();
}

void resetEnterObjects(LevelEditorLayer* self) {
    if (!self || !self->m_objects) return;
    for (auto obj : CCArrayExt<GameObject*>(self->m_objects)) {
        if (!obj) continue;
        self->resetAreaObjectValues(obj, true);
    }
}

void applyPlaytestEnter(LevelEditorLayer* self) {
    if (!self || !self->m_objects) return;
    auto base = reinterpret_cast<char*>(static_cast<GJBaseGameLayer*>(self));
    auto play = reinterpret_cast<PlayLayer*>(static_cast<GJBaseGameLayer*>(self));
    auto anchorAddr = reinterpret_cast<cocos2d::CCPoint*>(base + 0x389c);
    cocos2d::CCPoint savedAnchor = *anchorAddr;
    float camX = *reinterpret_cast<float*>(base + 0x50c);
    float viewW = *reinterpret_cast<float*>(base + 0x325c);
    float edge = camX + viewW;
    *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(geode::base::get()) + kEnterRightEdge) = edge;
    float baseThresh = *reinterpret_cast<float*>(base + 0x3268) + camX;
    auto director = cocos2d::CCDirector::sharedDirector();
    float half = director->getWinSize().width * 0.5f;
    float invisLeftBound = half - 75.f;
    float invisRightBound = half + 35.f;
    float invisLeftWidth = half - 105.f;
    float invisRightWidth = director->getScreenRight() - invisRightBound - 90.f;
    float playerX = self->m_player1 ? self->m_player1->getRealPosition().x : camX;
    for (auto obj : CCArrayExt<GameObject*>(self->m_objects)) {
        if (!obj || !isEnterTrigger(obj) || s_fired.count(obj)) continue;
        if (playerX < obj->getRealPosition().x) continue;
        s_fired.insert(obj);
        if (auto effect = typeinfo_cast<EffectGameObject*>(obj)) {
            fireEnterTrigger(self, effect);
        }
    }
    for (auto obj : CCArrayExt<GameObject*>(self->m_objects)) {
        if (!obj || obj == self->m_player1 || obj == self->m_player2) continue;
        *anchorAddr = obj->getRealPosition();
        if (obj->m_unk3F8) continue;
        bool right;
        int explicitType;
        if (obj->m_enterType != -1) {
            right = true;
            explicitType = obj->m_enterType;
        } else if (obj->m_exitType != -1) {
            right = false;
            explicitType = obj->m_exitType;
        } else {
            right = anchorAddr->x > baseThresh;
            explicitType = -1;
        }
        int code = 0;
        int ch = obj->m_enterChannel;
        if (ch >= 0) {
            auto& map = *reinterpret_cast<gd::vector<int>*>(base + (right ? 0x628 : 0x640));
            if (ch < (int)map.size()) code = map[ch];
        }
        if (obj->m_isUIObject) {
            code = -14;
            explicitType = -14;
        }
        if (obj->m_isInvisibleBlock) {
            if (!self->m_player2 || !self->m_player2->m_isDead) {
                play->updateInvisibleBlock(obj, invisRightBound, invisLeftBound,
                                           invisRightWidth, invisLeftWidth,
                                           self->m_lightBGColor);
            } else {
                setObjectOpacity(obj, 255);
            }
            if (code != -15) play->applyEnterEffect(obj, code, right);
            else if (!obj->m_ignoreEnter) play->applyCustomEnterEffect(obj, right);
            else play->applyEnterEffect(obj, code, right);
            continue;
        }
        if (obj->m_ignoreFade || code == -14) {
            setObjectOpacity(obj, 255);
            if (code != -15) play->applyEnterEffect(obj, code, right);
            else if (!obj->m_ignoreEnter) play->applyCustomEnterEffect(obj, right);
            else play->applyEnterEffect(obj, code, right);
            continue;
        }
        bool premade = obj->m_intrinsicDontFade
            && (obj->m_isSolidColorBlock || obj->m_baseOrDetailBlending)
            && (explicitType == -2 || explicitType == -1) && code == -2;
        if (premade) {
            setObjectOpacity(obj, 255);
            play->applyEnterEffect(obj, code, right);
            continue;
        }
        if (code == -15) {
            if (!obj->m_ignoreEnter) play->applyCustomEnterEffect(obj, right);
            else play->applyEnterEffect(obj, code, right);
            continue;
        }
        float x = obj->getRealPosition().x;
        float m = obj->m_fadeMargin;
        float dist = right ? (edge - (x - m)) : ((x + m) - camX);
        float fade = std::clamp(dist / 70.f, 0.f, 1.f);
        setObjectOpacity(obj, (unsigned char)(int)(fade * 255.f));
        play->applyEnterEffect(obj, code, right);
        continue;
    }
    *anchorAddr = savedAnchor;
}
