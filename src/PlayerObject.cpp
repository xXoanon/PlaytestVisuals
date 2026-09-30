#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

class $modify(PlaytestPlayerObject, PlayerObject) {
    bool init(int player, int ship, GJBaseGameLayer* gameLayer, cocos2d::CCLayer* layer, bool playLayer) {
        if (!PlayerObject::init(player, ship, gameLayer, layer, playLayer)) {
            return false;
        }

        if (gameLayer && gameLayer->m_isEditor) {
            m_playEffects = true;
            this->setupStreak();
            this->addAllParticles();
        }

        return true;
    }

    void activateStreak() {
        if (m_gameLayer && m_gameLayer->m_isEditor) {
            auto gm = GameManager::sharedState();
            auto* editorFlag = reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(gm) + 0x2ba);
            bool origFlag = *editorFlag;
            *editorFlag = false;

            PlayerObject::activateStreak();

            *editorFlag = origFlag;
            return;
        }

        PlayerObject::activateStreak();
    }

    void update(float dt) {
        auto gm = GameManager::sharedState();
        auto origPlayLayer = gm->m_playLayer;

        if (!origPlayLayer && m_gameLayer && m_gameLayer->m_isEditor) {
            gm->m_playLayer = reinterpret_cast<PlayLayer*>(m_gameLayer);
        }

        PlayerObject::update(dt);

        if (gm->m_playLayer == reinterpret_cast<PlayLayer*>(m_gameLayer)) {
            gm->m_playLayer = origPlayLayer;
        }
    }

    bool levelFlipping() {
        if (!m_playEffects) return false;

        if (m_gameLayer) {
            return m_gameLayer->isFlipping();
        }

        return false;
    }

    void spawnPortalCircle(cocos2d::ccColor3B color, float startRadius) {
        if (m_gameLayer && m_gameLayer->m_isEditor) {
            auto wave = CCCircleWave::create(startRadius, 5.f, 0.3f, true, true);
            wave->m_color = color;
            wave->setPosition(m_lastPortalPos);
            if (m_lastActivatedPortal) {
                wave->followObject(m_lastActivatedPortal, true);
            }
            m_gameLayer->m_objectLayer->addChild(wave);
            return;
        }

        PlayerObject::spawnPortalCircle(color, startRadius);
    }

    void incrementJumps() {
        if (m_gameLayer && m_gameLayer->m_isEditor) {
            return;
        }

        PlayerObject::incrementJumps();
    }

    void playDeathEffect() {
        auto gm = GameManager::sharedState();
        auto origPlayLayer = gm->m_playLayer;

        if (!origPlayLayer && m_gameLayer && m_gameLayer->m_isEditor) {
            gm->m_playLayer = reinterpret_cast<PlayLayer*>(m_gameLayer);
        }

        PlayerObject::playDeathEffect();

        if (gm->m_playLayer == reinterpret_cast<PlayLayer*>(m_gameLayer)) {
            gm->m_playLayer = origPlayLayer;
        }
    }
};
