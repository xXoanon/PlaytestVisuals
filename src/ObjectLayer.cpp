#include "ObjectLayer.hpp"

using namespace geode::prelude;

cocos2d::CCPoint layerPosition(cocos2d::CCNode* layer, GameObject* obj) {
    auto pos = obj->getPosition();
    if (auto parent = obj->getParent()) {
        pos = layer->convertToNodeSpace(parent->convertToWorldSpace(pos));
    }
    return pos;
}
