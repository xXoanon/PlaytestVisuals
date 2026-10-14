#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

#include "PortalShine.hpp"
#include "Rings.hpp"

using namespace geode::prelude;

class $modify(PlaytestGameLayer, GJBaseGameLayer) {
    struct Fields {
        std::vector<Ref<cocos2d::CCSprite>> m_cubeGlows;
    };

    void playerWillSwitchMode(PlayerObject* player, GameObject* object) {
        GJBaseGameLayer::playerWillSwitchMode(player, object);
        if (!(m_isEditor || GameManager::sharedState()->m_levelEditorLayer)) {
            return;
        }
        if (!object || object->m_objectID != 12 || object->m_hasNoEffects) {
            return;
        }
        auto gm = GameManager::sharedState();
        auto lel = gm->m_levelEditorLayer;
        if (!lel || lel->m_playbackMode != PlaybackMode::Playing) {
            return;
        }
        if (!m_objectLayer) {
            return;
        }
        spawnPortalShine(object, m_objectLayer, m_fields->m_cubeGlows);
    }

    void update(float dt) {
        GJBaseGameLayer::update(dt);
        syncPortalShine(m_fields->m_cubeGlows, m_objectLayer);
    }

    void collisionCheckObjects(PlayerObject* player, gd::vector<GameObject*>* objects, int count, float dt) {
        bool editor = m_isEditor;
        m_isEditor = false;
        GJBaseGameLayer::collisionCheckObjects(player, objects, count, dt);
        m_isEditor = editor;
    }

    void toggleDualMode(GameObject* obj, bool dual, PlayerObject* player, bool noEffects) {
        GJBaseGameLayer::toggleDualMode(obj, dual, player, noEffects);
        if (!(m_isEditor || GameManager::sharedState()->m_levelEditorLayer)) {
            return;
        }
        if (!m_player2) {
            return;
        }
        if (dual) {
            m_player2->m_playEffects = true;
            if (!m_player2->m_regularTrail || !m_player2->m_waveTrail) {
                m_player2->setupStreak();
            }
            m_player2->activateStreak();
            m_player2->spawnDualCircle();
            if (m_player1) {
                m_player1->spawnDualCircle();
            }
        } else {
            m_player2->deactivateStreak(true);
            m_player2->stopStreak2();
            if (m_player2->m_waveTrail) {
                m_player2->m_waveTrail->stopStroke();
                m_player2->m_waveTrail->reset();
            }
            if (m_player2->m_regularTrail) {
                m_player2->m_regularTrail->stopStroke();
                m_player2->m_regularTrail->reset();
            }
        }
    }
};
