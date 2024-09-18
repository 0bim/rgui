//
// Copyright (c) 2009-2013 Mikko Mononen memon@inside.org
// Port of _gl.h to _d3d9.h by Patrick Mours
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
#ifndef NANOVG_D3D9_H
#define NANOVG_D3D9_H

#include <d3d9.h>

#ifdef __cplusplus
extern "C" {
#endif

enum NVGcreateFlags {
	// Flag indicating if geometry based anti-aliasing is used (may not be needed when using MSAA).
	NVG_ANTIALIAS 		= 1<<0,
	// Flag indicating if strokes should be drawn using stencil buffer. The rendering will be a little
	// slower, but path overlaps (i.e. self-intersecting or sharp turns) will be drawn just once.
	NVG_STENCIL_STROKES	= 1<<1,
};

struct NVGcontext* nvgCreateD3D9(IDirect3DDevice9* pDevice, int flags);
void nvgDeleteD3D9(struct NVGcontext* ctx);

// These are additional flags on top of NVGimageFlags.
enum NVGimageFlagsD3D9 {
	NVG_IMAGE_NODELETE			= 1<<16,	// Do not delete texture object.
};

#ifdef __cplusplus
}
#endif

#endif /* NANOVG_D3D9_H */

#ifdef NANOVG_D3D9_IMPLEMENTATION

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "nanovg.h"

#include "D3D9VertexShader.h"
#include "D3D9PixelShaderAA.h"
#include "D3D9PixelShader.h"

enum D3D9NVGshaderType {
	NSVG_SHADER_FILLGRAD,
	NSVG_SHADER_FILLIMG,
	NSVG_SHADER_SIMPLE,
	NSVG_SHADER_IMG
};

struct D3D9NVGshader {
	IDirect3DPixelShader9* frag;
	IDirect3DVertexShader9* vert;
};

struct D3D9NVGtexture {
	int id;
	IDirect3DTexture9* tex;
	int width, height;
	int type;
	int flags;
};

enum D3D9NVGcallType {
	D3D9NVG_NONE = 0,
	D3D9NVG_FILL,
	D3D9NVG_CONVEXFILL,
	D3D9NVG_STROKE,
	D3D9NVG_TRIANGLES
};

struct D3D9NVGcall {
	int type;
	int image;
	int pathOffset;
	int pathCount;
	int triangleOffset;
	int triangleCount;
	int uniformOffset;
};

struct D3D9NVGpath {
	int fillOffset;
	int fillCount;
	int strokeOffset;
	int strokeCount;
};

struct D3D9NVGvertexbuffer {
	unsigned int MaxBufferEntries;
	unsigned int CurrentBufferEntry;
	IDirect3DVertexBuffer9* pBuffer;
};

struct D3D9NVGfragUniforms {
	// note: after modifying layout or size of uniform array,
	// don't forget to also update the fragment shader source!
	#define NANOVG_D3D9_UNIFORMARRAY_SIZE 14
	union {
		struct {
			float scissorMat[16]; // matrices are actually 3 vec4s
			float scissorExt[2];
			float scissorScale[2];
			float paintMat[16];
			struct NVGcolor innerCol;
			struct NVGcolor outerCol;
			float extent[2];
			float radius;
			float feather;
			float strokeMult[4];
			float type;
			float texType;
		};
		float uniformArray[NANOVG_D3D9_UNIFORMARRAY_SIZE][4];
	};
};

struct D3D9NVGcontext {
	
	struct D3D9NVGshader shader;
	struct D3D9NVGtexture* textures;
	float view[4];    
	int ntextures;
	int ctextures;
	int textureId;

	int fragSize;
	int flags;

	// Per frame buffers
	struct D3D9NVGcall* calls;
	int ccalls;
	int ncalls;
	struct D3D9NVGpath* paths;
	int cpaths;
	int npaths;
	struct NVGvertex* verts;
	int cverts;
	int nverts;
	unsigned char* uniforms;
	int cuniforms;
	int nuniforms;

	// D3D9
	// Geometry
	struct D3D9NVGvertexbuffer VertexBuffer;
	IDirect3DVertexDeclaration9* pLayoutRenderTriangles;

	// State
	IDirect3DDevice9* pDevice;

	LPDIRECT3DINDEXBUFFER9 pIndexBuffer;
};

static int D3D9nvg__maxi(int a, int b) { return a > b ? a : b; }

int D3D9nvg__getMaxVertCount(const struct NVGpath* paths, int npaths);

static struct D3D9NVGtexture* D3D9nvg__allocTexture(struct D3D9NVGcontext* D3D)
{
	struct D3D9NVGtexture* tex = NULL;
	int i;

	for (i = 0; i < D3D->ntextures; i++) {
		if (D3D->textures[i].id == 0) {
			tex = &D3D->textures[i];
			break;
		}
	}
	if (tex == NULL) {
		if (D3D->ntextures + 1 > D3D->ctextures) {
			struct D3D9NVGtexture* textures;
			int ctextures = D3D9nvg__maxi(D3D->ntextures+1, 4) +  D3D->ctextures/2; // 1.5x Overallocate
			textures = (struct D3D9NVGtexture*)realloc(D3D->textures, sizeof(struct D3D9NVGtexture)*ctextures);
			if (textures == NULL) return NULL;
			D3D->textures = textures;
			D3D->ctextures = ctextures;
		}
		tex = &D3D->textures[D3D->ntextures++];
	}

	memset(tex, 0, sizeof(*tex));
	tex->id = ++D3D->textureId;

	return tex;
}

static struct D3D9NVGtexture* D3D9nvg__findTexture(struct D3D9NVGcontext* D3D, int id)
{
	int i;
	for (i = 0; i < D3D->ntextures; i++)
		if (D3D->textures[i].id == id)
			return &D3D->textures[i];
	return NULL;
}

static int D3D9nvg__deleteTexture(struct D3D9NVGcontext* D3D, int id)
{
	int i;
	for (i = 0; i < D3D->ntextures; i++) {
		if (D3D->textures[i].id == id) {
			if (D3D->textures[i].tex != NULL && (D3D->textures[i].flags & NVG_IMAGE_NODELETE) == 0)
			{
				IDirect3DTexture9_Release(D3D->textures[i].tex);
			}
			memset(&D3D->textures[i], 0, sizeof(D3D->textures[i]));
			return 1;
		}
	}
	return 0;
}

static int D3D9nvg__createShader(struct D3D9NVGcontext* D3D, struct D3D9NVGshader* shader, const void* vshader, const void* fshader)
{
	IDirect3DVertexShader9* vert = NULL;
	IDirect3DPixelShader9* frag = NULL;
	HRESULT hr;
	
	memset(shader, 0, sizeof(*shader));

	// Shader byte code is created at compile time from the .hlsl files.
	// No need for error checks; shader errors can be fixed in the IDE.
	hr = IDirect3DDevice9_CreateVertexShader(D3D->pDevice, (const DWORD*)vshader, &vert);

	if (FAILED(hr))
		return 0;

	hr = IDirect3DDevice9_CreatePixelShader(D3D->pDevice, (const DWORD*)fshader, &frag);

	if (FAILED(hr))
	{
		IDirect3DVertexShader9_Release(vert);
		return 0;
	}
	
	shader->vert = vert;
	shader->frag = frag;

	return 1;
}

static void D3D9nvg__deleteShader(struct D3D9NVGshader* shader)
{
	if (shader->vert != NULL)
		IDirect3DVertexShader9_Release(shader->vert);
	if (shader->frag != NULL)
		IDirect3DPixelShader9_Release(shader->frag);
}

static unsigned int D3Dnvg_buildFanIndices(struct D3D9NVGcontext* D3D)
{
	/*UINT32 index0 = 0;
	UINT32 index1 = 1;
	UINT32 index2 = 2;
	UINT32 current = 0;

	D3D->pIndexBuffer = 0;
	HRESULT hr = IDirect3DDevice9_CreateIndexBuffer(D3D->pDevice, D3D->VertexBuffer.MaxBufferEntries * sizeof(int), D3DUSAGE_DYNAMIC, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &D3D->pIndexBuffer, 0);
	
	UINT32* pIndexData = 0;
	D3D->pIndexBuffer->lpVtbl->Lock(D3D->pIndexBuffer, 0, 0, &pIndexData, 0);

	while (current < (D3D->VertexBuffer.MaxBufferEntries - 3))
	{
		pIndexData[current++] = index0;
		pIndexData[current++] = index1++;
		pIndexData[current++] = index2++;
	}

	D3D->pIndexBuffer->lpVtbl->Unlock(D3D->pIndexBuffer);

	return hr;*/

	return 0;
}

static unsigned int D3D9nvg_updateVertexBuffer(struct D3D9NVGcontext* D3D)
{
	void* data;
	unsigned int offset = D3D->VertexBuffer.CurrentBufferEntry * sizeof(NVGvertex), size = D3D->nverts * sizeof(NVGvertex);
	HRESULT hr;

	if (D3D->nverts > D3D->VertexBuffer.MaxBufferEntries)
	{
		IDirect3DVertexBuffer9_Release(D3D->VertexBuffer.pBuffer);

		while (D3D->VertexBuffer.MaxBufferEntries < D3D->nverts)
			D3D->VertexBuffer.MaxBufferEntries *= 2;
		D3D->VertexBuffer.CurrentBufferEntry = 0;
		hr = IDirect3DDevice9_CreateVertexBuffer(D3D->pDevice, sizeof(NVGvertex) * D3D->VertexBuffer.MaxBufferEntries, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &D3D->VertexBuffer.pBuffer, NULL);

		if (FAILED(hr))
		{
			ZeroMemory(&D3D->VertexBuffer, sizeof(D3D->VertexBuffer));
			return 0;
		}
	}
	if ((D3D->VertexBuffer.CurrentBufferEntry + D3D->nverts) >= D3D->VertexBuffer.MaxBufferEntries || D3D->VertexBuffer.CurrentBufferEntry == 0)
	{
		offset = 0;
		D3D->VertexBuffer.CurrentBufferEntry = 0;
		IDirect3DVertexBuffer9_Lock(D3D->VertexBuffer.pBuffer, 0, size, &data, D3DLOCK_DISCARD);
	}
	else
	{
		IDirect3DVertexBuffer9_Lock(D3D->VertexBuffer.pBuffer, offset, size, &data, D3DLOCK_NOOVERWRITE);
	}

	memcpy(data, D3D->verts, size);
	
	IDirect3DVertexBuffer9_Unlock(D3D->VertexBuffer.pBuffer);

	D3D->VertexBuffer.CurrentBufferEntry += D3D->nverts;

	return offset;
}

static int D3D9nvg__renderCreate(void* uptr)
{
	HRESULT hr;
	struct D3D9NVGcontext* D3D = (struct D3D9NVGcontext*)uptr;

	const D3DVERTEXELEMENT9 LayoutRenderTriangles[] = 
	{
		{ 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 8, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	}; 
	
	if (D3D->flags & NVG_ANTIALIAS) {
		if (D3D9nvg__createShader(D3D, &D3D->shader, g_vs30_D3D9VertexShader_Main, g_ps30_D3D9PixelShaderAA_Main) == 0)
			return 0;
	}
	else {
		if (D3D9nvg__createShader(D3D, &D3D->shader, g_vs30_D3D9VertexShader_Main, g_ps30_D3D9PixelShader_Main) == 0)
			return 0;
	}

	D3D->VertexBuffer.MaxBufferEntries = 20000;
	D3D->VertexBuffer.CurrentBufferEntry = 0;

	// Create the vertex buffer.
	hr = IDirect3DDevice9_CreateVertexBuffer(D3D->pDevice, sizeof(NVGvertex) * D3D->VertexBuffer.MaxBufferEntries, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &D3D->VertexBuffer.pBuffer, NULL);
	
	if (FAILED(hr))
	{
		D3D9nvg__deleteShader(&D3D->shader);
		return 0;
	}

	hr = IDirect3DDevice9_CreateVertexDeclaration(D3D->pDevice, LayoutRenderTriangles, &D3D->pLayoutRenderTriangles);
	
	if (FAILED(hr))
	{
		D3D9nvg__deleteShader(&D3D->shader);
		IDirect3DVertexBuffer9_Release(D3D->VertexBuffer.pBuffer);
		return 0;
	}

	D3D->fragSize = sizeof(struct D3D9NVGfragUniforms) + 16 - sizeof(struct D3D9NVGfragUniforms) % 16;

	hr = D3Dnvg_buildFanIndices(D3D);

	if (FAILED(hr))
	{
		D3D9nvg__deleteShader(&D3D->shader);
		IDirect3DVertexBuffer9_Release(D3D->VertexBuffer.pBuffer);
		return 0;
	}
	
	return 1;
}

static int D3D9nvg__renderCreateTexture(void* uptr, int type, int w, int h, int imageFlags, const unsigned char* data)
{
	struct D3D9NVGcontext* D3D = (struct D3D9NVGcontext*)uptr;
	struct D3D9NVGtexture* tex = D3D9nvg__allocTexture(D3D);
	D3DLOCKED_RECT locked;
	HRESULT hr;
	INT levels = 1, usage = D3DUSAGE_DYNAMIC, size, pixelWidthBytes;
	D3DFORMAT format;
	int i, y;
	unsigned char *texData, *tmpData;
	
	if (tex == NULL)
	{
		return 0;
	}

	tex->width = w;
	tex->height = h;
	tex->type = type;
	tex->flags = imageFlags;

	// Mip maps
	if (imageFlags & NVG_IMAGE_GENERATE_MIPMAPS)
	{
		levels = 0;
		usage |= D3DUSAGE_AUTOGENMIPMAP;
	}

	if (type == NVG_TEXTURE_RGBA)
	{
		format = D3DFMT_A8R8G8B8;
		pixelWidthBytes = 4;
	}
	else
	{
		format = D3DFMT_L8;
		pixelWidthBytes = 1;
	}

	size = w * h * pixelWidthBytes;

	hr = IDirect3DDevice9_CreateTexture(D3D->pDevice, w, h, levels, usage, format, D3DPOOL_DEFAULT, &tex->tex, NULL);

	if (FAILED(hr))
	{
		return 0;
	}

	if (data != NULL)
	{
		hr = IDirect3DTexture9_LockRect(tex->tex, 0, &locked, NULL, 0);

		if (SUCCEEDED(hr))
		{
			texData = (unsigned char*)locked.pBits;

			// Have to copy by row since pitch can be different
			for (y = 0; y < h; y++) {
				memcpy(texData + y * locked.Pitch, data + y * w * pixelWidthBytes, w * pixelWidthBytes);
			}

			if (type == NVG_TEXTURE_RGBA)
			{
				for (y = 0; y < h; y++)
				{
					for (i = 0; i < w; i++)
					{
						tmpData = texData + y * locked.Pitch + i * pixelWidthBytes;
						unsigned char swp = tmpData[0];
						tmpData[0] = tmpData[2];
						tmpData[2] = swp;
					}
				}
			}

			IDirect3DTexture9_UnlockRect(tex->tex, 0);
		}
	}

	return tex->id;
}

static int D3D9nvg__renderDeleteTexture(void* uptr, int image)
{
	struct D3D9NVGcontext* D3D = (struct D3D9NVGcontext*)uptr;
	return D3D9nvg__deleteTexture(D3D, image);
}

static int D3D9nvg__renderUpdateTexture(void* uptr, int image, int x, int y, int w, int h, const unsigned char* data)
{
	struct D3D9NVGcontext* D3D = (struct D3D9NVGcontext*)uptr;
	struct D3D9NVGtexture* tex = D3D9nvg__findTexture(D3D, image);
	RECT rect;
	D3DLOCKED_RECT locked;
	HRESULT hr;
	INT pixelWidthBytes, size;
	unsigned char* texData;
	int i;
	
	if (tex == NULL)
	{
		return 0;
	}

	x = 0;
	w = tex->width;

	rect.left = x;
	rect.right = (x + w);
	rect.top = y;
	rect.bottom = (y + h);

	hr = IDirect3DTexture9_LockRect(tex->tex, 0, &locked, &rect, 0);

	if (FAILED(hr))
	{
		return 0;
	}

	if (tex->type == NVG_TEXTURE_RGBA)
	{
		pixelWidthBytes = 4;
	}
	else
	{
		pixelWidthBytes = 1;
	}

	size = w * h * pixelWidthBytes;
	texData = (unsigned char*)locked.pBits;

	memcpy(texData, data + y * tex->width * pixelWidthBytes, size);

	if (tex->type == NVG_TEXTURE_RGBA)
	{
		for (i = 0; i < size; i += 4)
		{
			unsigned char swp = texData[i];
			texData[i] = texData[i + 2];
			texData[i + 2] = swp;
		}
	}

	IDirect3DTexture9_UnlockRect(tex->tex, 0);
 
	return 1;
}

static int D3D9nvg__renderGetTextureSize(void* uptr, int image, int* w, int* h)
{
	struct D3D9NVGcontext* D3D = (struct D3D9NVGcontext*)uptr;
	struct D3D9NVGtexture* tex = D3D9nvg__findTexture(D3D, image);
	if (tex == NULL)
	{
		return 0;
	}
	*w = tex->width;
	*h = tex->height;
	return 1;
}

static void D3D9nvg__xformToMat3x3(float* m3, float* t)
{
	m3[0] = t[0];
	m3[1] = t[1];
	m3[2] = 0.0f;
	m3[3] = t[2];
	m3[4] = t[3];
	m3[5] = 0.0f;
	m3[6] = t[4];
	m3[7] = t[5];
	m3[8] = 1.0f;
}
static void D3D9nvg_copyMatrix3to4(float* pDest, const float* pSource)
{
	unsigned int i;
	for (i = 0; i < 4; i++)
	{
		memcpy(&pDest[i * 4], &pSource[i * 3], sizeof(float) * 3);
	}
}

static struct NVGcolor D3D9nvg__premulColor(struct NVGcolor c)
{
	c.r *= c.a;
	c.g *= c.a;
	c.b *= c.a;
	return c;
}

int D3D9nvg__convertPaint(struct D3D9NVGcontext* D3D, struct D3D9NVGfragUniforms* frag,
	struct NVGpaint* paint, NVGcompositeOperationState compositeOperation, struct NVGscissor* scissor,
	float width, float fringe, float strokeThr);

struct D3D9NVGfragUniforms* nvg__fragUniformPtr(struct D3D9NVGcontext* D3D, int i);

void D3D9nvg__setUniforms(struct D3D9NVGcontext* D3D, int uniformOffset, int image);

void D3D9nvg__renderViewport(void* uptr, float width, float height, float devicePixelRatio);

void D3D9nvg__fill(struct D3D9NVGcontext* D3D, struct D3D9NVGcall* call);

static void D3D9nvg__convexFill(struct D3D9NVGcontext* D3D, struct D3D9NVGcall* call);

void D3D9nvg__stroke(struct D3D9NVGcontext* D3D, struct D3D9NVGcall* call);

void D3D9nvg__triangles(struct D3D9NVGcontext* D3D, struct D3D9NVGcall* call);

void D3D9nvg__renderCancel(void* uptr);

void D3D9nvg__renderFlush(void* uptr);

static inline int D3D9nvg__maxVertCount(const struct NVGpath* paths, int npaths)
{
	return D3D9nvg__getMaxVertCount(paths, npaths);
}

struct D3D9NVGcall* D3D9nvg__allocCall(struct D3D9NVGcontext* D3D);

int D3D9nvg__allocPaths(struct D3D9NVGcontext* D3D, int n);

int D3D9nvg__allocVerts(struct D3D9NVGcontext* D3D, int n);

int D3D9nvg__allocFragUniforms(struct D3D9NVGcontext* D3D, int n);

static struct D3D9NVGfragUniforms* nvg__fragUniformPtr(struct D3D9NVGcontext* D3D, int i);

void D3D9nvg__vset(struct NVGvertex* vtx, float x, float y, float u, float v);

void D3D9nvg__renderFill(void* uptr, struct NVGpaint* paint, NVGcompositeOperationState compositeOperation, struct NVGscissor* scissor, float fringe,
	const float* bounds, const struct NVGpath* paths, int npaths);

void D3D9nvg__renderStroke(void* uptr, struct NVGpaint* paint, NVGcompositeOperationState compositeOperation, struct NVGscissor* scissor, float fringe,
	float strokeWidth, const struct NVGpath* paths, int npaths);

void D3D9nvg__renderTriangles(void* uptr, struct NVGpaint* paint, NVGcompositeOperationState compositeOperation, struct NVGscissor* scissor,
	const struct NVGvertex* verts, int nverts, float fringe);

void D3D9nvg__renderDelete(void* uptr);

struct NVGcontext* nvgCreateD3D9(IDirect3DDevice9* pDevice, int flags);

void nvgDeleteD3D9(struct NVGcontext* ctx);

#endif /* NANOVG_D3D9_IMPLEMENTATION */
