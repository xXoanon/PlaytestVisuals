#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

using namespace geode::prelude;

class $modify(PlaytestEditorLayer, LevelEditorLayer) {
    void onPlaytest() {
        LevelEditorLayer::onPlaytest();

        FMODAudioEngine::sharedEngine()->m_metering = true;

        for (auto* player : {m_player1, m_player2}) {
            if (!player) continue;
            player->m_playEffects = true;
            if (!player->m_regularTrail || !player->m_waveTrail) {
                player->setupStreak();
            }
            player->activateStreak();
        }
    }

    void updateEditor(float dt) {
        auto gm = GameManager::sharedState();
        auto origPlayLayer = gm->m_playLayer;

        auto* editorFlag = reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(gm) + 0x2ba);
        bool origEditorFlag = *editorFlag;

        if (m_playbackMode == PlaybackMode::Playing) {
            if (!origPlayLayer) {
                gm->m_playLayer = reinterpret_cast<PlayLayer*>(static_cast<GJBaseGameLayer*>(this));
            }
            *editorFlag = false;

            float pulse = FMODAudioEngine::sharedEngine()->m_pulse1;
            if (m_player1) m_player1->m_audioScale = pulse;
            if (m_player2) m_player2->m_audioScale = pulse;
        }

        LevelEditorLayer::updateEditor(dt);

        if (gm->m_playLayer == reinterpret_cast<PlayLayer*>(static_cast<GJBaseGameLayer*>(this))) {
            gm->m_playLayer = origPlayLayer;
        }
        *editorFlag = origEditorFlag;
    }

    void onStopPlaytest() {
        FMODAudioEngine::sharedEngine()->m_metering = false;

        for (auto* player : {m_player1, m_player2}) {
            if (!player) continue;
            player->m_playEffects = false;
            player->deactivateStreak(true);
            player->stopStreak2();
            if (player->m_waveTrail) {
                player->m_waveTrail->stopStroke();
                player->m_waveTrail->reset();
            }
            if (player->m_regularTrail) {
                player->m_regularTrail->stopStroke();
                player->m_regularTrail->reset();
            }
            player->deactivateParticle();
            player->setVisible(true);
            player->setOpacity(255);
        }

        LevelEditorLayer::onStopPlaytest();
    }

    void playerTookDamage(PlayerObject* player) {
        LevelEditorLayer::playerTookDamage(player);

        if (player) {
            player->playDeathEffect();
            player->spawnCircle2();
        }
    }
};
