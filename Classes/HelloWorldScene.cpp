#include "HelloWorldScene.h"

// Effect library folders
const char *HelloWorld::_libPaths[] =
{
	"Effects/AurasAndHalos",
	"Effects/BonusBursts",
	"Effects/Environment",
	"Effects/Explosions",
	"Effects/Explosions2",
	"Effects/FireandSmoke",
	"Effects/Flares",
	"Effects/Impacts",
	"Effects/lightshows",
	"Effects/LiquidTextures",
	"Effects/MuzzleFlashes",
	"Effects/pixels",
	"Effects/Readouts",
	"Effects/ShootemUpEffects",
	"Effects/StarBursts",
};

const int HelloWorld::_libCount = sizeof(_libPaths) / sizeof(char*);

bool HelloWorld::init()
{
	if (!Scene::init())
		return false;

	_canvasSize = Director::getInstance()->getVisibleSize();
	_origin = Director::getInstance()->getVisibleOrigin();

	// Init TimelineFX and UI
	this->InitTLFX();
	this->InitUI();

	// Load first library & effect
	this->SwitchLibrary(_iCurrentLibrary, _iCurrentEffect);

	// Start main loop
	scheduleUpdate();

	return true;
}

void HelloWorld::cleanup()
{
	delete _effectsLibrary;
	delete _particleManager;
}

void HelloWorld::update(float dt)
{
	_timer += dt;

	if (_bAutoPlay)
	{
		// Cycle through each library / effect
		if (_timer > 3.0f)
		{
			this->LoadNextEffect();
			return;
		}
	}

	// Always Loop the current effect (if autoplay)
	if (_currentEffect->IsDestroyed() || _timer > 5.0f)
	{
		this->SwitchEffect(_iCurrentEffect);
	}
}

void HelloWorld::InitTLFX()
{
	// TLFX Effects Library
	_effectsLibrary = new CCEffectsLibrary();
	_effectsLibrary->SetUpdateFrequency(60.0f);

	// TLFX Particle Manager (extends cocos2d::Node)
	_particleManager = new CCParticleManager();
	_particleManager->setPosition(_origin.x, _origin.y);
	_particleManager->SetScreenSize((int)_canvasSize.width, (int)_canvasSize.height);
	_particleManager->SetOrigin(0, -50);

	this->addChild(_particleManager);
}

void HelloWorld::InitUI()
{
	_currEffectLbl = Label::create();
	_prevEffectBtn = Button::create("btn_prev.png");
	_nextEffectBtn = Button::create("btn_next.png");
	_prevLibraryBtn = Button::create("btn_prev_lib.png");
	_nextLibraryBtn = Button::create("btn_next_lib.png");
	_autoPlayChk = CheckBox::create("autoplay_back.png", "autoplay_chk.png");
	_prevEffectBtn->setPosition(Vec2(_canvasSize.width * 0.5f - 150, _origin.y + 75));
	_nextEffectBtn->setPosition(Vec2(_canvasSize.width * 0.5f + 150, _origin.y + 75));
	_prevLibraryBtn->setPosition(Vec2(_canvasSize.width * 0.5f - 250, _origin.y + 75));
	_nextLibraryBtn->setPosition(Vec2(_canvasSize.width * 0.5f + 250, _origin.y + 75));
	_currEffectLbl->setPosition(Vec2(_canvasSize.width * 0.5f, _origin.y + 75));
	_currEffectLbl->setAlignment(TextHAlignment::CENTER);
	_currEffectLbl->setOverflow(Label::Overflow::RESIZE_HEIGHT);
	_currEffectLbl->setWidth(200.0f);
	_autoPlayChk->setPosition(Vec2(_canvasSize.width - 140, _origin.y + 75));
	_autoPlayChk->setSelected(this->_bAutoPlay);

	// Event listeners
	_prevEffectBtn->addClickEventListener([this](Ref*) { this->SwitchEffect(_iCurrentEffect - 1); });
	_nextEffectBtn->addClickEventListener([this](Ref*) { this->SwitchEffect(_iCurrentEffect + 1); });
	_prevLibraryBtn->addClickEventListener([this](Ref*) { this->SwitchLibrary(_iCurrentLibrary - 1); });
	_nextLibraryBtn->addClickEventListener([this](Ref*) { this->SwitchLibrary(_iCurrentLibrary + 1); });
	_autoPlayChk->addEventListener([this](Ref*, CheckBox::EventType e)
	{
		this->_bAutoPlay = (e == CheckBox::EventType::SELECTED);
	});

	// Add children
	this->addChild(_currEffectLbl);
	this->addChild(_prevEffectBtn);
	this->addChild(_nextEffectBtn);
	this->addChild(_prevLibraryBtn);
	this->addChild(_nextLibraryBtn);
	this->addChild(_autoPlayChk);
}

// Cycle through all libraries
void HelloWorld::LoadNextEffect()
{
	_timer = 0;

	if (++_iCurrentEffect >= _effectsLibrary->GetEffectCount())
	{
		if (++_iCurrentLibrary >= _libCount)
			_iCurrentLibrary = 0;

		_iCurrentEffect = 0;
		this->SwitchLibrary(_iCurrentLibrary);
	}
	else
	{
		this->SwitchEffect(_iCurrentEffect);
	}
}

// Switch effect for the currently loaded library
void HelloWorld::SwitchEffect(int i)
{
	_timer = 0;

	int effectCount = _effectsLibrary->GetEffectCount();
	if (i < 0)
		i = effectCount - 1;
	else if (i > effectCount - 1)
		i = 0;

	_iCurrentEffect = i;

	// TimelineFX auto destroys effects when no particles left
	if (_currentEffect)
		_currentEffect->HardKill();

	// Load a copy to prevent effects library from being altered
	TLFX::Effect *eff = _effectsLibrary->GetEffect(i);
	_currentEffect = new TLFX::Effect(*eff, _particleManager);
	_currentEffect->DoNotTimeout();
	_particleManager->AddEffect(_currentEffect);

	// Update label
	char name[128];
	sprintf(name, "[%d]\n%s\n\n[%d]\n%s",
		_iCurrentLibrary, _libPaths[_iCurrentLibrary],
		i, _currentEffect->GetPath());

	_currEffectLbl->setString(name);
}

// Switch library and load first effect
void HelloWorld::SwitchLibrary(int i, int iFirstEffect/*=0*/)
{
	if (i < 0)
		i = _libCount - 1;
	else if (i > _libCount - 1)
		i = 0;

	// Clear all running (and dying) effects
	_particleManager->ClearLayer(0);
	
	// Clear current library, load new library
	_iCurrentLibrary = i;
	_effectsLibrary->ClearAll();
	_effectsLibrary->Load(_libPaths[i]);

	// Run first effect
	_currentEffect = nullptr;
	_iCurrentEffect = iFirstEffect;
	this->SwitchEffect(_iCurrentEffect);
}

Scene *HelloWorld::createScene()
{
	return HelloWorld::create();
}
