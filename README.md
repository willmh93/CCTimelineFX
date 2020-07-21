# CCTimelineFX

Cocos2d-x v4 port of the [TimelineFX](https://www.rigzsoft.co.uk/timelinefx-particle-effects-editor/) particle engine

## Basic Usage

##### Initialization
```cpp
// Load Library
_effectsLibrary = new CCEffectsLibrary();
_effectsLibrary->SetUpdateFrequency(60.0f);
_effectsLibrary->Load("Effects/AurasAndHalos");

// Create Particle Manager
_particleManager = new CCParticleManager();
auto size = Director::getInstance()->getVisibleSize();
_particleManager->SetScreenSize(size.width, size.height);

addChild(_particleManager);

// Grab effect from library, take a copy
TLFX::Effect *eff = _effectsLibrary->GetEffect("Halos/Multi colour");
_currentEffect = new TLFX::Effect(*eff, _particleManager); // Make copy
_currentEffect->DoNotTimeout(); // Handle killing effect ourself
_particleManager->AddEffect(_currentEffect);
```

##### Cleanup
```cpp
delete _effectsLibrary;
delete _particleManager;
```


##### Killing an Effect
```cpp
// To instantly remove effect, use:
_currentEffect->HardKill();

// To stop effect creating more particles, use:
_currentEffect->SoftKill();

```

## Sample Project Setup

Clone the repository and initialize the cocos2d submodule:

```
git clone https://github.com/willmh93/CCTimelineFX.git

cd CCTimelineFX
git submodule update --init

cd cocos2d
python download-deps.py
```

###### Running on Windows

```
cd ..
mkdir win32-build
cd win32-build
cmake .. -G"Visual Studio 15 2017" -Tv141
```

Open **CCTimelineFX.sln**, right-click **CCTimelineFX** in the Solution Explorer, click **Set as StartUp Project**, and run.

### Prerequisites

Ensure your environment is set up for Cocos2d-x. Tested with [Python 2.7.16](https://www.python.org/downloads/release/python-2716/) and [CMake 3.17.3](https://cmake.org/download/). 