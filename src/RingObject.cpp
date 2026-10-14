#include <Geode/Geode.hpp>
#include <Geode/modify/RingObject.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

#include "Rings.hpp"

using namespace geode::prelude;

class $modify(PlaytestRingObject, RingObject) {
    void powerOnObject(int state) {
        auto editor = LevelEditorLayer::get();
        if (editor && editor->m_playbackMode == PlaybackMode::Playing) {
            if (m_isRingPoweredOn || ringIsArmed(this)) {
                m_isRingPoweredOn = true;
                return;
            }
            m_isRingPoweredOn = true;
            if (m_hasNoEffects) {
                return;
            }
            armRing(this);
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
            if (!tryConsumeRingWave(this, false)) {
                return;
            }
            if (!editor->m_objectLayer) {
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
            expireRingWave(wave, 0.25f);
            return;
        }
        RingObject::spawnCircle();
    }
};
