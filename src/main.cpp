#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

class $modify(PlaytestPlayerObject, PlayerObject) {
    bool init(int player, int ship, GJBaseGameLayer* gameLayer, cocos2d::CCLayer* layer, bool playLayer) {
        if (!PlayerObject::init(player, ship, gameLayer, layer, playLayer)) {
            return false;
        }

        if (gameLayer && gameLayer->m_isEditor) {
            this->setupStreak();
            this->addAllParticles();
        }

        return true;
    }
};

class $modify(PlaytestEditorLayer, LevelEditorLayer) {
    void onPlaytest() {
        LevelEditorLayer::onPlaytest();

        if (m_player1) {
            m_player1->m_playEffects = true;
            m_player1->activateStreak();
        }
        if (m_player2) {
            m_player2->m_playEffects = true;
            m_player2->activateStreak();
        }
    }

    void onStopPlaytest() {
        if (m_player1) {
            m_player1->m_playEffects = false;
            m_player1->deactivateStreak(true);
            m_player1->stopStreak2();
            m_player1->deactivateParticle();
        }
        if (m_player2) {
            m_player2->m_playEffects = false;
            m_player2->deactivateStreak(true);
            m_player2->stopStreak2();
            m_player2->deactivateParticle();
        }

        LevelEditorLayer::onStopPlaytest();
    }
};
