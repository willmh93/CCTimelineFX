# CCTimelineFX

Cocos2d-x v4 port of TimelineFX particle engine

## Getting Started

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