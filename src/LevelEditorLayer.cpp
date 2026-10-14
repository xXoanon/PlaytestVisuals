#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/EditorUI.hpp>

#include "EnterExitEffects.hpp"
#include "Mirror.hpp"
#include "Particles.hpp"
#include "PlaytestGlow.hpp"
#include "PortalBacks.hpp"
#include "PortalParticles.hpp"
#include "Portals.hpp"
#include "Rings.hpp"
#include "SpeedPortals.hpp"

#include <algorithm>
#include <cmath>

using namespace geode::prelude;

class $modify(PlaytestEditorLayer, LevelEditorLayer) {
    struct Fields {
        bool m_origPreviewParticles = false;
        MirrorData m_mirror;
        SpeedData m_speed;
        PortalBackData m_backs;
        ParticleData m_particles;
        RingsData m_ringObjects;
    };

    void pauseMirror() {
        ::pauseMirror(this, m_fields->m_mirror);
        syncPortalBacks(this, m_fields->m_backs);
        syncOwnedParticles(this, m_fields->m_particles);
        dropSpeedVisuals(this, m_fields->m_speed);
    }

    void resumeMirror() {
        if (!::resumeMirror(this, m_fields->m_mirror)) {
            return;
        }
        syncPortalBacks(this, m_fields->m_backs);
        syncOwnedParticles(this, m_fields->m_particles);
    }

    void clearPauseMirror() {
        if (m_playbackMode == PlaybackMode::Paused
            && (m_cameraFlip != 1.f || !m_fields->m_mirror.orig.empty())) {
            this->pauseMirror();
        }
    }

    bool activateTriggerEffect(EffectGameObject* object, float currentTime, float playTime,
                               float boundTime, bool active) {
        if (m_playbackMode == PlaybackMode::Playing && object && isEnterTrigger(object)) {
            fireEnterTrigger(this, object);
            return true;
        }
        return LevelEditorLayer::activateTriggerEffect(object, currentTime, playTime, boundTime,
                                                       active);
    }

    void onPlaytest() {
        LevelEditorLayer::onPlaytest();
        m_fields->m_origPreviewParticles = m_previewParticles;
        m_previewParticles = true;
        this->updatePreviewParticles();
        unclaimParticles(this, m_fields->m_particles);
        releaseOwnedParticles(this, m_fields->m_particles);
        removePortalBacks(this, m_fields->m_backs);
        removeSpeedPortals(this, m_fields->m_speed);
        resetRingWaves();
        resetRingScale();
        resetSpeedStreak();
        restoreMirrorObjects(this, m_fields->m_mirror);
        resetMirrorState(this);
        resetEnterState(this);
        applyPlaytestGlow(this, true);
        playtestRings(this, m_fields->m_ringObjects);

        if (m_objects) {
            for (auto obj : CCArrayExt<GameObject*>(m_objects)) {
                if (!obj) {
                    continue;
                }
                if (auto plist = portalParticlePlist(obj->m_objectType, obj->m_objectID)) {
                    auto tint = portalParticleTint(obj->m_objectType, obj->m_objectID);
                    attachParticle(this, m_fields->m_particles, obj, plist, false,
                                   cocos2d::ccColor3B{255, 255, 255}, &tint);
                    continue;
                }
                if (auto plist = speedPortalPlist(obj->m_objectID)) {
                    SpeedPortalRing ring;
                    auto tint = speedPortalRingData(obj->m_objectID, ring)
                        ? ring.color
                        : cocos2d::ccColor3B{255, 255, 255};
                    attachParticle(this, m_fields->m_particles, obj, plist, true, tint);
                    playtestSpeedPortalRig(this, m_fields->m_speed, obj);
                    continue;
                }
                if (!obj->m_particle && !obj->m_hasNoParticles && !obj->m_particleLocked) {
                    obj->claimParticle();
                    if (obj->m_particle) {
                        m_fields->m_particles.claimed.push_back(obj);
                        continue;
                    }
                }
                if (obj->m_particle) {
                    continue;
                }
                if (typeinfo_cast<RingObject*>(obj)) {
                    auto tint = obj->m_objectType == GameObjectType::DropRing
                        ? cocos2d::ccColor3B{255, 255, 255}
                        : getRingColor(obj->m_objectType, obj);
                    attachParticle(this, m_fields->m_particles, obj, "ringEffect.plist",
                                   true, tint);
                } else if (obj->m_objectType == GameObjectType::YellowJumpPad ||
                           obj->m_objectType == GameObjectType::PinkJumpPad ||
                           obj->m_objectType == GameObjectType::GravityPad ||
                           obj->m_objectType == GameObjectType::RedJumpPad ||
                           obj->m_objectType == GameObjectType::SpiderPad) {
                    attachParticle(this, m_fields->m_particles, obj, "bumpEffect.plist", true,
                                   getPadColor(obj->m_objectType, obj));
                }
            }
        }
        FMODAudioEngine::sharedEngine()->enableMetering();
        playtestPortalBacks(this, m_fields->m_backs);
        if (auto scene = CCDirector::sharedDirector()->getRunningScene()) {
            if (auto old = scene->getChildByID("gravity-effect"_spr)) {
                old->removeFromParent();
            }
        }
        for (auto player : {m_player1, m_player2}) {
            if (!player) {
                continue;
            }
            player->m_playEffects = true;
            if (!player->m_regularTrail || !player->m_waveTrail) {
                player->setupStreak();
            }
            player->activateStreak();
        }
    }

    void updateVisibility(float dt) {
        this->clearPauseMirror();
        this->resumeMirror();
        LevelEditorLayer::updateVisibility(dt);
        if (m_playbackMode != PlaybackMode::Playing) {
            return;
        }
        float pulse = FMODAudioEngine::sharedEngine()->m_pulse1;
        updateRingScale(this, m_fields->m_ringObjects, pulse);
        applyPlaytestEnter(this);
        updateMirrorObjects(this, m_fields->m_mirror);
        syncSpeedGlows(this, m_fields->m_speed);
        updatePlaytestGlow(this);
        syncPortalBacks(this, m_fields->m_backs);
        syncOwnedParticles(this, m_fields->m_particles);
    }

    void updateEditor(float dt) {
        this->clearPauseMirror();
        this->resumeMirror();
        auto gm = GameManager::sharedState();
        auto origPlayLayer = gm->m_playLayer;
        bool origEnabled = gm->m_editorEnabled;
        bool scoped = false;
        if (m_playbackMode == PlaybackMode::Playing) {
            if (!origPlayLayer) {
                gm->m_playLayer = reinterpret_cast<PlayLayer*>(static_cast<GJBaseGameLayer*>(this));
            }
            gm->m_editorEnabled = false;
            scoped = true;
            updateRings(this, m_fields->m_ringObjects);
            updateSpeedStreak(this, m_fields->m_speed);
            float pulse = FMODAudioEngine::sharedEngine()->m_pulse1;
            if (m_player1) {
                m_player1->m_audioScale = pulse;
            }
            if (m_player2) {
                m_player2->m_audioScale = pulse;
            }
        }
        LevelEditorLayer::updateEditor(dt);
        if (gm->m_playLayer == reinterpret_cast<PlayLayer*>(static_cast<GJBaseGameLayer*>(this))) {
            gm->m_playLayer = origPlayLayer;
        }
        if (scoped) {
            gm->m_editorEnabled = origEnabled;
        }
        updateRingReset(dt);
    }

    void onStopPlaytest() {
        resetSpeedStreak();
        applyPlaytestGlow(this, false);
        resetEnterObjects(this);
        resetEnterState(this);
        restoreMirrorObjects(this, m_fields->m_mirror);
        resetMirrorState(this);
        removeSpeedPortals(this, m_fields->m_speed);
        stopRings(this, m_fields->m_ringObjects);
        unclaimParticles(this, m_fields->m_particles);
        releaseOwnedParticles(this, m_fields->m_particles);
        removePortalBacks(this, m_fields->m_backs);
        resetRingWaves();
        m_previewParticles = m_fields->m_origPreviewParticles;
        this->updatePreviewParticles();
        FMODAudioEngine::sharedEngine()->disableMetering();
        if (auto scene = CCDirector::sharedDirector()->getRunningScene()) {
            if (auto old = scene->getChildByID("gravity-effect"_spr)) {
                old->removeFromParent();
            }
        }
        for (auto player : {m_player1, m_player2}) {
            if (!player) {
                continue;
            }
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
        damageRings(m_fields->m_ringObjects);
        if (player) {
            player->playDeathEffect();
            player->spawnCircle2();
        }
    }
};

class $modify(PlaytestPauseButton, EditorUI) {
    void onPlaytest(cocos2d::CCObject* sender) {
        auto editor = LevelEditorLayer::get();
        bool wasPlaying = editor && editor->m_playbackMode == PlaybackMode::Playing;
        EditorUI::onPlaytest(sender);
        auto playtest = static_cast<PlaytestEditorLayer*>(LevelEditorLayer::get());
        if (!playtest) {
            return;
        }
        if (wasPlaying) {
            playtest->pauseMirror();
            return;
        }
        playtest->resumeMirror();
    }
};
