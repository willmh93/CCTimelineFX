#ifndef CCTIMELINEFX_H
#include "CCTimelineFX.h"
#endif

#include "TLFXPugiXMLLoader.h"
#include <cmath>

void TFX_QuadBatch::init()
{
	_dirty = false;
	_texture = nullptr;
	_additive = false;
	_particleCount = 0;
	_blendFunc = cocos2d::BlendFunc::ALPHA_NON_PREMULTIPLIED;

	auto &pipelieDescriptor = _quadCommand.getPipelineDescriptor();
	auto *program = cocos2d::backend::Program::getBuiltinProgram(cocos2d::backend::ProgramType::POSITION_TEXTURE_COLOR);

	_programState = new (std::nothrow) cocos2d::backend::ProgramState(program);
	pipelieDescriptor.programState = _programState;

	_mvpMatrixLocaiton = pipelieDescriptor.programState->getUniformLocation("u_MVPMatrix");
	_textureLocation = pipelieDescriptor.programState->getUniformLocation("u_texture");

	auto vertexLayout = _programState->getVertexLayout();
	const auto& attributeInfo = _programState->getProgram()->getActiveAttributes();
	auto iter = attributeInfo.find("a_position");
	if (iter != attributeInfo.end())
	{
		vertexLayout->setAttribute("a_position", iter->second.location, cocos2d::backend::VertexFormat::FLOAT3, 0, false);
	}
	iter = attributeInfo.find("a_texCoord");
	if (iter != attributeInfo.end())
	{
		vertexLayout->setAttribute("a_texCoord", iter->second.location, cocos2d::backend::VertexFormat::FLOAT2, offsetof(cocos2d::V3F_C4B_T2F, texCoords), false);
	}
	iter = attributeInfo.find("a_color");
	if (iter != attributeInfo.end())
	{
		vertexLayout->setAttribute("a_color", iter->second.location, cocos2d::backend::VertexFormat::UBYTE4, offsetof(cocos2d::V3F_C4B_T2F, colors), true);
	}
	vertexLayout->setLayout(sizeof(cocos2d::V3F_C4B_T2F));
}

void TFX_QuadBatch::destroy()
{
	if (_programState)
		delete _programState;
}

void TFX_QuadBatch::updateIndices()
{
	for (int i = 0; i < _particleCount; ++i)
	{
		const unsigned int i6 = i * 6;
		const unsigned int i4 = i * 4;
		_indices[i6 + 0] = (unsigned short)i4 + 0;
		_indices[i6 + 1] = (unsigned short)i4 + 1;
		_indices[i6 + 2] = (unsigned short)i4 + 2;

		_indices[i6 + 5] = (unsigned short)i4 + 1;
		_indices[i6 + 4] = (unsigned short)i4 + 2;
		_indices[i6 + 3] = (unsigned short)i4 + 3;
	}
}

void TFX_QuadBatch::updateQuads()
{
	cocos2d::Vec2 newPos;
	cocos2d::Vec2 pos = cocos2d::Vec2::ZERO;
	cocos2d::Size texSize = _texture->getContentSizeInPixels();

	cocos2d::V3F_C4B_T2F_Quad* quad = &(_quads[0]);

	float _r, _g, _b, _a;
	for (size_t i = 0; i < _particleCount; ++i, ++quad)
	{
		TFX_QuadData &p = _quadData[i];

		updateParticleQuad(quad, p, texSize.width, texSize.height);

		_r = (int)(p.col.r * 255.0f);
		_g = (int)(p.col.g * 255.0f);
		_b = (int)(p.col.b * 255.0f);
		_a = (int)(p.col.a * 255.0f);

		quad->tl.colors.set(_r, _g, _b, _a);
		quad->tr.colors.set(_r, _g, _b, _a);
		quad->bl.colors.set(_r, _g, _b, _a);
		quad->br.colors.set(_r, _g, _b, _a);
	}
}

void updateParticleQuad(cocos2d::V3F_C4B_T2F_Quad *quad, TFX_QuadData &p, float w, float h)
{
	// Is frame size smaller than texture size? Assume multiple frames
	if (p.fw < w || p.fh < h)
	{
		// Number of frames horizontally / vertically
		int framesX = w / p.fw;
		int framesY = h / p.fh;

		// Frame x/y position (pixels)
		int fx = p.frame % framesX;
		int fy = p.frame / framesX;

		// Frame rect UVs
		float u0 = (fx * p.fw) / w;
		float v0 = (fy * p.fh) / h;
		float u1 = ((fx + 1) * p.fw) / w;
		float v1 = ((fy + 1) * p.fh) / h;
		
		// Set UVs
		quad->tl.texCoords.u = u0;
		quad->tl.texCoords.v = v0;
		quad->tr.texCoords.u = u1;
		quad->tr.texCoords.v = v0;
		quad->bl.texCoords.u = u0;
		quad->bl.texCoords.v = v1;
		quad->br.texCoords.u = u1;
		quad->br.texCoords.v = v1;
		
		// Calculate verts below using the frame size
		w = p.fw;
		h = p.fh;
	}
	else
	{
		// Single frame
		quad->tl.texCoords.u = 0;
		quad->tl.texCoords.v = 0;
		quad->tr.texCoords.u = 1;
		quad->tr.texCoords.v = 0;
		quad->bl.texCoords.u = 0;
		quad->bl.texCoords.v = 1;
		quad->br.texCoords.u = 1;
		quad->br.texCoords.v = 1;
	}

	// Local quad corners, scaled, translated by handle pos
	float x0 = (-p.x * p.sx);
	float y0 = (-p.y * p.sy);
	float x1 = (-p.x + w) * p.sx;
	float y1 = y0;
	float x2 = x0;
	float y2 = (-p.y + h) * p.sy;
	float x3 = x1;
	float y3 = y2;

	float r = -CC_DEGREES_TO_RADIANS(p.rot);
	float cos = cosf(r);
	float sin = sinf(r);

	// Translate by particle pos, apply rotation
	quad->tl.vertices.x = p.px + (x0 * cos - y0 * sin);
	quad->tl.vertices.y = p.py + (x0 * sin + y0 * cos);
	quad->tr.vertices.x = p.px + (x1 * cos - y1 * sin);
	quad->tr.vertices.y = p.py + (x1 * sin + y1 * cos);
	quad->bl.vertices.x = p.px + (x2 * cos - y2 * sin);
	quad->bl.vertices.y = p.py + (x2 * sin + y2 * cos);
	quad->br.vertices.x = p.px + (x3 * cos - y3 * sin);
	quad->br.vertices.y = p.py + (x3 * sin + y3 * cos);
}

TLFX::XMLLoader* CCEffectsLibrary::CreateLoader() const
{
	return new TLFX::PugiXMLLoader(0);
}

TLFX::AnimImage* CCEffectsLibrary::CreateImage() const
{
	return new CCImage();
}

CCImage::CCImage() : _texture(NULL)
{

}

CCImage::~CCImage()
{
	if (_texture)
		delete _texture;
}

bool CCImage::Load(const char *filename)
{
	cocos2d::Image *img = new cocos2d::Image();
	img->initWithImageFile(cocos2d::FileUtils::getInstance()->fullPathForFilename(filename));

	_texture = new cocos2d::Texture2D;
	_texture->initWithImage(img);
	delete img;

	return true;
}

CCParticleManager::CCParticleManager(int particles /*= particleLimit*/, int layers /*= 1*/)
	: TLFX::ParticleManager(particles, layers)
{
	_nullKey = std::make_pair(nullptr, false);
	ClearBatchCache();
}

void CCParticleManager::onEnter()
{
	Node::onEnter();

	// update after action in run!
	this->scheduleUpdateWithPriority(1);
}

void CCParticleManager::onExit()
{
	this->unscheduleUpdate();
	Node::onExit();
}

void CCParticleManager::update(float dt)
{
	ParticleManager::Update();
}

void CCParticleManager::draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)
{
	// Clear previous frame's draw data
	ClearBatchCache();

	// Let TimelineFX draw particles to the batch cache
	DrawParticles();

	// Ensure final batch is pushed to _batches (if only 1 texture)
	Flush();

	for (auto it = _batches.begin(); it != _batches.end(); it++)
	{
		// Grab next batch
		TFX_QuadBatch &b = it->second;

		if (b._particleCount > 0)
		{
            // Update quads, colors, and UVs
            b.updateQuads();

			// Render each batch
			auto programState = b._quadCommand.getPipelineDescriptor().programState;
			programState->setTexture(b._textureLocation, 0, b._texture->getBackendTexture());

			cocos2d::Mat4 projectionMat = cocos2d::Director::getInstance()->getMatrix(cocos2d::MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
			programState->setUniform(b._mvpMatrixLocaiton, projectionMat.m, sizeof(projectionMat.m));

			b._quadCommand.init(0, b._texture, b._blendFunc, &b._quads[0], (ssize_t)b._particleCount, _transform, flags);
			renderer->addCommand(&b._quadCommand);
		}
	}
}

void CCParticleManager::Flush()
{
	// Batch not ready to be flushed if previous key is null (first draw call)
	if (_prevKey == _nullKey)
		return;

	// Process current batch before moving on to next texture
	TFX_QuadBatch &_last = _batches[_prevKey];

	// Update texture indices if batch has grown since last frame
	if (_last._dirty)
	{
		_last.updateIndices();
		_last._dirty = false;
	}
}

void CCParticleManager::ClearBatchCache()
{
	_prevKey = _nullKey;

	// Don't erase the batch cache.
	// Overwrite them on the next frame to avoid reallocating new quads/indicies
	for (auto it = _batches.begin(); it != _batches.end(); it++)
		it->second.clear();
}

void CCParticleManager::DestroyBatchCache()
{
	_prevKey = _nullKey;

	for (auto &b : _batches)
		b.second.destroy();

	_batches.clear();
}

void CCParticleManager::DrawSprite(
	TLFX::AnimImage* sprite,
	float px, float py,
	float frame, float fw, float fh,
	float x, float y, float rotation,
	float scaleX, float scaleY,
	unsigned char r,
	unsigned char g,
	unsigned char b,
	float a,
	bool bAdditive)
{
	CCImage *img = (CCImage*)sprite;
	cocos2d::Texture2D *tex = img->GetTexture();

	// Split batch up if texture changes, or if blend mode changes
	auto key = std::make_pair(tex, bAdditive);

	auto it = _batches.find(key);
	if (it == _batches.end())
	{
		// New texture, create new batch
		_batches[key] = TFX_QuadBatch();
		_batches[key].init();
	}

	if (_prevKey != _nullKey && key != _prevKey)
		Flush();

	_prevKey = key;
	_batches[key].push(
		tex, px, _vpH-py, frame, fw, fh, x, y, rotation, scaleX, -scaleY,
		cocos2d::Color4F(cocos2d::Color3B(r,g,b), a),
		bAdditive
	);
}

void TFX_QuadBatch::push(
	cocos2d::Texture2D *_tex,
	float _px, float _py,
	float _frame, float _fw, float _fh,
	float _x, float _y,
	float _rot,
	float _scaleX, float _scaleY,
	cocos2d::Color4F _col,
	bool _additive)
{
	_texture = _tex;
	_blendFunc = _additive ?
		cocos2d::BlendFunc::ADDITIVE :
		cocos2d::BlendFunc::ALPHA_NON_PREMULTIPLIED;

	TFX_QuadData qd;
	qd.px = _px;
	qd.py = _py;
	qd.frame = (int)_frame;
	qd.fw = _fw; // Frame width
	qd.fh = _fh; // Frame height
	qd.x = _x;
	qd.y = _y;
	qd.rot = _rot;
	qd.sx = _scaleX;
	qd.sy = _scaleY;
	qd.col = _col;

	// Has this batch grown since the previous frame? Resize batch data
	if (_particleCount >= _quadData.size())
	{
		int maxParticles = _particleCount + 1;

		_quadData.resize(maxParticles);
		_quads.resize(maxParticles);
		_indices.resize(maxParticles * 6);

		_dirty = true;
	}

	_quadData[_particleCount] = qd;
	_particleCount++;
}
