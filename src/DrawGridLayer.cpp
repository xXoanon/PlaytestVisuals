#include <Geode/Geode.hpp>
#include <Geode/modify/DrawGridLayer.hpp>

using namespace geode::prelude;

class $modify(PlaytestDrawGridLayer, DrawGridLayer) {
    void draw() {
        cocos2d::CCNode* layers[2] = {m_objectLayer, this->getParent()};
        cocos2d::CCNode* unflipped[2] = {nullptr, nullptr};
        float oldScale[2] = {0.f, 0.f};
        float oldPos[2] = {0.f, 0.f};
        int count = 0;
        float winW = 0.f;
        bool sized = false;
        for (auto* layer : layers) {
            if (!layer || layer->getScaleX() >= 0.f) {
                continue;
            }
            bool done = false;
            for (int i = 0; i < count; i++) {
                if (unflipped[i] == layer) {
                    done = true;
                    break;
                }
            }
            if (done) {
                continue;
            }
            if (!sized) {
                winW = cocos2d::CCDirector::sharedDirector()->getWinSize().width;
                sized = true;
            }
            unflipped[count] = layer;
            oldScale[count] = layer->getScaleX();
            oldPos[count] = layer->getPositionX();
            layer->setScaleX(-oldScale[count]);
            layer->setPositionX(winW - oldPos[count]);
            count++;
        }
        DrawGridLayer::draw();
        for (int i = count - 1; i >= 0; i--) {
            unflipped[i]->setScaleX(oldScale[i]);
            unflipped[i]->setPositionX(oldPos[i]);
        }
    }
};
