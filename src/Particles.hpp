#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

struct PortalParticleTint;

struct ParticleData {
    std::vector<Ref<GameObject>> claimed;
    std::vector<Ref<GameObject>> owners;
    std::vector<Ref<cocos2d::CCParticleSystemQuad>> particles;
};

cocos2d::ccColor3B getPadColor(GameObjectType type, GameObject* obj);
cocos2d::ccColor3B getRingColor(GameObjectType type, GameObject* obj);

void attachParticle(LevelEditorLayer* self, ParticleData& data, GameObject* obj,
                    const char* plist, bool tint, cocos2d::ccColor3B color,
                    const PortalParticleTint* portalTint = nullptr);
void unclaimParticles(LevelEditorLayer* self, ParticleData& data);
void releaseOwnedParticles(LevelEditorLayer* self, ParticleData& data);
void syncOwnedParticles(LevelEditorLayer* self, ParticleData& data);
