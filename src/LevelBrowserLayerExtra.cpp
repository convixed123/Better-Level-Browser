#include "LevelBrowserLayerExtra.hpp"
#include "Geode/cocos/actions/CCActionInterval.h"
#include "Geode/cocos/cocoa/CCArray.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCLayer.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCScene.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/utils/cocos.hpp"
#include <Geode/Enums.hpp>
#include <Geode/binding/AppDelegate.hpp>
#include <Geode/binding/BoomListLayer.hpp>
#include <Geode/binding/BoomListView.hpp>
#include <Geode/binding/CCContentLayer.hpp>
#include <Geode/binding/CustomListView.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/GJListLayer.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/LevelCell.hpp>

LevelBrowserLayerExtra* LevelBrowserLayerExtra::create(GJSearchObject* searchObject) {
    auto ret = new LevelBrowserLayerExtra();
    if (ret->init(searchObject)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

CCScene* LevelBrowserLayerExtra::scene(GJSearchObject* searchObject) {
    auto scene = CCScene::create();
    auto layer = LevelBrowserLayerExtra::create(searchObject);

    AppDelegate::get()->m_runningScene = scene;
    scene->addChild(layer);

    return scene;
}

bool LevelBrowserLayerExtra::init(GJSearchObject* searchObject) {
    if (!CCLayer::init()) return false;

    this->setID("LevelBrowserLayerExtra");

    this->m_searchObject = searchObject;
    this->m_searchObject->retain();

    this->setKeyboardEnabled(true);
    this->setKeypadEnabled(true);

    this->guiMain();
    this->guiList();

    this->loadLevels();

    return true;
}

void LevelBrowserLayerExtra::guiMain() {
    auto winSize = CCDirector::get()->getWinSize();

    auto backButtonMenu = CCMenu::create(nullptr);
    backButtonMenu->setID("back-menu");
    backButtonMenu->setAnchorPoint({0, 0});
    backButtonMenu->setScale(1);
    backButtonMenu->setContentSize({50, 50});
    backButtonMenu->setPosition(-1, winSize.height - 48);
    this->addChild(backButtonMenu, 1);

    auto backBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
        this,
        menu_selector(LevelBrowserLayerExtra::onBack)
    );
    backBtn->setID("back-button");
    backBtn->setPosition({25, 25});
    backButtonMenu->addChild(backBtn);

    auto bg = CCSprite::create("GJ_gradientBG.png");
    bg->setID("background");
    bg->setColor({0, 102, 255});
    bg->setAnchorPoint({0, 0});
    bg->setPosition({0, 0});
    bg->setScaleX((winSize.width + 10) / bg->getTextureRect().size.width);
    bg->setScaleY(winSize.height / bg->getTextureRect().size.height);
    this->addChild(bg, 0);

    auto leftCorner = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
    leftCorner->setID("left-corner");
    leftCorner->setAnchorPoint({0, 0});
    leftCorner->setContentSize({71.5, 71.5});
    leftCorner->setPosition({-1, -1});
    this->addChild(leftCorner, 1);

    auto rightCorner = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
    rightCorner->setID("right-corner");
    rightCorner->setFlipX(true);
    rightCorner->setAnchorPoint({0, 0});
    rightCorner->setContentSize({71.5, 71.5});
    rightCorner->setPosition({(winSize.width - 70.5f), -1});
    this->addChild(rightCorner, 1);

    this->m_infoLabel = CCLabelBMFont::create("0 levels are loaded", "goldFont.fnt");
    this->m_infoLabel->setID("info-label");
    this->m_infoLabel->setPosition({winSize.width - 70, winSize.height - 12});
    this->m_infoLabel->setScale(0.5f);
    this->addChild(this->m_infoLabel);
}

void LevelBrowserLayerExtra::guiList() {
    auto winSize = CCDirector::get()->getWinSize();

    this->m_storedLevels = CCArray::create();
    this->m_storedLevels->retain();

    this->m_listMenu = CCMenu::create();
    this->m_listMenu->setID("list-menu");
    this->m_listMenu->setPosition({0, 0});
    this->addChild(this->m_listMenu);

    this->m_loadingCircle = LoadingCircle::create();
    this->m_loadingCircle->setID("loading-circle");
    this->m_loadingCircle->setPosition({0, 0});
    this->m_loadingCircle->setParentLayer(this);
    this->m_loadingCircle->show();
    this->m_listMenu->addChild(this->m_loadingCircle, 2);
    
    auto listLayer = GJListLayer::create(
        nullptr,
        "Online Levels",
        {0, 0, 0, 0},
        356, 220, 0
    );
    listLayer->setID("list-layer");
    listLayer->setContentSize({356, 175});
	listLayer->setPositionX(winSize.width / 2.f - listLayer->getContentSize().width / 2.f);
	listLayer->setPositionY((winSize.height / 2.f - listLayer->getContentSize().height / 2.f) - 27.5f);
    this->m_listMenu->addChild(listLayer, 1);

    this->m_loadingCircleExt = LoadingCircle::create();
    this->m_loadingCircleExt->setID("loading-circle-extra");
    this->m_loadingCircleExt->setScale(0.5f);
    this->m_loadingCircleExt->setPosition({215.f, -123.f});
    this->m_loadingCircleExt->setVisible(false);
    this->m_loadingCircleExt->setParentLayer(this);
    this->m_loadingCircleExt->show();
    this->m_listMenu->addChild(this->m_loadingCircleExt);
}

void LevelBrowserLayerExtra::loadLevels() {
    if (this->m_loadingInProcess) return;
    this->m_loadingInProcess = true;

    if (!this->m_justOpenned) {
        this->m_loadingCircleExt->setVisible(true);
    }

    if (!this->m_searchObject) {
        this->m_searchObject = GJSearchObject::create(
            SearchType::Search,
            ""
        );
    }

    this->runAction(CCSequence::create(
        CCDelayTime::create(1.f),
        CallFuncExt::create([this](){
            auto glm = GameLevelManager::get();
            glm->m_levelManagerDelegate = this;
            glm->getOnlineLevels(this->m_searchObject->getPageObject(this->m_page));
        }),
        nullptr
    ));
}

void LevelBrowserLayerExtra::updateList() {
    if (auto listViewOld = this->getChildByIDRecursive("list-view")) {
        listViewOld->removeFromParent();
    }

    auto winSize = CCDirector::get()->getWinSize();

    auto listView = CustomListView::create(this->m_storedLevels, BoomListType::Level, 214, 357);
    listView->setID("list-view");
    listView->setPositionX(winSize.width / 2 - listView->getContentSize().width / 2);
	listView->setPositionY((winSize.height / 2 - listView->getContentSize().height / 2) - 2.f);
    this->m_listMenu->addChild(listView, 0);

    this->m_infoLabel->setString(fmt::format("{} levels are loaded", this->m_storedLevels->count()).c_str());

    if (auto contentLayer = this->getListContentLayer()) {
        if (!this->m_justOpenned) {
            contentLayer->setPositionY((contentLayer->getContentHeight() - this->m_listScrollTop) * -1);
            this->m_loadingCircleExt->setVisible(false);
        } else {
            this->m_loadingCircle->setVisible(false);
        }
    }

    this->m_justOpenned = false;
    this->m_loadingInProcess = false;
}

CCContentLayer* LevelBrowserLayerExtra::getListContentLayer() {
    auto listView = this->getChildByIDRecursive("list-view");
    if (!listView) return nullptr;

    auto tableView = listView->getChildByType<TableView>(0);
    if (!tableView) return nullptr;

    auto contentLayer = tableView->getChildByType<CCContentLayer>(0);
    if (!contentLayer) return nullptr;

    return contentLayer;
}

void LevelBrowserLayerExtra::draw() {
    auto contentLayer = this->getListContentLayer();
    if (!contentLayer) return;

    this->m_listScroll = contentLayer->getPositionY();
    this->m_listScrollTop = contentLayer->getContentHeight() + this->m_listScroll;

    if (this->m_listScroll >= 0) {
        if (!this->m_loadingInProcess) {
            ++this->m_page;
            log::info("Loading new levels from page {}...", this->m_page);

            this->loadLevels();
        }
    }
}

void LevelBrowserLayerExtra::loadLevelsFinished(CCArray* levels, const char*) {
    CCArrayExt<GJGameLevel*> levelsExt = levels;
    for (auto lvl : levelsExt) {
        if (!lvl) continue;
        this->m_storedLevels->addObject(lvl);
    }

    this->updateList();
}

void LevelBrowserLayerExtra::loadLevelsFailed(const char* key) {
    this->m_loadingCircle->setVisible(false);
    this->m_loadingCircleExt->setVisible(false);
}

void LevelBrowserLayerExtra::keyBackClicked() {
    this->onBack(nullptr);
}

void LevelBrowserLayerExtra::onBack(CCObject*) {
    CCDirector::get()->popSceneWithTransition(0.5f, kPopTransitionFade);
}

LevelBrowserLayerExtra::~LevelBrowserLayerExtra() {
    this->m_storedLevels->release();
    this->m_searchObject->release();

    GameLevelManager::get()->m_levelManagerDelegate = nullptr;
}