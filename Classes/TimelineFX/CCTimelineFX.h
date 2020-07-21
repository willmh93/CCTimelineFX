#ifndef CCTIMELINEFX_H
#define CCTIMELINEFX_H

#include "TLFXEffectsLibrary.h"
#include "TLFXParticleManager.h"
#include "TLFXEffect.h"
#include "TLFXAnimImage.h"
#include <cocos2d.h>

struct TFX_QuadData;
class CCImage;

void updateParticleQuad(cocos2d::V3F_C4B_T2F_Quad *quad, TFX_QuadData &p, float w, float h);


// Quad transformations for batch draw (for single texture)
struct TFX_QuadData
{
	float px, py;   // Particle X/Y
	float x, y;     // Handle X/Y
	float rot;
	float sx, sy;
	int frame;
	float fw, fh;
	cocos2d::Color4F col;
};

struct TFX_QuadBatch
{
	bool _dirty;
	size_t _particleCount;

	// Quad data auto-freed when batch vector is cleared
	std::vector<TFX_QuadData> _quadData;
	std::vector<cocos2d::V3F_C4B_T2F_Quad> _quads; // Verts, Colors, Tex-Coords
	std::vector<unsigned short> _indices;
	
	cocos2d::Texture2D *_texture;
	bool _additive;

	cocos2d::QuadCommand _quadCommand;
	cocos2d::backend::ProgramState *_programState;
	cocos2d::backend::UniformLocation _mvpMatrixLocaiton;
	cocos2d::backend::UniformLocation _textureLocation;
	cocos2d::BlendFunc _blendFunc;

	void init();
	void destroy();

	void updateIndices();
	void updateQuads();

	void clear() { _particleCount = 0; }
	void push(
		cocos2d::Texture2D *_tex,
		float _px, float _py,
		float _frame, float _fw, float _fh,
		float _x, float _y,
		float _rot,
		float _scaleX, float _scaleY,
		cocos2d::Color4F _col,
		bool _additive);
};

class CCEffectsLibrary : public TLFX::EffectsLibrary
{
public:
	virtual TLFX::XMLLoader* CreateLoader() const;
	virtual TLFX::AnimImage* CreateImage() const;
};

class CCParticleManager : public TLFX::ParticleManager, public cocos2d::Node
{
protected:

	typedef std::pair<cocos2d::Texture2D*, bool> batch_key;

	batch_key _prevKey;
	batch_key _nullKey;

	std::map< batch_key, TFX_QuadBatch > _batches;

	virtual void DrawSprite(
		TLFX::AnimImage* sprite,
		float px, float py,
		float frame, float fw, float fh,
		float x, float y, float rotation,
		float scaleX, float scaleY,
		unsigned char r,
		unsigned char g,
		unsigned char b,
		float a,
		bool bAdditive) override;

public:

	CCParticleManager(
		int particles = TLFX::ParticleManager::particleLimit,
		int layers = 1);

	void onEnter() override;
	void onExit() override;
	void update(float dt) override;
	void draw(
		cocos2d::Renderer *renderer,
		const cocos2d::Mat4 &transform,
		uint32_t flags) override;
	
	void Flush();
	void ClearBatchCache();
	void DestroyBatchCache();

	std::set<TLFX::Effect*>::iterator RemoveEffect(TLFX::Effect* effect) override
	{
		// When effect is destroyed, remove batch map lookup for the textures it contains
		DestroyBatchCache();

		return ParticleManager::RemoveEffect(effect);
	}
};

class CCImage : public TLFX::AnimImage
{
public:
	cocos2d::Texture2D *_texture;

	CCImage();
	~CCImage();

	bool Load(const char *filename);

	cocos2d::Texture2D *GetTexture() const
	{
		return _texture;
	}
};

#endif