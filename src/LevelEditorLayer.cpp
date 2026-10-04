#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/RingObject.hpp>
#include <Geode/binding/GravityEffectSprite.hpp>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

using namespace geode::prelude;

cocos2d::ccColor3B getPadColor(GameObjectType type, GameObject* obj);
cocos2d::ccColor3B getRingColor(GameObjectType type, GameObject* obj);

struct PortalParticleTint {
    cocos2d::ccColor4F start;
    cocos2d::ccColor4F end;
    bool hasStart;
    bool hasEnd;
};

PortalParticleTint portalParticleTint(GameObjectType type) {
    auto none = [] { return PortalParticleTint{{0, 0, 0, 0}, {0, 0, 0, 0}, false, false}; };
    auto only = [](int r, int g, int b) {
        return PortalParticleTint{{(float)r / 255.f, (float)g / 255.f, (float)b / 255.f, 1.f}, {0, 0, 0, 0}, true, false};
    };
    auto both = [](int r1, int g1, int b1, int r2, int g2, int b2) {
        return PortalParticleTint{
            {(float)r1 / 255.f, (float)g1 / 255.f, (float)b1 / 255.f, 1.f},
            {(float)r2 / 255.f, (float)g2 / 255.f, (float)b2 / 255.f, 1.f},
            true, true};
    };
    switch (type) {
        case GameObjectType::InverseMirrorPortal: return only(255, 150, 0);
        case GameObjectType::BallPortal: return both(255, 100, 0, 255, 100, 0);
        case GameObjectType::UfoPortal: return both(255, 200, 0, 255, 100, 0);
        case GameObjectType::DualPortal: return both(255, 200, 0, 255, 100, 0);
        case GameObjectType::SoloPortal: return both(0, 200, 255, 0, 100, 255);
        case GameObjectType::WavePortal: return both(0, 200, 255, 0, 100, 255);
        case GameObjectType::RobotPortal: return both(150, 150, 150, 50, 50, 75);
        case GameObjectType::TeleportPortal: return both(0, 255, 255, 0, 100, 150);
        case GameObjectType::SpiderPortal: return both(200, 0, 255, 200, 0, 255);
        case GameObjectType::SwingPortal: return both(255, 255, 0, 255, 200, 0);
        case GameObjectType::GravityTogglePortal: return both(0, 255, 0, 0, 255, 0);
        default: return none();
    }
}

static int s_frame = 0;
static std::unordered_map<RingObject*, int> s_lastOutline;
static std::unordered_map<RingObject*, int> s_lastHit;
static std::unordered_set<RingObject*> s_armed;
static std::vector<Ref<RingObject>> s_scaleReset;
static float s_resetDelay = 0.f;

bool tryConsumeRingWave(RingObject* ring, bool click) {
    auto& map = click ? s_lastHit : s_lastOutline;
    auto it = map.find(ring);
    if (it != map.end() && s_frame - it->second < 12) {
        return false;
    }
    map[ring] = s_frame;
    return true;
}

bool isPortalType(GameObjectType type) {
    switch (type) {
        case GameObjectType::NormalGravityPortal:
        case GameObjectType::InverseGravityPortal:
        case GameObjectType::ShipPortal:
        case GameObjectType::CubePortal:
        case GameObjectType::InverseMirrorPortal:
        case GameObjectType::NormalMirrorPortal:
        case GameObjectType::BallPortal:
        case GameObjectType::RegularSizePortal:
        case GameObjectType::MiniSizePortal:
        case GameObjectType::UfoPortal:
        case GameObjectType::DualPortal:
        case GameObjectType::SoloPortal:
        case GameObjectType::WavePortal:
        case GameObjectType::RobotPortal:
        case GameObjectType::SpiderPortal:
        case GameObjectType::SwingPortal:
        case GameObjectType::GravityTogglePortal:
            return true;
        default:
            return false;
    }
}

const char* portalBackFrame(int objectID) {
    if (objectID >= 10 && objectID <= 111) {
        switch (objectID) {
            case 11: return "portal_02_back_001.png";
            case 12: return "portal_03_back_001.png";
            case 13: return "portal_04_back_001.png";
            case 45: return "portal_05_back_001.png";
            case 46: return "portal_06_back_001.png";
            case 47: return "portal_07_back_001.png";
            case 99: return "portal_08_back_001.png";
            case 101: return "portal_09_back_001.png";
            case 111: return "portal_10_back_001.png";
            default: return "portal_01_back_001.png";
        }
    }
    switch (objectID) {
        case 286: return "portal_11_back_001.png";
        case 287: return "portal_12_back_001.png";
        case 660: return "portal_13_back_001.png";
        case 745: return "portal_14_back_001.png";
        case 747: return "portal_15_back_001.png";
        case 749: return "portal_16_back_001.png";
        case 1331: return "portal_17_back_001.png";
        case 1933: return "portal_18_back_001.png";
        case 2064: return "portal_16_back_001.png";
        case 2902: return "portal_15_back_001.png";
        case 2926: return "portal_19_back_001.png";
        default: return "portal_01_back_001.png";
    }
}

const char* portalParticlePlist(GameObjectType type) {
    switch (type) {
        case GameObjectType::NormalGravityPortal: return "portalEffect01.plist";
        case GameObjectType::InverseGravityPortal: return "portalEffect02.plist";
        case GameObjectType::CubePortal: return "portalEffect03.plist";
        case GameObjectType::ShipPortal: return "portalEffect04.plist";
        case GameObjectType::InverseMirrorPortal: return "portalEffect02.plist";
        case GameObjectType::NormalMirrorPortal: return "portalEffect01.plist";
        case GameObjectType::BallPortal: return "portalEffect02.plist";
        case GameObjectType::RegularSizePortal: return "portalEffect08.plist";
        case GameObjectType::MiniSizePortal: return "portalEffect09.plist";
        case GameObjectType::UfoPortal: return "portalEffect02.plist";
        case GameObjectType::DualPortal: return "portalEffect09.plist";
        case GameObjectType::SoloPortal: return "portalEffect09.plist";
        case GameObjectType::WavePortal: return "portalEffect02.plist";
        case GameObjectType::RobotPortal: return "portalEffect02.plist";
        case GameObjectType::SpiderPortal: return "portalEffect02.plist";
        case GameObjectType::SwingPortal: return "portalEffect02.plist";
        case GameObjectType::GravityTogglePortal: return "portalEffect02.plist";
        default: return nullptr;
    }
}

class $modify(PlaytestRingObject, RingObject) {
    void powerOnObject(int state) {
        auto editor = LevelEditorLayer::get();
        if (editor && editor->m_playbackMode == PlaybackMode::Playing) {
            if (m_isRingPoweredOn || s_armed.count(this)) {
                m_isRingPoweredOn = true;
                return;
            }
            m_isRingPoweredOn = true;
            if (m_hasNoEffects) {
                return;
            }
            s_armed.insert(this);
            this->spawnCircle();
            return;
        }
        RingObject::powerOnObject(state);
    }

    void spawnCircle() {
        auto editor = LevelEditorLayer::get();
        if (editor && editor->m_playbackMode == PlaybackMode::Playing) {
            if (m_hasNoEffects) {
                return;
            }
            if (!editor->m_objectLayer) {
                return;
            }
            if (!tryConsumeRingWave(this, false)) {
                return;
            }
            auto wave = CCCircleWave::create(5.f, 55.f, 0.25f, false, true);
            if (!wave) {
                return;
            }
            wave->m_circleMode = CircleMode::Outline;
            wave->m_lineWidth = 2;
            wave->setPosition(this->getPosition());
            wave->followObject(this, false);
            editor->m_objectLayer->addChild(wave, this->getZOrder());
            return;
        }
        RingObject::spawnCircle();
    }
};

class $modify(PlaytestEditorLayer, LevelEditorLayer) {
    struct Fields {
        bool m_origPreviewParticles = false;
        std::vector<Ref<RingObject>> m_rings;
        std::vector<Ref<GameObject>> m_claimed;
        std::vector<Ref<GameObject>> m_particleOwners;
        std::vector<Ref<cocos2d::CCSprite>> m_portalBacks;
    };

    void releaseOwnedParticles() {
        for (auto obj : m_fields->m_particleOwners) {
            if (obj && obj->m_particle) {
                obj->m_particle->removeFromParent();
                obj->m_particle = nullptr;
            }
        }
        m_fields->m_particleOwners.clear();
    }

    void addPortalBacks() {
        if (!m_objects || !m_objectLayer) {
            return;
        }
        for (auto obj : CCArrayExt<GameObject*>(m_objects)) {
            if (!obj || !isPortalType(obj->m_objectType)) {
                continue;
            }
            auto back = GameObject::createWithFrame(portalBackFrame(obj->m_objectID));
            if (!back) {
                continue;
            }
            auto pos = obj->getPosition();
            if (auto parent = obj->getParent()) {
                pos = m_objectLayer->convertToNodeSpace(parent->convertToWorldSpace(pos));
            }
            back->m_objectID = 0x26;
            back->setPosition(pos);
            back->copyGroups(obj);
            m_objectLayer->addChild(back, obj->getZOrder() - 100);
            m_fields->m_portalBacks.push_back(back);
        }
    }

    void removePortalBacks() {
        for (auto sprite : m_fields->m_portalBacks) {
            if (sprite) {
                sprite->removeFromParent();
            }
        }
        m_fields->m_portalBacks.clear();
    }

void triggerGravitySweep(bool flip, bool sideways, cocos2d::ccColor3B color) {
        auto director = CCDirector::sharedDirector();
        auto scene = director->getRunningScene();
        if (!scene) {
            return;
        }
        if (auto old = scene->getChildByID("gravity-effect"_spr)) {
            old->removeFromParent();
        }
        auto sprite = GravityEffectSprite::create();
        if (!sprite) {
            return;
        }
        sprite->setID("gravity-effect"_spr);
        sprite->updateSpritesColor(color);
        auto win = director->getWinSize();
        CCPoint from;
        CCPoint to;
        if (!sideways) {
            sprite->setFlipY(!flip);
            if (flip) {
                from = CCPoint(win.width * 0.5f, -95.f);
                to = CCPoint(win.width * 0.5f, win.height + 95.f);
            } else {
                from = CCPoint(win.width * 0.5f, win.height + 95.f);
                to = CCPoint(win.width * 0.5f, -95.f);
            }
        } else {
            sprite->setRotation(90.f);
            if (flip) {
                from = CCPoint(-95.f, win.height * 0.5f);
                to = CCPoint(win.width + 95.f, win.height * 0.5f);
            } else {
                from = CCPoint(win.width + 95.f, win.height * 0.5f);
                to = CCPoint(-95.f, win.height * 0.5f);
            }
        }
        sprite->setPosition(from);
        scene->addChild(sprite, 100);
        auto move = CCMoveTo::create(0.4f, to);
        auto clean = CCCallFunc::create(sprite, callfunc_selector(CCNode::removeFromParent));
        sprite->runAction(CCSequence::create(move, clean, nullptr));
    }

    void attachParticle(GameObject* obj, const char* plist, bool tint, cocos2d::ccColor3B color,
                        const PortalParticleTint* portalTint = nullptr) {
        if (!obj || !m_objectLayer) {
            return;
        }
        auto p = CCParticleSystemQuad::create(plist, false);
        if (!p) {
            return;
        }
        p->setPosition(obj->getPosition());
        p->setRotation(obj->getRotation());
        p->setScale(obj->getScale());
        p->setPositionType(kCCPositionTypeGrouped);
        obj->m_particle = p;
        if (portalTint) {
            if (portalTint->hasStart) {
                p->setStartColor(portalTint->start);
            }
            if (portalTint->hasEnd) {
                p->setEndColor(portalTint->end);
            }
        }
        if (tint) {
            obj->updateParticleColor(color);
        }
        m_objectLayer->addChild(p, obj->getZOrder() - 1);
        m_fields->m_particleOwners.push_back(obj);
    }

    void onPlaytest() {
        LevelEditorLayer::onPlaytest();
        m_fields->m_origPreviewParticles = m_previewParticles;
        m_previewParticles = true;
        this->updatePreviewParticles();
        m_fields->m_rings.clear();
        for (auto obj : m_fields->m_claimed) {
            if (obj && obj->m_particle) {
                obj->unclaimParticle();
            }
        }
        m_fields->m_claimed.clear();
        this->releaseOwnedParticles();
        this->removePortalBacks();
        s_lastOutline.clear();
        s_lastHit.clear();
        s_armed.clear();
        s_scaleReset.clear();
        s_resetDelay = 0.f;
        if (m_objects) {
            for (auto obj : CCArrayExt<GameObject*>(m_objects)) {
                if (!obj) {
                    continue;
                }
                if (auto ring = typeinfo_cast<RingObject*>(obj)) {
                    ring->m_isActivated = false;
                    ring->m_isRingPoweredOn = false;
                    m_fields->m_rings.push_back(ring);
                }
                if (obj->m_hasParticles && !obj->m_hasNoParticles && !obj->m_particleLocked && !obj->m_particle) {
                    obj->claimParticle();
                    if (obj->m_particle) {
                        m_fields->m_claimed.push_back(obj);
                        continue;
                    }
                }
                if (obj->m_particle) {
                    continue;
                }
                if (auto plist = portalParticlePlist(obj->m_objectType)) {
                    auto tint = portalParticleTint(obj->m_objectType);
                    this->attachParticle(obj, plist, false, cocos2d::ccColor3B{255, 255, 255}, &tint);
                } else if (typeinfo_cast<RingObject*>(obj)) {
                    auto tint = obj->m_objectType == GameObjectType::DropRing
                        ? cocos2d::ccColor3B{255, 255, 255}
                        : getRingColor(obj->m_objectType, obj);
                    this->attachParticle(obj, "ringEffect.plist", true, tint);
                } else if (obj->m_objectType == GameObjectType::YellowJumpPad ||
                           obj->m_objectType == GameObjectType::PinkJumpPad ||
                           obj->m_objectType == GameObjectType::GravityPad ||
                           obj->m_objectType == GameObjectType::RedJumpPad ||
                           obj->m_objectType == GameObjectType::SpiderPad) {
                    this->attachParticle(obj, "bumpEffect.plist", true, getPadColor(obj->m_objectType, obj));
                }
            }
        }
        FMODAudioEngine::sharedEngine()->enableMetering();
        this->addPortalBacks();
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
        LevelEditorLayer::updateVisibility(dt);
        if (m_playbackMode != PlaybackMode::Playing) {
            return;
        }
        float pulse = FMODAudioEngine::sharedEngine()->m_pulse1;
        for (auto const& ref : m_fields->m_rings) {
            auto ring = ref.data();
            if (!ring) {
                continue;
            }
            if (ring->m_unk3F8) {
                continue;
            }
            if (!ring->m_usesAudioScale || ring->m_hasNoAudioScale) {
                continue;
            }
            float v = pulse;
            if (ring->m_customAudioScale) {
                v = ring->m_minAudioScale
                    + (ring->m_maxAudioScale - ring->m_minAudioScale) * (pulse - 0.1f);
            }
            ring->setRScale(std::min(v + 0.3f, 1.2f));
        }
    }

    void updateEditor(float dt) {
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
            s_frame++;
            float pulse = FMODAudioEngine::sharedEngine()->m_pulse1;
            if (m_player1) {
                m_player1->m_audioScale = pulse;
            }
            if (m_player2) {
                m_player2->m_audioScale = pulse;
            }
            for (auto it = s_armed.begin(); it != s_armed.end();) {
                auto ring = *it;
                bool close = false;
                for (auto player : {m_player1, m_player2}) {
                    if (!player || !ring) {
                        continue;
                    }
                    auto a = player->getPosition();
                    auto b = ring->getPosition();
                    float dx = a.x - b.x;
                    float dy = a.y - b.y;
                    if (dx * dx + dy * dy < 6400.f) {
                        close = true;
                        break;
                    }
                }
                if (close) {
                    ++it;
                } else {
                    if (ring) {
                        ring->m_isRingPoweredOn = false;
                    }
                    it = s_armed.erase(it);
                }
            }
        }
        LevelEditorLayer::updateEditor(dt);
        if (gm->m_playLayer == reinterpret_cast<PlayLayer*>(static_cast<GJBaseGameLayer*>(this))) {
            gm->m_playLayer = origPlayLayer;
        }
        if (scoped) {
            gm->m_editorEnabled = origEnabled;
        }
        if (s_resetDelay > 0.f) {
            s_resetDelay -= dt;
            if (s_resetDelay <= 0.f) {
                for (auto ring : s_scaleReset) {
                    if (ring) {
                        ring->m_isActivated = false;
                        ring->m_isRingPoweredOn = false;
                        ring->resetRScaleForced();
                        ring->setRScale(1.f);
                    }
                }
                s_scaleReset.clear();
            }
        }
    }

    void onStopPlaytest() {
        for (auto ring : m_fields->m_rings) {
            if (ring) {
                ring->m_isActivated = false;
                ring->m_isRingPoweredOn = false;
                ring->resetRScaleForced();
                ring->setRScale(1.f);
                s_scaleReset.push_back(ring);
            }
        }
        s_resetDelay = 0.5f;
        m_fields->m_rings.clear();
        for (auto obj : m_fields->m_claimed) {
            if (obj && obj->m_particle) {
                obj->unclaimParticle();
            }
        }
        m_fields->m_claimed.clear();
        this->releaseOwnedParticles();
        this->removePortalBacks();
        s_lastOutline.clear();
        s_lastHit.clear();
        s_armed.clear();
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
        for (auto ring : m_fields->m_rings) {
            if (ring) {
                ring->m_isRingPoweredOn = false;
                ring->resetRScaleForced();
            }
        }
        if (player) {
            player->playDeathEffect();
            player->spawnCircle2();
        }
    }
};

void triggerGravitySweep(bool flip, bool sideways, cocos2d::ccColor3B color) {
    if (auto editor = static_cast<PlaytestEditorLayer*>(LevelEditorLayer::get())) {
        editor->triggerGravitySweep(flip, sideways, color);
    }
}

class $modify(PlaytestGameLayer, GJBaseGameLayer) {
    void toggleDualMode(GameObject* obj, bool dual, PlayerObject* player, bool noEffects) {
        GJBaseGameLayer::toggleDualMode(obj, dual, player, noEffects);
        if (!m_isEditor) {
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
