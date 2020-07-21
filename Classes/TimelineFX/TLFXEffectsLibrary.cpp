#include "TLFXEffectsLibrary.h"
#include "TLFXEffect.h"
#include "TLFXEmitter.h"
#include "TLFXAnimImage.h"
#include <CCFileUtils.h>
#include <ZipUtils.h>
#include <cassert>

namespace TLFX
{

const float EffectsLibrary::globalPercentMin        = 0;
const float EffectsLibrary::globalPercentMax        = 20.0f;
const float EffectsLibrary::globalPercentSteps      = 100.0f;

const float EffectsLibrary::globalPercentVMin       = 0;
const float EffectsLibrary::globalPercentVMax       = 10.0f;
const float EffectsLibrary::globalPercentVSteps     = 200.0f;

const float EffectsLibrary::angleMin                = 0;
const float EffectsLibrary::angleMax                = 1080.0f;
const float EffectsLibrary::angleSteps              = 54.0f;

const float EffectsLibrary::emissionRangeMin        = 0;
const float EffectsLibrary::emissionRangeMax        = 180.0f;
const float EffectsLibrary::emissionRangeSteps      = 30.0f;

const float EffectsLibrary::dimensionsMin           = 0;
const float EffectsLibrary::dimensionsMax           = 200.0f;
const float EffectsLibrary::dimensionsSteps         = 40.0f;

const float EffectsLibrary::lifeMin                 = 0;
const float EffectsLibrary::lifeMax                 = 100000.0f;
const float EffectsLibrary::lifeSteps               = 200.0f;

const float EffectsLibrary::amountMin               = 0;
const float EffectsLibrary::amountMax               = 2000.f;
const float EffectsLibrary::amountSteps             = 100.f;

const float EffectsLibrary::velocityMin             = 0;
const float EffectsLibrary::velocityMax             = 10000.0f;
const float EffectsLibrary::velocitySteps           = 100.0f;

const float EffectsLibrary::velocityOverTimeMin     = -20.0f;
const float EffectsLibrary::velocityOverTimeMax     = 20.0f;
const float EffectsLibrary::velocityOverTimeSteps   = 200.f;

const float EffectsLibrary::weightMin               = -2500.0f;
const float EffectsLibrary::weightMax               = 2500.0f;
const float EffectsLibrary::weightSteps             = 200.0f;

const float EffectsLibrary::weightVariationMin      = 0;
const float EffectsLibrary::weightVariationMax      = 2500.0f;
const float EffectsLibrary::weightVariationSteps    = 250.0f;

const float EffectsLibrary::spinMin                 = -2000.0f;
const float EffectsLibrary::spinMax                 = 2000.0f;
const float EffectsLibrary::spinSteps               = 100.0f;

const float EffectsLibrary::spinVariationMin        = 0;
const float EffectsLibrary::spinVariationMax        = 2000.0f;
const float EffectsLibrary::spinVariationSteps      = 100.0f;

const float EffectsLibrary::spinOverTimeMin         = -20.0f;
const float EffectsLibrary::spinOverTimeMax         = 20.0f;
const float EffectsLibrary::spinOverTimeSteps       = 200.0f;

const float EffectsLibrary::directionOverTimeMin    = 0;
const float EffectsLibrary::directionOverTimeMax    = 4320.0f;
const float EffectsLibrary::directionOverTimeSteps  = 216.0f;

const float EffectsLibrary::framerateMin            = 0;
const float EffectsLibrary::framerateMax            = 200.0f;
const float EffectsLibrary::framerateSteps          = 100.0f;

const float EffectsLibrary::maxDirectionVariation   = 22.5f;
const float EffectsLibrary::maxVelocityVariation    = 30.0f;
const int   EffectsLibrary::motionVariationInterval = 30;

#ifdef _DEBUG
int EffectsLibrary::particlesCreated = 0;
#endif


float EffectsLibrary::_updateFrequency           = 30.0f;                  //  times per second
float EffectsLibrary::_updateTime                = 1000.0f / EffectsLibrary::_updateFrequency;
float EffectsLibrary::_currentUpdateTime         = EffectsLibrary::_updateFrequency;
float EffectsLibrary::_lookupFrequency           = EffectsLibrary::_updateTime;
float EffectsLibrary::_lookupFrequencyOverTime   = 1.0f;


EffectsLibrary::EffectsLibrary()
{

}

EffectsLibrary::~EffectsLibrary()
{
    ClearAll();
}

void NormalizePath(char *p)
{
	while (*p)
	{
		if (*p == '\\')
			*p = '/';
		p++;
	}
}

#ifndef MAX_PATH
#define MAX_PATH PATH_MAX
#endif
std::string FixLocalFilepath(const char *_imgPath, const char *_xmlPath)
{
	char imgPath[MAX_PATH];
	char xmlPath[MAX_PATH];
	strcpy(imgPath, _imgPath);
	strcpy(xmlPath, _xmlPath);
	NormalizePath(imgPath);
	NormalizePath(xmlPath);

	char *imgName = imgPath;
	char *p = imgPath;

	do
	{
		p = strstr(p, "/");
		if (p)
			imgName = ++p;
	} while (p);

	p = &xmlPath[strlen(xmlPath) - 1];
	while (p >= xmlPath)
	{
		if (*p == '/')
		{
			*p = 0;
			break;
		}
		p--;
	}

	char ret[MAX_PATH];
	sprintf(ret, "%s/%s", xmlPath, imgName);
	return ret;
}

bool IsDirectoryPath(const char *path)
{
	char *p = (char*)&path[strlen(path) - 1];
	while (p != path)
	{
		if (*p == '/' || *p == '\\')
			return true;
		if (*p == '.')
			return false;
		p--;
	}
	return true;
}

std::string UnzipEff(const char *zipPath, const char *dstFolderName)
{
	cocos2d::ZipFile *zFile = new cocos2d::ZipFile(zipPath);

	std::string fileName = zFile->getFirstFilename();
	std::string file = fileName;

	ssize_t filesize;
	unsigned char* data = zFile->getFileData(fileName, &filesize);

	std::string directoryName = cocos2d::FileUtils::getInstance()->getWritablePath() + dstFolderName;

	if (IsDirectoryPath(directoryName.c_str()) && !cocos2d::FileUtils::getInstance()->isDirectoryExist(directoryName))
	{
		cocos2d::FileUtils::getInstance()->createDirectory(directoryName);
	}

	while (data != nullptr)
	{
		std::string fullFileName = directoryName + "/" + file;

		if (IsDirectoryPath(fullFileName.c_str()) && !cocos2d::FileUtils::getInstance()->isDirectoryExist(fullFileName))
		{
			cocos2d::FileUtils::getInstance()->createDirectory(fullFileName);
		}

		FILE *fp = fopen(fullFileName.c_str(), "wb");

		if (fp)
		{
			fwrite(data, 1, filesize, fp);
			fclose(fp);
		}
		free(data);
		fileName = zFile->getNextFilename();
		file = fileName;

		data = zFile->getFileData(fileName, &filesize);
	}

	return directoryName;
}

bool EffectsLibrary::Load( const char *_filename, bool compile /*= true*/ )
{
	std::string xmlPath;

	if (strstr(_filename, ".eff"))
	{
		// Windows only since APK doesn't allow zipped assets
		char zipPath[MAX_PATH];
		strcpy(zipPath, cocos2d::FileUtils::getInstance()->fullPathForFilename(_filename).c_str());

		char dstFolderName[MAX_PATH];
		strcpy(dstFolderName, _filename);
		*strstr(dstFolderName, ".eff") = 0;

		// Unzip first
		xmlPath = UnzipEff(zipPath, dstFolderName);
		xmlPath += "/data.xml";
	}
	else if (IsDirectoryPath(_filename))
	{
		xmlPath = _filename;
		if (xmlPath.back() != '/')
			xmlPath += '/';
		xmlPath += "data.xml";
		xmlPath = cocos2d::FileUtils::getInstance()->fullPathForFilename(xmlPath);
	}
	else // XML path passed directly
	{
		xmlPath = cocos2d::FileUtils::getInstance()->fullPathForFilename(_filename);
	}

	const char *filename = xmlPath.c_str();

    XMLLoader *loader = CreateLoader();
    bool loaded;
    if ((loaded = loader->Open(filename)))
    {
        AnimImage *shape;
        while ((shape = CreateImage()), loader->GetNextShape(shape))
        {
			// Fix path
			shape->SetFilename(FixLocalFilepath(shape->GetFilename(), filename).c_str());

            AddSprite(shape);
        }
        delete shape;               // last even shape is safe to delete

        // try to locate an effect in xml doc
        loader->LocateEffect();

        Effect *effect;
        while ((effect = loader->GetNextEffect(_shapeList)))
        {
            if (compile)
                effect->CompileAll();

            AddEffect(effect);
            // ??? effect->NewDirectory();
            // ??? effect->AddEffect(effect);
        }


        // try to locate a super effect in xml doc
        loader->LocateSuperEffect();

        Effect *superEffect;
        while ((superEffect = loader->GetNextSuperEffect(_shapeList)))
        {
            if (compile)
                superEffect->CompileAll();

            AddSuperEffect(superEffect);
        }

        _name = filename;
    }

    delete loader;
    return loaded;
}

void EffectsLibrary::AddSuperEffect(Effect *effect)
{
    std::string name = effect->GetPath();

    auto old = _effects.find(name);
    if (old != _effects.end())
    {
        delete old->second;
        // no need to erase, we are assigning new one immediately
    }

    _effects[name] = effect;

    /*auto effects = e->GetEffects();
    for (auto it = effects.begin(); it != effects.end(); ++it)
    {
        AddEffect(static_cast<Effect*>(*it));
    }*/

}

void EffectsLibrary::AddEffect( Effect *e )
{
    std::string name = e->GetPath();

    auto old = _effects.find(name);
    if (old != _effects.end())
    {
        delete old->second;
        // no need to erase, we are assigning new one immediately
    }

    _effects[name] = e;

    auto emitters = e->GetChildren();

	// CCTimelineFX Bug fix: Some library effects have emitters with the same name,
	// which causes a memory overwrite issue. Rename duplicates so each emitter has
	// a unique name. (e.g. ShootemUpEffects.eff, Deep Space Explosion)
	char renameBuf[256];
	for (auto it = emitters.begin(); it != emitters.end(); ++it)
	{
		int renameAttempts = 1;
		bool duplicate;
		
		do
		{
			duplicate = false;

			// Scan emitters that come *before* this one to see if 'it' is a duplicate
			for (auto it2 = emitters.begin(); it2 != it; ++it2)
			{
				if (strcmp((*it)->GetName(), (*it2)->GetName()) == 0)
				{
					duplicate = true;
					break;
				}
			}

			auto *emitter = static_cast<Emitter*>(*it);
			if (duplicate)
			{
				// Give new name
				sprintf(renameBuf, "%s%d", emitter->GetName(), renameAttempts++);
				emitter->SetName(renameBuf);
			}

			// Update path with parent's name (which may have been a duplicate)
			std::string path = e->GetPath();
			path = path + "/" + emitter->GetName();
			emitter->SetPath(path.c_str());
		}
		while (duplicate);
	}

    for (auto it = emitters.begin(); it != emitters.end(); ++it)
    {
        AddEmitter(static_cast<Emitter*>(*it));
    }
}

void EffectsLibrary::AddEmitter( Emitter *e )
{
    std::string name = e->GetPath();

    auto old = _emitters.find(name);
    if (old != _emitters.end())
    {
        delete old->second;
        // no need to erase, we are assigning new one immediately
    }

    _emitters[name] = e;

    auto effects = e->GetEffects();

	// CCTimelineFX Bug fix: Some library effects have effects with the same name,
	// which causes a memory overwrite issue. Rename duplicates so each effect has
	// a unique name. (e.g. explosions.eff, Mushroom 2/Explosion/Fireball Explosion Variation 11)
	char renameBuf[256];
	for (auto it = effects.begin(); it != effects.end(); ++it)
	{
		int renameAttempts = 1;
		bool duplicate;

		do
		{
			duplicate = false;

			// Scan emitters that come *before* this one to see if 'it' is a duplicate
			for (auto it2 = effects.begin(); it2 != it; ++it2)
			{
				if (strcmp((*it)->GetName(), (*it2)->GetName()) == 0)
				{
					duplicate = true;
					break;
				}
			}

			auto *effect = static_cast<Effect*>(*it);
			if (duplicate)
			{
				// Give new name
				sprintf(renameBuf, "%s%d", effect->GetName(), renameAttempts++);
				effect->SetName(renameBuf);
			}

			// Update path with parent's name (which may have been a duplicate)
			std::string path = e->GetPath();
			path = path + "/" + effect->GetName();
			effect->SetPath(path.c_str());
			
		} while (duplicate);
	}

    for (auto it = effects.begin(); it != effects.end(); ++it)
    {
        AddEffect(*it);
    }
}

void EffectsLibrary::ClearAll()
{
    _name = "";

    for (auto it = _effects.begin(); it != _effects.end(); ++it)
        delete it->second;
    _effects.clear();

    for (auto it = _emitters.begin(); it != _emitters.end(); ++it)
        delete it->second;
    _emitters.clear();

    for (auto it = _shapeList.begin(); it != _shapeList.end(); ++it)
        delete *it;
    _shapeList.clear();
}

Effect* EffectsLibrary::GetEffect( const char *name ) const
{
    auto effect = _effects.find(name);
    if (effect != _effects.end())
        return effect->second;
    return NULL;
}


TLFX::Effect* EffectsLibrary::GetEffect(int index) const
{
	int i = 0;
	for (auto it = _effects.begin(); it != _effects.end(); it++)
	{
		// Don't return sub-effects
		if (it->second->GetParentEmitter() != NULL)
			continue;

		if (i++ == index)
			return it->second;
	}
	return NULL;
}


int EffectsLibrary::GetEffectCount()
{
	int ret = 0;
	for (auto it = _effects.begin(); it != _effects.end(); it++)
	{
		// Don't add sub-effects
		if (it->second->GetParentEmitter() != NULL)
			continue;

		ret++;
	}

	return ret;
}

Emitter* EffectsLibrary::GetEmitter( const char *name ) const
{
    auto emitter = _emitters.find(name);
    if (emitter != _emitters.end())
        return emitter->second;
    return NULL;
}

void EffectsLibrary::SetUpdateFrequency( float freq )
{
    _updateFrequency = freq;
    _updateTime = 1000.f / _updateFrequency;
    _currentUpdateTime = _updateFrequency;
}

float EffectsLibrary::GetUpdateFrequency()
{
    return _updateFrequency;
}

float EffectsLibrary::GetUpdateTime()
{
    return _updateTime;
}

float EffectsLibrary::GetCurrentUpdateTime()
{
    return _currentUpdateTime;
}

void EffectsLibrary::SetLookupFrequency( float freq )
{
    _lookupFrequency = freq;
}

float EffectsLibrary::GetLookupFrequency()
{
    return _lookupFrequency;
}

void EffectsLibrary::SetLookupFrequencyOverTime( float freq )
{
    _lookupFrequencyOverTime = freq;
}

float EffectsLibrary::GetLookupFrequencyOverTime()
{
    return _lookupFrequencyOverTime;
}

bool EffectsLibrary::AddSprite( AnimImage *sprite )
{
	char *filename = (char*)sprite->GetFilename();

    if (strlen(sprite->GetName()) == 0)
    {
        const char *name = strrchr(filename, '/');
        if (name)
            ++name;                         // char right after /
        else
            name = filename;
        sprite->SetName(name);
    }

    if (!sprite->Load(cocos2d::FileUtils::getInstance()->fullPathForFilename(filename).c_str()))
        return false;

    _shapeList.push_back(sprite);
    return true;
}

} // namespace TLFX
