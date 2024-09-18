#ifdef NANOVG_D3D9_IMPLEMENTATION
#include <d3dx9math.h>
#include "nanovg_d3d9.hpp"

static int D3D9nvg__convertPaint(struct D3D9NVGcontext* D3D, struct D3D9NVGfragUniforms* frag,
	struct NVGpaint* paint, NVGcompositeOperationState compositeOperation, struct NVGscissor* scissor,
	float width, float fringe, float strokeThr)
{
	struct D3D9NVGtexture* tex = NULL;
	float invxform[6], paintMat[9], scissorMat[9];

	memset(frag, 0, sizeof(*frag));

	frag->innerCol = D3D9nvg__premulColor(paint->innerColor);
	frag->outerCol = D3D9nvg__premulColor(paint->outerColor);

	//TODO: Use compositeOperation

	if (scissor->extent[0] < -0.5f || scissor->extent[1] < -0.5f)
	{
		frag->scissorExt[0] = 1.0f;
		frag->scissorExt[1] = 1.0f;
		frag->scissorScale[0] = 1.0f;
		frag->scissorScale[1] = 1.0f;
	}
	else
	{
		nvgTransformInverse(invxform, scissor->xform);
		D3D9nvg__xformToMat3x3(scissorMat, invxform);
		D3D9nvg_copyMatrix3to4(frag->scissorMat, scissorMat);
		frag->scissorExt[0] = scissor->extent[0];
		frag->scissorExt[1] = scissor->extent[1];
		frag->scissorScale[0] = sqrtf(scissor->xform[0] * scissor->xform[0] + scissor->xform[2] * scissor->xform[2]) / fringe;
		frag->scissorScale[1] = sqrtf(scissor->xform[1] * scissor->xform[1] + scissor->xform[3] * scissor->xform[3]) / fringe;
	}

	frag->extent[0] = paint->extent[0];
	frag->extent[1] = paint->extent[1];

	frag->strokeMult[0] = (width * 0.5f + fringe * 0.5f) / fringe;
	frag->strokeMult[1] = strokeThr;

	if (paint->image != 0)
	{
		tex = D3D9nvg__findTexture(D3D, paint->image);
		if (tex == NULL)
		{
			return 0;
		}

		if ((tex->flags & NVG_IMAGE_FLIPY) != 0)
		{
			float flipped[6];
			nvgTransformScale(flipped, 1.0f, -1.0f);
			nvgTransformMultiply(flipped, paint->xform);
			nvgTransformInverse(invxform, flipped);
		}
		else
		{
			nvgTransformInverse(invxform, paint->xform);
		}
		frag->type = NSVG_SHADER_FILLIMG;

		if (tex->type == NVG_TEXTURE_RGBA)
		{
			frag->texType = (tex->flags & NVG_IMAGE_PREMULTIPLIED) ? 0.0f : 1.0f;
		}
		else
		{
			frag->texType = 2.0f;
		}
	}
	else
	{
		frag->type = NSVG_SHADER_FILLGRAD;
		frag->radius = paint->radius;
		frag->feather = paint->feather;
		nvgTransformInverse(invxform, paint->xform);
	}

	D3D9nvg__xformToMat3x3(paintMat, invxform);
	D3D9nvg_copyMatrix3to4(frag->paintMat, paintMat);

	return 1;
}

static struct D3D9NVGfragUniforms* nvg__fragUniformPtr(struct D3D9NVGcontext* D3D, int i);

void D3D9nvg__setUniforms(struct D3D9NVGcontext* D3D, int uniformOffset, int image)
{
	// Pixel shader constants
	struct D3D9NVGfragUniforms* frag = nvg__fragUniformPtr(D3D, uniformOffset);
	IDirect3DDevice9_SetPixelShaderConstantF(D3D->pDevice, 0, &frag->uniformArray[0][0], NANOVG_D3D9_UNIFORMARRAY_SIZE);

	if (image != 0)
	{
		struct D3D9NVGtexture* tex = D3D9nvg__findTexture(D3D, image);
		if (tex != NULL)
		{
			IDirect3DDevice9_SetTexture(D3D->pDevice, 0, (struct IDirect3DBaseTexture9*)tex->tex);

			IDirect3DDevice9_SetSamplerState(D3D->pDevice, 0, D3DSAMP_ADDRESSU, tex->flags & NVG_IMAGE_REPEATX ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
			IDirect3DDevice9_SetSamplerState(D3D->pDevice, 0, D3DSAMP_ADDRESSV, tex->flags & NVG_IMAGE_REPEATY ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
			IDirect3DDevice9_SetSamplerState(D3D->pDevice, 0, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);
		}
	}
}

void D3D9nvg__renderViewport(void* uptr, float width, float height, float devicePixelRatio)
{
	NVG_NOTUSED(devicePixelRatio);

	struct D3D9NVGcontext* D3D = (struct D3D9NVGcontext*)uptr;
	D3D->view[0] = (float)width;
	D3D->view[1] = (float)height;
}

void D3D9nvg__fill(struct D3D9NVGcontext* D3D, struct D3D9NVGcall* call)
{
	struct D3D9NVGpath* paths = &D3D->paths[call->pathOffset];
	int i, npaths = call->pathCount;

	// set bindpoint for solid loc
	D3D9nvg__setUniforms(D3D, call->uniformOffset, 0);

	// Draw shapes
	IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILENABLE, TRUE);
	IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_COLORWRITEENABLE, 0);
	IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
	IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
	IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILPASS, D3DSTENCILOP_INCRSAT);
	IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILFUNC, D3DCMP_ALWAYS);
	IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_KEEP);
	IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP);
	IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILPASS, D3DSTENCILOP_DECRSAT);
	IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CULLMODE, D3DCULL_NONE);

	for (i = 0; i < npaths; i++)
	{
		// Draws a fan using indices to fake it up, since there isn't a fan primitive in D3D11.
		/*if (paths[i].fillCount > 2)
		{
			unsigned int numIndices = ((paths[i].fillCount - 2) * 3);
			assert(numIndices < D3D->VertexBuffer.MaxBufferEntries);
			if (numIndices < D3D->VertexBuffer.MaxBufferEntries)
			{
				IDirect3DDevice9_DrawIndexedPrimitive(D3D->pDevice, D3DPT_TRIANGLELIST, paths[i].fillOffset, 0, 0, numIndices, paths[i].fillCount);
			}
		}*/
		if (paths[i].fillCount > 2)
			IDirect3DDevice9_DrawPrimitive(D3D->pDevice, D3DPT_TRIANGLEFAN, paths[i].fillOffset, paths[i].fillCount - 2);
	}

	IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CULLMODE, D3DCULL_CW);
	IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_COLORWRITEENABLE, 0xf);

	D3D9nvg__setUniforms(D3D, call->uniformOffset + D3D->fragSize, call->image);

	// Draw anti-aliased pixels
	if (D3D->flags & NVG_ANTIALIAS)
	{
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILFUNC, D3DCMP_EQUAL);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILFUNC, D3DCMP_EQUAL);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_KEEP);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILPASS, D3DSTENCILOP_KEEP);

		// Draw fringes
		for (i = 0; i < npaths; i++)
		{
			IDirect3DDevice9_DrawPrimitive(D3D->pDevice, D3DPT_TRIANGLESTRIP, paths[i].strokeOffset, paths[i].strokeCount - 2);
		}
	}

	// Draw fill
	IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILFUNC, D3DCMP_NOTEQUAL);
	IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILFAIL, D3DSTENCILOP_ZERO);
	IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILZFAIL, D3DSTENCILOP_ZERO);
	IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILPASS, D3DSTENCILOP_ZERO);
	IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILFUNC, D3DCMP_NOTEQUAL);
	IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_ZERO);
	IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_ZERO);
	IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILPASS, D3DSTENCILOP_ZERO);

	IDirect3DDevice9_DrawPrimitive(D3D->pDevice, D3DPT_TRIANGLELIST, call->triangleOffset, call->triangleCount / 3);

	IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILENABLE, FALSE);
}

void D3D9nvg__convexFill(struct D3D9NVGcontext* D3D, struct D3D9NVGcall* call)
{
	struct D3D9NVGpath* paths = &D3D->paths[call->pathOffset];
	int i, npaths = call->pathCount;

	D3D9nvg__setUniforms(D3D, call->uniformOffset, call->image);

	for (i = 0; i < npaths; i++)
	{
		IDirect3DDevice9_DrawPrimitive(D3D->pDevice, D3DPT_TRIANGLEFAN, paths[i].fillOffset, paths[i].fillCount - 2);
	}

	if (D3D->flags & NVG_ANTIALIAS)
	{
		// Draw fringes
		for (i = 0; i < npaths; i++)
		{
			IDirect3DDevice9_DrawPrimitive(D3D->pDevice, D3DPT_TRIANGLESTRIP, paths[i].strokeOffset, paths[i].strokeCount - 2);
		}
	}
}

void D3D9nvg__stroke(struct D3D9NVGcontext* D3D, struct D3D9NVGcall* call)
{
	struct D3D9NVGpath* paths = &D3D->paths[call->pathOffset];
	int npaths = call->pathCount, i;

	if (D3D->flags & NVG_STENCIL_STROKES)
	{
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILENABLE, TRUE);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILFUNC, D3DCMP_EQUAL);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILPASS, D3DSTENCILOP_INCR);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILFUNC, D3DCMP_EQUAL);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_KEEP);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILPASS, D3DSTENCILOP_INCR);

		D3D9nvg__setUniforms(D3D, call->uniformOffset + D3D->fragSize, call->image);

		// Fill the stroke base without overlap
		for (i = 0; i < npaths; i++)
		{
			IDirect3DDevice9_DrawPrimitive(D3D->pDevice, D3DPT_TRIANGLESTRIP, paths[i].strokeOffset, paths[i].strokeCount - 2);
		}

		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILPASS, D3DSTENCILOP_KEEP);

		D3D9nvg__setUniforms(D3D, call->uniformOffset, call->image);

		// Draw anti-aliased pixels.
		for (i = 0; i < npaths; i++)
		{
			IDirect3DDevice9_DrawPrimitive(D3D->pDevice, D3DPT_TRIANGLESTRIP, paths[i].strokeOffset, paths[i].strokeCount - 2);
		}

		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_COLORWRITEENABLE, 0);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILFAIL, D3DSTENCILOP_ZERO);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILZFAIL, D3DSTENCILOP_ZERO);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILPASS, D3DSTENCILOP_ZERO);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILFUNC, D3DCMP_ALWAYS);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_ZERO);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_ZERO);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILPASS, D3DSTENCILOP_ZERO);

		// Clear stencil buffer.		
		for (i = 0; i < npaths; i++)
		{
			IDirect3DDevice9_DrawPrimitive(D3D->pDevice, D3DPT_TRIANGLESTRIP, paths[i].strokeOffset, paths[i].strokeCount - 2);
		}

		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_COLORWRITEENABLE, 0xf);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILENABLE, FALSE);
	}
	else
	{
		D3D9nvg__setUniforms(D3D, call->uniformOffset, call->image);

		for (i = 0; i < npaths; i++)
		{
			IDirect3DDevice9_DrawPrimitive(D3D->pDevice, D3DPT_TRIANGLESTRIP, paths[i].strokeOffset, paths[i].strokeCount - 2);
		}
	}
}

void D3D9nvg__triangles(struct D3D9NVGcontext* D3D, struct D3D9NVGcall* call)
{
	D3D9nvg__setUniforms(D3D, call->uniformOffset, call->image);

	IDirect3DDevice9_DrawPrimitive(D3D->pDevice, D3DPT_TRIANGLELIST, call->triangleOffset, call->triangleCount / 3);
}

void D3D9nvg__renderCancel(void* uptr)
{
	struct D3D9NVGcontext* D3D = (struct D3D9NVGcontext*)uptr;
	D3D->nverts = 0;
	D3D->npaths = 0;
	D3D->ncalls = 0;
	D3D->nuniforms = 0;
}

void D3D9nvg__renderFlush(void* uptr)
{
	struct D3D9NVGcontext* D3D = (struct D3D9NVGcontext*)uptr;
	int i;

	if (D3D->ncalls > 0)
	{
		unsigned int buffer0Offset = D3D9nvg_updateVertexBuffer(D3D);

		// Upload vertex data
		IDirect3DDevice9_SetStreamSource(D3D->pDevice, 0, D3D->VertexBuffer.pBuffer, buffer0Offset, sizeof(NVGvertex));
		IDirect3DDevice9_SetVertexDeclaration(D3D->pDevice, D3D->pLayoutRenderTriangles);

		// Ensure valid state
		IDirect3DDevice9_SetVertexShader(D3D->pDevice, D3D->shader.vert);
		IDirect3DDevice9_SetPixelShader(D3D->pDevice, D3D->shader.frag);

		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_SRCBLEND, D3DBLEND_ONE);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		//IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CULLMODE, D3DCULL_CW);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_ALPHABLENDENABLE, TRUE);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_ZENABLE, D3DZB_FALSE);
		//IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_SCISSORTESTENABLE, FALSE);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_COLORWRITEENABLE, 0xf);
		//IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILMASK, 0xffffffff);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILWRITEMASK, 0xffffffff);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILFUNC, D3DCMP_ALWAYS);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_KEEP);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_CCW_STENCILPASS, D3DSTENCILOP_KEEP);
		IDirect3DDevice9_SetRenderState(D3D->pDevice, D3DRS_STENCILREF, 0);

		D3DXMATRIX constant_data;
		IDirect3DDevice9_GetVertexShaderConstantF(D3D->pDevice, 0, constant_data, 1);

		IDirect3DDevice9_SetVertexShaderConstantF(D3D->pDevice, 0, D3D->view, 1);

		IDirect3DDevice9_SetSamplerState(D3D->pDevice, 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice9_SetSamplerState(D3D->pDevice, 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice9_SetSamplerState(D3D->pDevice, 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice9_SetSamplerState(D3D->pDevice, 0, D3DSAMP_MIPMAPLODBIAS, 0);
		IDirect3DDevice9_SetSamplerState(D3D->pDevice, 0, D3DSAMP_MAXMIPLEVEL, 0);
		IDirect3DDevice9_SetSamplerState(D3D->pDevice, 0, D3DSAMP_MAXANISOTROPY, 1);
		IDirect3DDevice9_SetSamplerState(D3D->pDevice, 0, D3DSAMP_SRGBTEXTURE, FALSE);

		// Draw shapes	   
		for (i = 0; i < D3D->ncalls; i++) {
			struct D3D9NVGcall* call = &D3D->calls[i];

			if (call->type == D3D9NVG_FILL)
				D3D9nvg__fill(D3D, call);
			else if (call->type == D3D9NVG_CONVEXFILL)
				D3D9nvg__convexFill(D3D, call);
			else if (call->type == D3D9NVG_STROKE)
				D3D9nvg__stroke(D3D, call);
			else if (call->type == D3D9NVG_TRIANGLES)
				D3D9nvg__triangles(D3D, call);
		}

		IDirect3DDevice9_SetVertexShaderConstantF(D3D->pDevice, 0, constant_data, 1);
	}

	// Reset calls
	D3D->nverts = 0;
	D3D->npaths = 0;
	D3D->ncalls = 0;
	D3D->nuniforms = 0;
}

struct D3D9NVGcall* D3D9nvg__allocCall(struct D3D9NVGcontext* D3D)
{
	struct D3D9NVGcall* ret = NULL;
	if (D3D->ncalls + 1 > D3D->ccalls) {
		struct D3D9NVGcall* calls;
		int ccalls = D3D9nvg__maxi(D3D->ncalls + 1, 128) + D3D->ccalls / 2; // 1.5x Overallocate
		calls = (struct D3D9NVGcall*)realloc(D3D->calls, sizeof(struct D3D9NVGcall) * ccalls);
		if (calls == NULL) return NULL;
		D3D->calls = calls;
		D3D->ccalls = ccalls;
	}
	ret = &D3D->calls[D3D->ncalls++];
	memset(ret, 0, sizeof(struct D3D9NVGcall));
	return ret;
}

int D3D9nvg__allocPaths(struct D3D9NVGcontext* D3D, int n)
{
	int ret = 0;
	if (D3D->npaths + n > D3D->cpaths) {
		struct D3D9NVGpath* paths;
		int cpaths = D3D9nvg__maxi(D3D->npaths + n, 128) + D3D->cpaths / 2; // 1.5x Overallocate
		paths = (struct D3D9NVGpath*)realloc(D3D->paths, sizeof(struct D3D9NVGpath) * cpaths);
		if (paths == NULL) return -1;
		D3D->paths = paths;
		D3D->cpaths = cpaths;
	}
	ret = D3D->npaths;
	D3D->npaths += n;
	return ret;
}

int D3D9nvg__allocVerts(struct D3D9NVGcontext* D3D, int n)
{
	int ret = 0;
	if (D3D->nverts + n > D3D->cverts) {
		struct NVGvertex* verts;
		int cverts = D3D9nvg__maxi(D3D->nverts + n, 4096) + D3D->cverts / 2; // 1.5x Overallocate
		verts = (struct NVGvertex*)realloc(D3D->verts, sizeof(struct NVGvertex) * cverts);
		if (verts == NULL) return -1;
		D3D->verts = verts;
		D3D->cverts = cverts;
	}
	ret = D3D->nverts;
	D3D->nverts += n;
	return ret;
}

int D3D9nvg__allocFragUniforms(struct D3D9NVGcontext* D3D, int n)
{
	int ret = 0, structSize = D3D->fragSize;
	if (D3D->nuniforms + n > D3D->cuniforms) {
		unsigned char* uniforms;
		int cuniforms = D3D9nvg__maxi(D3D->nuniforms + n, 128) + D3D->cuniforms / 2; // 1.5x Overallocate
		uniforms = (unsigned char*)realloc(D3D->uniforms, structSize * cuniforms);
		if (uniforms == NULL) return -1;
		D3D->uniforms = uniforms;
		D3D->cuniforms = cuniforms;
	}
	ret = D3D->nuniforms * structSize;
	D3D->nuniforms += n;
	return ret;
}

struct D3D9NVGfragUniforms* nvg__fragUniformPtr(struct D3D9NVGcontext* D3D, int i)
{
	return (struct D3D9NVGfragUniforms*)&D3D->uniforms[i];
}

int D3D9nvg__getMaxVertCount(const struct NVGpath* paths, int npaths)
{
	int count = 0;
	for (int i = 0; i < npaths; i++)
	{
		NVGpath path = paths[i];
		count += path.nfill + path.nstroke;
	}

	return count;
}

void D3D9nvg__vset(struct NVGvertex* vtx, float x, float y, float u, float v)
{
	vtx->x = x;
	vtx->y = y;
	vtx->u = u;
	vtx->v = v;
}

void D3D9nvg__renderFill(void* uptr, struct NVGpaint* paint, NVGcompositeOperationState compositeOperation, struct NVGscissor* scissor, float fringe,
	const float* bounds, const struct NVGpath* paths, int npaths)
{
	struct D3D9NVGcontext* D3D = (struct D3D9NVGcontext*)uptr;
	struct D3D9NVGcall* call = D3D9nvg__allocCall(D3D);
	struct NVGvertex* quad;
	struct D3D9NVGfragUniforms* frag;
	int i, maxverts, offset;

	if (call == NULL) return;

	call->type = D3D9NVG_FILL;
	call->pathOffset = D3D9nvg__allocPaths(D3D, npaths);
	if (call->pathOffset == -1) goto error;
	call->pathCount = npaths;
	call->image = paint->image;

	if (npaths == 1 && paths[0].convex)
		call->type = D3D9NVG_CONVEXFILL;

	// Allocate vertices for all the paths.
	maxverts = D3D9nvg__maxVertCount(paths, npaths) + 6;
	offset = D3D9nvg__allocVerts(D3D, maxverts);
	if (offset == -1) goto error;

	for (i = 0; i < npaths; i++) {
		struct D3D9NVGpath* copy = &D3D->paths[call->pathOffset + i];
		const struct NVGpath* path = &paths[i];
		memset(copy, 0, sizeof(struct D3D9NVGpath));
		if (path->nfill > 0) {
			copy->fillOffset = offset;
			copy->fillCount = path->nfill;
			memcpy(&D3D->verts[offset], path->fill, sizeof(struct NVGvertex) * path->nfill);
			offset += path->nfill;
		}
		if (path->nstroke > 0) {
			copy->strokeOffset = offset;
			copy->strokeCount = path->nstroke;
			memcpy(&D3D->verts[offset], path->stroke, sizeof(struct NVGvertex) * path->nstroke);
			offset += path->nstroke;
		}
	}

	// Quad
	call->triangleOffset = offset;
	call->triangleCount = 6;
	quad = &D3D->verts[call->triangleOffset];
	D3D9nvg__vset(&quad[0], bounds[0], bounds[3], 0.5f, 1.0f);
	D3D9nvg__vset(&quad[1], bounds[2], bounds[3], 0.5f, 1.0f);
	D3D9nvg__vset(&quad[2], bounds[2], bounds[1], 0.5f, 1.0f);

	D3D9nvg__vset(&quad[3], bounds[0], bounds[3], 0.5f, 1.0f);
	D3D9nvg__vset(&quad[4], bounds[2], bounds[1], 0.5f, 1.0f);
	D3D9nvg__vset(&quad[5], bounds[0], bounds[1], 0.5f, 1.0f);

	// Setup uniforms for draw calls
	if (call->type == D3D9NVG_FILL) {
		call->uniformOffset = D3D9nvg__allocFragUniforms(D3D, 2);
		if (call->uniformOffset == -1) goto error;
		// Simple shader for stencil
		frag = nvg__fragUniformPtr(D3D, call->uniformOffset);
		memset(frag, 0, sizeof(*frag));
		frag->strokeMult[1] = -1.0f;
		frag->type = NSVG_SHADER_SIMPLE;
		// Fill shader
		D3D9nvg__convertPaint(D3D, nvg__fragUniformPtr(D3D, call->uniformOffset + D3D->fragSize), paint, compositeOperation, scissor, fringe, fringe, -1.0f);
	}
	else {
		call->uniformOffset = D3D9nvg__allocFragUniforms(D3D, 1);
		if (call->uniformOffset == -1) goto error;
		// Fill shader
		D3D9nvg__convertPaint(D3D, nvg__fragUniformPtr(D3D, call->uniformOffset), paint, compositeOperation, scissor, fringe, fringe, -1.0f);
	}

	return;

error:
	// We get here if call alloc was ok, but something else is not.
	// Roll back the last call to prevent drawing it.
	if (D3D->ncalls > 0) D3D->ncalls--;
}

void D3D9nvg__renderStroke(void* uptr, struct NVGpaint* paint, NVGcompositeOperationState compositeOperation, struct NVGscissor* scissor, float fringe,
	float strokeWidth, const struct NVGpath* paths, int npaths)
{
	struct D3D9NVGcontext* D3D = (struct D3D9NVGcontext*)uptr;
	struct D3D9NVGcall* call = D3D9nvg__allocCall(D3D);
	int i, maxverts, offset;

	if (call == NULL) return;

	call->type = D3D9NVG_STROKE;
	call->pathOffset = D3D9nvg__allocPaths(D3D, npaths);
	if (call->pathOffset == -1) goto error;
	call->pathCount = npaths;
	call->image = paint->image;

	// Allocate vertices for all the paths.
	maxverts = D3D9nvg__maxVertCount(paths, npaths);
	offset = D3D9nvg__allocVerts(D3D, maxverts);
	if (offset == -1) goto error;

	for (i = 0; i < npaths; i++) {
		struct D3D9NVGpath* copy = &D3D->paths[call->pathOffset + i];
		const struct NVGpath* path = &paths[i];
		memset(copy, 0, sizeof(struct D3D9NVGpath));
		if (path->nstroke) {
			copy->strokeOffset = offset;
			copy->strokeCount = path->nstroke;
			memcpy(&D3D->verts[offset], path->stroke, sizeof(struct NVGvertex) * path->nstroke);
			offset += path->nstroke;
		}
	}

	if (D3D->flags & NVG_STENCIL_STROKES) {
		// Fill shader
		call->uniformOffset = D3D9nvg__allocFragUniforms(D3D, 2);
		if (call->uniformOffset == -1) goto error;

		D3D9nvg__convertPaint(D3D, nvg__fragUniformPtr(D3D, call->uniformOffset), paint, compositeOperation, scissor, strokeWidth, fringe, -1.0f);
		D3D9nvg__convertPaint(D3D, nvg__fragUniformPtr(D3D, call->uniformOffset + D3D->fragSize), paint, compositeOperation, scissor, strokeWidth, fringe, 1.0f - 0.5f / 255.0f);

	}
	else {
		// Fill shader
		call->uniformOffset = D3D9nvg__allocFragUniforms(D3D, 1);
		if (call->uniformOffset == -1) goto error;
		D3D9nvg__convertPaint(D3D, nvg__fragUniformPtr(D3D, call->uniformOffset), paint, compositeOperation, scissor, strokeWidth, fringe, -1.0f);
	}

	return;

error:
	// We get here if call alloc was ok, but something else is not.
	// Roll back the last call to prevent drawing it.
	if (D3D->ncalls > 0) D3D->ncalls--;
}

void D3D9nvg__renderTriangles(void* uptr, struct NVGpaint* paint, NVGcompositeOperationState compositeOperation, struct NVGscissor* scissor,
	const struct NVGvertex* verts, int nverts, float fringe)
{
	struct D3D9NVGcontext* D3D = (struct D3D9NVGcontext*)uptr;
	struct D3D9NVGcall* call;
	struct D3D9NVGfragUniforms* frag;

	if (nverts == 0)
	{
		return;
	}

	call = D3D9nvg__allocCall(D3D);

	if (call == NULL) return;

	call->type = D3D9NVG_TRIANGLES;
	call->image = paint->image;

	// Allocate vertices for all the paths.
	call->triangleOffset = D3D9nvg__allocVerts(D3D, nverts);
	if (call->triangleOffset == -1) goto error;
	call->triangleCount = nverts;

	memcpy(&D3D->verts[call->triangleOffset], verts, sizeof(struct NVGvertex) * nverts);

	// Fill shader
	call->uniformOffset = D3D9nvg__allocFragUniforms(D3D, 1);
	if (call->uniformOffset == -1) goto error;
	frag = nvg__fragUniformPtr(D3D, call->uniformOffset);
	D3D9nvg__convertPaint(D3D, frag, paint, compositeOperation, scissor, 1.0f, fringe, -1.0f);
	frag->type = NSVG_SHADER_IMG;

	return;

error:
	// We get here if call alloc was ok, but something else is not.
	// Roll back the last call to prevent drawing it.
	if (D3D->ncalls > 0) D3D->ncalls--;
}

void D3D9nvg__renderDelete(void* uptr)
{
	struct D3D9NVGcontext* D3D = (struct D3D9NVGcontext*)uptr;
	int i;
	if (D3D == NULL)
	{
		return;
	}

	D3D9nvg__deleteShader(&D3D->shader);

	for (i = 0; i < D3D->ntextures; i++)
	{
		if (D3D->textures[i].tex != NULL && (D3D->textures[i].flags & NVG_IMAGE_NODELETE) == 0)
		{
			IDirect3DTexture9_Release(D3D->textures[i].tex);
		}
	}

	IDirect3DVertexBuffer9_Release(D3D->VertexBuffer.pBuffer);

	IDirect3DVertexDeclaration9_Release(D3D->pLayoutRenderTriangles);

	free(D3D->textures);

	free(D3D->paths);
	free(D3D->verts);
	free(D3D->uniforms);
	free(D3D->calls);

	free(D3D);
}

struct NVGcontext* nvgCreateD3D9(IDirect3DDevice9* pDevice, int flags)
{
	struct NVGparams params;
	struct NVGcontext* ctx = NULL;
	struct D3D9NVGcontext* D3D = (struct D3D9NVGcontext*)malloc(sizeof(struct D3D9NVGcontext));
	if (D3D == NULL)
	{
		goto error;
	}
	memset(D3D, 0, sizeof(struct D3D9NVGcontext));
	D3D->pDevice = pDevice;

	memset(&params, 0, sizeof(params));
	params.renderCreate = D3D9nvg__renderCreate;
	params.renderCreateTexture = D3D9nvg__renderCreateTexture;
	params.renderDeleteTexture = D3D9nvg__renderDeleteTexture;
	params.renderUpdateTexture = D3D9nvg__renderUpdateTexture;
	params.renderGetTextureSize = D3D9nvg__renderGetTextureSize;
	params.renderViewport = D3D9nvg__renderViewport;
	params.renderCancel = D3D9nvg__renderCancel;
	params.renderFlush = D3D9nvg__renderFlush;
	params.renderFill = D3D9nvg__renderFill;
	params.renderStroke = D3D9nvg__renderStroke;
	params.renderTriangles = D3D9nvg__renderTriangles;
	params.renderDelete = D3D9nvg__renderDelete;
	params.userPtr = D3D;
	params.edgeAntiAlias = flags & NVG_ANTIALIAS ? 1 : 0;

	D3D->flags = flags;

	ctx = nvgCreateInternal(&params);
	if (ctx == NULL) goto error;

	return ctx;

error:
	// 'D3D9' is freed by nvgDeleteInternal.
	if (ctx != NULL) nvgDeleteInternal(ctx);
	return NULL;
}

void nvgDeleteD3D9(struct NVGcontext* ctx)
{
	nvgDeleteInternal(ctx);
}

#endif