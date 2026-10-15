#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

#include "GravitySweep.hpp"
#include "Particles.hpp"
#include "PortalShine.hpp"
#include "Rings.hpp"

#include <cmath>

using namespace geode::prelude;

class $modify(PlaytestPlayerObject, PlayerObject) {
    struct Fields {
        bool m_gravityInit = false;
        bool m_gravity = false;
        std::vector<Ref<cocos2d::CCSprite>> m_portalGlows;
    };

    bool init(int player, int ship, GJBaseGameLayer* gameLayer, cocos2d::CCLayer* layer, bool playLayer) {
        if (!PlayerObject::init(player, ship, gameLayer, layer, playLayer)) {
            return false;
        }
        if (gameLayer && (gameLayer->m_isEditor || GameManager::sharedState()->m_levelEditorLayer)) {
            this->setupStreak();
            this->addAllParticles();
        }
        return true;
    }

    void activateStreak() {
        if (m_gameLayer && (m_gameLayer->m_isEditor || GameManager::sharedState()->m_levelEditorLayer)) {
            auto gm = GameManager::sharedState();
            bool effects = m_playEffects;
            bool editor = m_gameLayer->m_isEditor;
            bool enabled = gm->m_editorEnabled;
            auto lel = gm->m_levelEditorLayer;
            auto play = gm->m_playLayer;
            m_playEffects = true;
            m_gameLayer->m_isEditor = false;
            gm->m_editorEnabled = false;
            gm->m_levelEditorLayer = nullptr;
            if (!play) {
                gm->m_playLayer = reinterpret_cast<PlayLayer*>(m_gameLayer);
            }
            PlayerObject::activateStreak();
            m_playEffects = effects;
            m_gameLayer->m_isEditor = editor;
            gm->m_editorEnabled = enabled;
            gm->m_levelEditorLayer = lel;
            gm->m_playLayer = play;
            return;
        }
        PlayerObject::activateStreak();
    }

    void update(float dt) {
        auto gm = GameManager::sharedState();
        auto play = gm->m_playLayer;
        bool swapped = false;
        if (!play && m_gameLayer && (m_gameLayer->m_isEditor || gm->m_levelEditorLayer)) {
            gm->m_playLayer = reinterpret_cast<PlayLayer*>(m_gameLayer);
            swapped = true;
        }
        resetPortalColor();
        PlayerObject::update(dt);
        if (swapped && gm->m_playLayer == reinterpret_cast<PlayLayer*>(m_gameLayer)) {
            gm->m_playLayer = play;
        }
        if (!m_gameLayer || !(m_gameLayer->m_isEditor || gm->m_levelEditorLayer)) {
            return;
        }
        syncPortalShine(m_fields->m_portalGlows, m_gameLayer->m_objectLayer);
        if (!m_fields->m_gravityInit) {
            m_fields->m_gravityInit = true;
            m_fields->m_gravity = m_isUpsideDown;
            return;
        }
        if (m_isUpsideDown == m_fields->m_gravity) {
            return;
        }
        m_fields->m_gravity = m_isUpsideDown;
        if (m_lastActivatedPortal && m_lastActivatedPortal->m_hasNoEffects) {
            return;
        }
        triggerGravitySweep(m_isUpsideDown, m_isSideways, gravitySweepColor(m_isUpsideDown));
    }

    void placeStreakPoint() {
        if (m_gameLayer && (m_gameLayer->m_isEditor || GameManager::sharedState()->m_levelEditorLayer)) {
            auto gm = GameManager::sharedState();
            bool editor = m_gameLayer->m_isEditor;
            auto lel = gm->m_levelEditorLayer;
            auto play = gm->m_playLayer;
            m_gameLayer->m_isEditor = false;
            gm->m_levelEditorLayer = nullptr;
            if (!play) {
                gm->m_playLayer = reinterpret_cast<PlayLayer*>(m_gameLayer);
            }
            PlayerObject::placeStreakPoint();
            m_gameLayer->m_isEditor = editor;
            gm->m_levelEditorLayer = lel;
            gm->m_playLayer = play;
            return;
        }
        PlayerObject::placeStreakPoint();
    }

    void flashPortalWhite(GameObject* obj) {
        auto layer = m_gameLayer ? m_gameLayer->m_objectLayer : nullptr;
        spawnPortalShine(obj, layer, m_fields->m_portalGlows);
    }

    void spawnPortalCircle(cocos2d::ccColor3B color, float radius) {
        setPortalColor(color);
        if (m_gameLayer && (m_gameLayer->m_isEditor || GameManager::sharedState()->m_levelEditorLayer)) {
            if (!m_gameLayer->m_objectLayer) {
                return;
            }
            auto wave = CCCircleWave::create(radius, 5.f, 0.3f, true, true);
            if (!wave) {
                return;
            }
            wave->m_color = color;
            wave->setPosition(m_lastPortalPos);
            if (m_lastActivatedPortal) {
                wave->followObject(m_lastActivatedPortal, false);
                m_gameLayer->m_objectLayer->addChild(wave, m_lastActivatedPortal->getZOrder());
            } else {
                m_gameLayer->m_objectLayer->addChild(wave);
            }
            if (m_lastActivatedPortal && !m_lastActivatedPortal->m_hasNoEffects) {
                this->flashPortalWhite(m_lastActivatedPortal);
            }
            return;
        }
        PlayerObject::spawnPortalCircle(color, radius);
    }

    void toggleDartMode(bool enable, bool noEffects) {
        if (m_gameLayer && (m_gameLayer->m_isEditor || GameManager::sharedState()->m_levelEditorLayer)) {
            auto gm = GameManager::sharedState();
            bool effects = m_playEffects;
            bool enabled = gm->m_editorEnabled;
            auto play = gm->m_playLayer;
            m_playEffects = false;
            gm->m_editorEnabled = false;
            if (!play) {
                gm->m_playLayer = reinterpret_cast<PlayLayer*>(m_gameLayer);
            }
            PlayerObject::toggleDartMode(enable, noEffects);
            gm->m_playLayer = play;
            gm->m_editorEnabled = enabled;
            m_playEffects = effects;
            if (!effects || !enable || noEffects || m_isDead) {
                return;
            }
            if (!m_gameLayer->m_objectLayer) {
                return;
            }
            if (m_lastActivatedPortal && !m_lastActivatedPortal->m_hasNoEffects) {
                this->flashPortalWhite(m_lastActivatedPortal);
            }
            auto wave = CCCircleWave::create(10.f, 60.f, 0.4f, false, true);
            if (!wave) {
                return;
            }
            if (m_lastActivatedPortal) {
                wave->m_color = m_lastActivatedPortal->getColor();
            } else {
                wave->m_color = {0, 255, 255};
            }
            wave->setPosition(m_lastPortalPos);
            wave->m_circleMode = CircleMode::Outline;
            wave->m_lineWidth = 4;
            if (m_lastActivatedPortal) {
                wave->followObject(m_lastActivatedPortal, false);
                m_gameLayer->m_objectLayer->addChild(wave, m_lastActivatedPortal->getZOrder());
            } else {
                m_gameLayer->m_objectLayer->addChild(wave);
            }
            return;
        }
        PlayerObject::toggleDartMode(enable, noEffects);
    }

    void spawnScaleCircle() {
        if (m_gameLayer && (m_gameLayer->m_isEditor || GameManager::sharedState()->m_levelEditorLayer)) {
            if (!m_playEffects) {
                return;
            }
            if (!m_gameLayer->m_objectLayer) {
                return;
            }
            if (m_lastActivatedPortal && !m_lastActivatedPortal->m_hasNoEffects) {
                this->flashPortalWhite(m_lastActivatedPortal);
            }
            auto wave = CCCircleWave::create(10.f, 40.f, 0.3f, false, true);
            if (!wave) {
                return;
            }
            if (m_lastActivatedPortal) {
                wave->m_color = m_lastActivatedPortal->getColor();
            } else {
                wave->m_color = {255, 255, 255};
            }
            wave->setPosition(m_lastPortalPos);
            if (m_lastActivatedPortal) {
                wave->followObject(m_lastActivatedPortal, false);
                m_gameLayer->m_objectLayer->addChild(wave, m_lastActivatedPortal->getZOrder());
            } else {
                m_gameLayer->m_objectLayer->addChild(wave);
            }
            return;
        }
        PlayerObject::spawnScaleCircle();
    }

    void incrementJumps() {
        if (m_gameLayer && (m_gameLayer->m_isEditor || GameManager::sharedState()->m_levelEditorLayer)) {
            return;
        }
        PlayerObject::incrementJumps();
    }

    void playDeathEffect() {
        auto gm = GameManager::sharedState();
        auto old = gm->m_playLayer;
        bool swapped = false;
        if (!old && m_gameLayer && (m_gameLayer->m_isEditor || gm->m_levelEditorLayer)) {
            gm->m_playLayer = reinterpret_cast<PlayLayer*>(m_gameLayer);
            swapped = true;
        }
        PlayerObject::playDeathEffect();
        if (swapped && gm->m_playLayer == reinterpret_cast<PlayLayer*>(m_gameLayer)) {
            gm->m_playLayer = old;
        }
    }

    void spawnSpiderDashStreak(cocos2d::CCPoint from, cocos2d::CCPoint to, cocos2d::ccColor3B color) {
        auto layer = m_gameLayer ? m_gameLayer->m_objectLayer : nullptr;
        if (!layer) {
            return;
        }
        auto sprite = CCSprite::createWithSpriteFrameName("spiderDash_002.png");
        if (!sprite) {
            return;
        }
        if (m_isSideways) {
            sprite->setRotation(m_isUpsideDown ? 0.f : 180.f);
        } else {
            sprite->setRotation(m_isUpsideDown ? 90.f : -90.f);
        }
        float width = sprite->getContentSize().width;
        if (width > 0.f) {
            float fromAxis = m_isSideways ? from.x : from.y;
            float toAxis = m_isSideways ? to.x : to.y;
            sprite->setScaleX((std::fabs(toAxis - fromAxis) + 30.f) / width);
        }
        if (m_isSideways) {
            sprite->setPosition(cocos2d::CCPoint((from.x + to.x) * 0.5f, from.y));
        } else {
            sprite->setPosition(cocos2d::CCPoint(from.x, (from.y + to.y) * 0.5f));
        }
        sprite->setColor(color);
        sprite->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
        layer->addChild(sprite, 40);
        auto cache = CCSpriteFrameCache::sharedSpriteFrameCache();
        auto frames = CCArray::create();
        for (int i = 1; i <= 8; i++) {
            auto name = fmt::format("spiderDash_{:03}.png", i);
            auto frame = cache->spriteFrameByName(name.c_str());
            if (frame) {
                frames->addObject(frame);
            }
        }
        if (frames->count() == 0) {
            return;
        }
        auto anim = CCAnimation::createWithSpriteFrames(frames, 0.04f);
        auto cleanup = CCCallFunc::create(sprite, callfunc_selector(CCNode::removeFromParent));
        sprite->runAction(CCSequence::create(CCAnimate::create(anim), cleanup, nullptr));
    }

    void playSpiderDashEffect(cocos2d::CCPoint from, cocos2d::CCPoint to) {
        if (m_gameLayer && (m_gameLayer->m_isEditor || GameManager::sharedState()->m_levelEditorLayer)) {
            bool effects = m_playEffects;
            m_playEffects = false;
            PlayerObject::playSpiderDashEffect(from, to);
            m_playEffects = effects;
            float side = m_isGoingLeft ? -1.f : 1.f;
            from.x += side * 7.5f;
            to.x += side * 7.5f;
            auto color = m_swapColors ? m_playerColor1 : m_playerColor2;
            float size = m_vehicleSize > 0.f ? m_vehicleSize : 1.f;
            auto parent = m_parentLayer ? m_parentLayer : m_gameLayer->m_objectLayer;
            if (parent) {
                auto a = CCCircleWave::create(size * 13.f, 1.f, 0.15f, false, false);
                if (a) {
                    a->m_color = color;
                    a->m_opacityMod = 0.42f;
                    a->setPosition(from);
                    parent->addChild(a, 0);
                }
                auto b = CCCircleWave::create(size * 26.f, 2.f, 0.25f, false, false);
                if (b) {
                    b->m_color = color;
                    b->m_opacityMod = 0.84f;
                    b->setPosition(to);
                    parent->addChild(b, 0);
                }
                auto c = CCCircleWave::create(size * 10.f, size * 45.f, 0.25f, false, false);
                if (c) {
                    c->m_color = color;
                    c->m_circleMode = CircleMode::Outline;
                    c->setPosition(to);
                    parent->addChild(c, 0);
                }
            }
            m_flashMainColor = {255, 255, 255};
            m_flashSecondColor = {255, 255, 255};
            m_flashTime = m_totalTime;
            m_flashDuration = 0.3f;
            m_flashDelay = 0.05f;
            if (m_spiderSprite) {
                m_spiderSprite->stopActionByTag(10);
                m_spiderSprite->setScale(0.4f);
                auto squash = CCEaseElasticOut::create(CCScaleTo::create(0.45f, 1.f), 0.35f);
                squash->setTag(10);
                m_spiderSprite->runAction(squash);
            }
            this->spawnSpiderDashStreak(from, to, color);
            return;
        }
        PlayerObject::playSpiderDashEffect(from, to);
    }

    void ringJump(RingObject* ring, bool skipCheck) {
        if (m_gameLayer && (m_gameLayer->m_isEditor || GameManager::sharedState()->m_levelEditorLayer)) {
            bool effects = m_playEffects;
            double yBefore = m_yVelocity;
            bool flipBefore = m_isUpsideDown;
            bool dashBefore = m_isDashing;
            auto posBefore = this->getPosition();
            m_playEffects = false;
            PlayerObject::ringJump(ring, skipCheck);
            m_playEffects = effects;
            if (!ring) {
                return;
            }
            bool jumped = skipCheck || m_yVelocity != yBefore || m_isUpsideDown != flipBefore ||
                (m_isDashing && !dashBefore) || posBefore.x != this->getPosition().x || posBefore.y != this->getPosition().y;
            if (!jumped) {
                return;
            }
            ring->playTriggerEffect();
            if (ring->m_hasNoEffects) {
                return;
            }
            if (!tryConsumeRingWave(ring, true)) {
                return;
            }
            if (!m_gameLayer->m_objectLayer) {
                return;
            }
            float radius = 35.f;
            if (ring->m_objectType == GameObjectType::RedJumpRing) {
                radius = 42.f;
            }
            auto wave = CCCircleWave::create(radius, 5.f, 0.35f, true, true);
            if (!wave) {
                return;
            }
            wave->m_color = getRingColor(ring->m_objectType, ring);
            wave->setPosition(ring->getPosition());
            wave->followObject(ring, false);
            m_gameLayer->m_objectLayer->addChild(wave, ring->getZOrder());
            expireRingWave(wave, 0.35f);
            return;
        }
        PlayerObject::ringJump(ring, skipCheck);
    }

    void playBumpEffect(int type, GameObject* obj) {
        if (m_gameLayer && (m_gameLayer->m_isEditor || GameManager::sharedState()->m_levelEditorLayer)) {
            if (!m_playEffects) {
                return;
            }
            if (!m_gameLayer->m_objectLayer) {
                return;
            }
            auto target = m_lastActivatedPortal ? static_cast<GameObject*>(m_lastActivatedPortal) : obj;
            float radius = 10.f;
            if (m_vehicleSize >= 1.f && (type == 34 || type == 35)) {
                radius = 12.f;
            }
            auto wave = CCCircleWave::create(radius, 40.f, 0.25f, false, true);
            if (!wave) {
                return;
            }
            wave->m_color = getPadColor(static_cast<GameObjectType>(type), target);
            if (target) {
                wave->setPosition(target->getPosition());
                wave->followObject(target, false);
                m_gameLayer->m_objectLayer->addChild(wave, target->getZOrder());
            } else {
                wave->setPosition(this->getPosition());
                m_gameLayer->m_objectLayer->addChild(wave);
            }
            return;
        }
        PlayerObject::playBumpEffect(type, obj);
    }

    void spawnDualCircle() {
        if (m_gameLayer && (m_gameLayer->m_isEditor || GameManager::sharedState()->m_levelEditorLayer)) {
            if (!m_playEffects) {
                return;
            }
            if (!m_gameLayer->m_objectLayer) {
                return;
            }
            auto wave = CCCircleWave::create(5.f, 25.f, 0.25f, true, true);
            if (!wave) {
                return;
            }
            wave->m_color = m_playerColor1;
            wave->setPosition(this->getPosition());
            wave->followObject(this, false);
            m_gameLayer->m_objectLayer->addChild(wave);
            if (m_lastActivatedPortal && !m_lastActivatedPortal->m_hasNoEffects) {
                this->flashPortalWhite(m_lastActivatedPortal);
            }
            return;
        }
        PlayerObject::spawnDualCircle();
    }
};
