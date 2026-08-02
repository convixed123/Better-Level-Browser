#pragma once

#include "Geode/cocos/cocoa/CCArray.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCLayer.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCScene.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include <Geode/Geode.hpp>
#include <Geode/binding/BoomListLayer.hpp>
#include <Geode/binding/CCContentLayer.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/GJListLayer.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/LevelDownloadDelegate.hpp>
#include <Geode/binding/LevelManagerDelegate.hpp>
#include <Geode/binding/LoadingCircle.hpp>
#include <cstddef>

using namespace geode::prelude;

class LevelBrowserLayerExtra : public CCLayer, public LevelManagerDelegate {
private:
    CCMenu* m_reloadMenu;

    LoadingCircle* m_loadingCircle;
    LoadingCircle* m_loadingCircleExt;

    CCMenu* m_listMenu;
    GJListLayer* m_listLayer;
    GJSearchObject* m_searchObject;

    CCLabelBMFont* m_infoLabel;

    CCArray* m_storedLevels;
    size_t m_addedLevelsCount;

    size_t m_page{0};
    bool m_loadingInProcess{false};

    float m_listScroll{-1.f};
    float m_listScrollTop{0.f};

    bool m_justOpenned{true};
private:
    bool init(GJSearchObject* searchObject);

    void draw() override;

    void guiMain();
    void guiList();

    void loadLevels();

    void updateList();

    CCContentLayer* getListContentLayer();

    void loadLevelsFinished(CCArray* levels, const char*) override;
    void loadLevelsFailed(const char*) override;

    void onBack(CCObject*);
    void keyBackClicked() override;

    void onReload(CCObject*);
    void onSoftReload(CCObject*);
public:
    static LevelBrowserLayerExtra* create(GJSearchObject* searchObject);
    static CCScene* scene(GJSearchObject* searchObject);

    ~LevelBrowserLayerExtra();
};