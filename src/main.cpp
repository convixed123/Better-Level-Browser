#include "Geode/cocos/CCDirector.h"
#include "Geode/cocos/actions/CCActionInterval.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCTransition.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/cocos/menu_nodes/CCMenuItem.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/utils/cocos.hpp"
#include "LevelBrowserLayerExtra.hpp"
#include <Geode/Enums.hpp>
#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/LevelSearchLayer.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>

using namespace geode::prelude;

class $modify(HookedLevelSearchLayer, LevelSearchLayer) {
    static void onModify(auto& self) {
        (void) self.setHookPriorityPost("LevelSearchLayer::init", Priority::Last);
    }

    struct Fields {
        CCMenuItemToggler* m_betterSearchToggler;
        CCMenuItemSpriteExtra* m_searchBtnExtra;
    };

    bool init(int type) {
        if (!LevelSearchLayer::init(type)) return false;

        auto filterMenu = this->getChildByIDRecursive("other-filter-menu");
        if (!filterMenu) return true;

        auto betterSearchTogglerMenu = CCMenu::create();
        betterSearchTogglerMenu->setID("better-search-toggler-menu");
        betterSearchTogglerMenu->setContentSize({40, 40});
        filterMenu->addChild(betterSearchTogglerMenu);
        filterMenu->updateLayout();

        auto betterSearchLabel = CCLabelBMFont::create("Better Browser", "goldFont.fnt");
        betterSearchLabel->setID("better-search-label");
        betterSearchLabel->setScale(0.25f);
        betterSearchLabel->setPosition({filterMenu->getContentWidth() / 2.f, 40.f});
        betterSearchTogglerMenu->addChild(betterSearchLabel, 1);

        this->m_fields->m_betterSearchToggler = CCMenuItemToggler::create(
            CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
            CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
            this,
            menu_selector(HookedLevelSearchLayer::onBetterSearchToggler)
        );
        this->m_fields->m_betterSearchToggler->setID("better-search-toggler");
        this->m_fields->m_betterSearchToggler->setPosition({filterMenu->getContentWidth() / 2.f, 20.f});
        betterSearchTogglerMenu->addChild(this->m_fields->m_betterSearchToggler, 0);

        auto searchMenu = this->getChildByIDRecursive("search-button-menu");
        if (!searchMenu) return true;

        auto searchBtn = static_cast<CCMenuItemSpriteExtra*>(searchMenu->getChildByID("search-level-button"));
        if (!searchBtn) return true;

        this->m_fields->m_searchBtnExtra = CCMenuItemSpriteExtra::create(
            searchBtn->m_pNormalImage,
            this,
            menu_selector(HookedLevelSearchLayer::onSearchExtra)
        );
        this->m_fields->m_searchBtnExtra->setID("search-level-button-extra");
        this->m_fields->m_searchBtnExtra->setPosition(searchBtn->getPosition());
        this->m_fields->m_searchBtnExtra->setZOrder(searchBtn->getZOrder());
        searchMenu->addChild(this->m_fields->m_searchBtnExtra);

        if (Mod::get()->getSavedValue<bool>("better-search-enabled", true)) {
            this->m_fields->m_betterSearchToggler->activate();
        }

        this->updateSearchBtn();

        return true;
    }

    void updateSearchBtn() {
        auto searchMenu = this->getChildByIDRecursive("search-button-menu");
        if (!searchMenu) return;

        auto searchBtn = static_cast<CCMenuItemSpriteExtra*>(searchMenu->getChildByID("search-level-button"));
        if (!searchBtn) return;

        if (this->m_fields->m_betterSearchToggler->isToggled()) {
            searchBtn->setVisible(false);
            this->m_fields->m_searchBtnExtra->setVisible(true);
        } else {
            searchBtn->setVisible(true);
            this->m_fields->m_searchBtnExtra->setVisible(false);
        }
    }

    void onBetterSearchToggler(CCObject*) {
        this->runAction(CCSequence::create(
            CCDelayTime::create(0.f),
            CallFuncExt::create([this](){
                this->updateSearchBtn();
                Mod::get()->setSavedValue<bool>("better-search-enabled", this->m_fields->m_betterSearchToggler->isToggled());
            }),
            nullptr
        ));
    }

    void onSearchExtra(CCObject*) {
        auto searchObject = this->getSearchObject(SearchType::Search, this->m_searchInput->getString());
        auto scene = LevelBrowserLayerExtra::scene(searchObject);
        CCDirector::get()->pushScene(CCTransitionFade::create(0.5f, scene));
    }
};