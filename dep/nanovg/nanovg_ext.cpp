#include "nanovg.h"
extern "C"
{
	#include "nanovg.c"
	#include "nanovg_gl.h"
}

#include <assets/AssetManager.h>
#include <assets/BitmapFont.h>
#include <renderer/util/TextDrawer.h>

using VecType = std::vector<AssetHandle<BitmapFont>>;

float nvgBitmapText(NVGcontext* ctx, AssetHandle<BitmapFont> bfont, int alig, float x, float y, 
	const char* string)
{
	TextDrawer::Alignment alignment = (TextDrawer::Alignment)alig;
	VecType* vec = (VecType*)ctx->ext_ctx.fonts;
	BitmapFont* font = bfont.data;
	vec->push_back(std::move(bfont));	

	NVGstate* state = nvg__getState(ctx);
	float scale = nvg__getFontScale(state);
	scale = 1.0f;
	float invscale = 1.0f / scale;
	
	auto[glyphs, x_off] = TextDrawer::get_glyphs_and_size(std::string(string), font, glm::vec2(scale, scale)); 
	glm::vec2 npos = glm::vec2(x, y);

	if (alignment == TextDrawer::CENTER)
	{
		npos.x -= x_off * 0.5f;
	}
	else if(alignment == TextDrawer::RIGHT)
	{
		npos.x -= x_off;
	}

	float iwidth = (float)font->img->get_width();
	float iheight = (float)font->img->get_height();

	const char* end = string + strlen(string);
	int cverts = nvg__maxi(2, (int)(end - string)) * 6; // conservative estimate.
	NVGvertex* verts = nvg__allocTempVerts(ctx, cverts);
	int nverts = 0;

	for(size_t i = 0; i < glyphs.size(); i++)
	{
		BitmapFont::Glyph gl = glyphs[i];

		glm::vec2 tex_off = glm::vec2((float)gl.x / iwidth, (float)gl.y / iheight);
		glm::vec2 tex_size = glm::vec2((float)gl.width / iwidth, (float)gl.height / iheight);
		glm::vec2 size = glm::vec2(gl.width * scale, gl.height * scale);
		glm::vec2 offset = glm::vec2(gl.xoffset * scale, gl.yoffset * scale);


		glm::vec4 q = glm::vec4(npos.x + offset.x, npos.y + offset.y, npos.x + size.x + offset.x, npos.y + size.y + offset.y);
		glm::vec4 t = glm::vec4(tex_off.x, tex_off.y, tex_off.x + tex_size.x, tex_off.y + tex_size.y);


		// Same as nvgText
		float c[4*2];
		// Vertices of the quad
		nvgTransformPoint(&c[0],&c[1], state->xform, q.x*invscale, q.y*invscale);
		nvgTransformPoint(&c[2],&c[3], state->xform, q.z*invscale, q.y*invscale);
		nvgTransformPoint(&c[4],&c[5], state->xform, q.z*invscale, q.w*invscale);
		nvgTransformPoint(&c[6],&c[7], state->xform, q.x*invscale, q.w*invscale);
		// Create triangles
		if (nverts+6 <= cverts) 
		{
			nvg__vset(&verts[nverts], c[0], c[1], t.x, t.y); nverts++;
			nvg__vset(&verts[nverts], c[4], c[5], t.z, t.w); nverts++;
			nvg__vset(&verts[nverts], c[2], c[3], t.z, t.y); nverts++;
			nvg__vset(&verts[nverts], c[0], c[1], t.x, t.y); nverts++;
			nvg__vset(&verts[nverts], c[6], c[7], t.x, t.w); nverts++;
			nvg__vset(&verts[nverts], c[4], c[5], t.z, t.w); nverts++;
		}
		
		npos.x += (float)gl.xadvance * scale;

	}

	NVGpaint paint = state->fill;
	paint.image = nvglCreateImageFromHandleGL3(ctx, font->img->id, font->img->get_width(), font->img->get_height(), 0);
	paint.innerColor.a *= state->alpha;
	paint.outerColor.a *= state->alpha;
	
	NVGcompositeOperationState comp = state->compositeOperation;
	ctx->params.renderTriangles(ctx->params.userPtr, &paint, comp, &state->scissor, verts, nverts, ctx->fringeWidth);


	ctx->drawCallCount++;
	ctx->textTriCount += nverts/3;

	return 0.0f;
}

void nvgCreateExt(NVGcontext* ctx)
{
	ctx->ext_ctx.fonts = (void*)(new VecType());
}

void nvgEndFrameExt(NVGcontext* ctx)
{
	VecType* vec = (VecType*)ctx->ext_ctx.fonts;

	vec->clear();
}


