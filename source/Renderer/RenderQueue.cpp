#pragma once

#include <algorithm>

#include "RenderQueue.h"
#include "SpriteID.h"
#include "Sprite.h"
#include "SpriteTransform.h"

#include "../Math/AABB.h"


RenderQueue::RenderQueue(Renderer& renderer, Camera& camera, const Vector2& gameWorldDim)
	: _Camera(&camera),
	  _GameWorldDim(gameWorldDim),
	  _SpriteAtlas(renderer),
	  _CameraAABB(AABB(0, 100, 0, 100)),
	  _CameraScale(0)
{
	_RenderQueue.reserve(512); // arbitrary, but a decent starting size for total number of rendered sprites?)
}

void
RenderQueue::EnqueueBackground(const SpriteID spriteID, const float rotation, const Layer layer)
{
	// @TODO: This is broken.
	SDL_Rect targetRect;
	targetRect.w = static_cast<int>(_GameWorldDim.x);
	targetRect.h = static_cast<int>(_GameWorldDim.y);
	targetRect.x = 0;
	targetRect.y = 0;
	EnqueueScreenSpace(spriteID, targetRect, rotation, layer);
}

void
RenderQueue::EnqueueScreenSpace(const SpriteID spriteID, const SDL_Rect& targetRect, const float rotation, const Layer layer)
{
	//@NOTE: I used to do some complex queueing here where sorted insertion became O(log k) where k is the number of layers in use.
	// It turns out that having an O(1) insert and doing the sort at the end is faster, since we enqueue far more often than we
	// fetch.
	const auto [id, texture, source] = _SpriteAtlas.Get(spriteID);

	Element el;
	el.Tex     = texture;
	el.SrcRect = source;
	el.DstRect = targetRect;
	el.Angle   = rotation;
	el.Layer   = layer;

	_RenderQueue.push_back(el);
}

void
RenderQueue::EnqueueLooped(const SpriteTransform& transform)
{
	auto spriteStartMinX = static_cast<float>(transform.Position.x);
	auto spriteStartMaxX = static_cast<float>(transform.Position.w) + spriteStartMinX;

	auto spriteMinY = static_cast<float>(transform.Position.y);
	auto spriteMaxY = static_cast<float>(transform.Position.h) + spriteMinY;

	// Scroll backwards until we go "too far".
	//We step forward one at the beginning of the next step.
	while(spriteMaxY > _CameraAABB.top)
	{
		spriteMaxY -= _GameWorldDim.y;
		spriteMinY -= _GameWorldDim.y;
	}

	while(spriteStartMaxX > _CameraAABB.left)
	{
		spriteStartMaxX -= _GameWorldDim.x;
		spriteStartMinX -= _GameWorldDim.x;
	}

	auto newPos = transform.Position;
	newPos.w    = static_cast<int>(newPos.w * _CameraScale);
	newPos.h    = static_cast<int>(newPos.h * _CameraScale);
	while(true)
	{
		spriteMinY += _GameWorldDim.y;

		if(spriteMinY < _CameraAABB.bottom)
		{
			auto spriteMinX = spriteStartMinX;

			while(true)
			{
				spriteMinX += _GameWorldDim.x;

				if(spriteMinX < _CameraAABB.right)
				{
					const auto spriteMin = _Camera->WorldToCamera(Vector2(spriteMinX, spriteMinY));

					newPos.x = static_cast<int>(spriteMin.x);
					newPos.y = static_cast<int>(spriteMin.y);
					EnqueueScreenSpace(transform.ID, newPos, transform.Rotation, transform.Layer);
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			break;
		}
	}
}


// @TODO: Consider locking here and unlocking on clear. Rename methods to be explicit if you do so.
const std::vector<RenderQueue::Element>&
RenderQueue::GetRenderQueue()
{
	std::sort(_RenderQueue.begin(), _RenderQueue.end(),
	          [](const Element& a, const Element& b) -> bool
	          {
		          return a.Layer < b.Layer;
	          });

	return _RenderQueue;
}

void
RenderQueue::CacheCameraInfo(Camera* cam)
{
	_CameraAABB  = cam->GetCameraView();
	_CameraScale = cam->GetCameraScale();
	_Camera = cam;
}

void
RenderQueue::Clear()
{
	_RenderQueue.clear();
}
