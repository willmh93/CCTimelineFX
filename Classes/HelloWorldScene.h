#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "TimelineFX/CCTimelineFX.h"

USING_NS_CC;
using namespace cocos2d::ui;

class HelloWorld : public cocos2d::Scene
{
public:

	cocos2d::Size          _canvasSize;
	cocos2d::Vec2          _origin;

	static const char     *_libPaths[];
	static const int       _libCount;

	CCParticleManager     *_particleManager;
	TLFX::EffectsLibrary  *_effectsLibrary;
	TLFX::Effect          *_currentEffect;
	int                    _iCurrentEffect;
	int                    _iCurrentLibrary;
	bool                   _bAutoPlay;
    float                  _timer;

	Button                *_prevEffectBtn;
	Button                *_nextEffectBtn;
	Button                *_prevLibraryBtn;
	Button                *_nextLibraryBtn;
	CheckBox              *_autoPlayChk;
	Label                 *_currEffectLbl;

	//// Methods ////

	HelloWorld() :
		_currentEffect(nullptr),
		_bAutoPlay(true),
		_iCurrentLibrary(0),
		_iCurrentEffect(0),
        _timer(0.0f)
	{}

	bool init() override;
	void cleanup() override;
	void update(float dt) override;

	void InitTLFX();
	void InitUI();
	void LoadNextEffect();
	void SwitchEffect(int i);
	void SwitchLibrary(int i, int iFirstEffect = 0);

	///
	static cocos2d::Scene* createScene();
	CREATE_FUNC(HelloWorld);
};

#endif
