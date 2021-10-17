#pragma once

#include <algorithm>

#include "RenderQueue.h"
#include "SpriteID.h"
#include "Sprite.h"
#include "SpriteTransform.h"

#include "../Math/OBB.h"
#include "../Math/AABB.h"


RenderQueue::RenderQueue(Renderer& renderer, const int screenWidth, const int screenHeight)
	: ScreenWidth(screenWidth),
	  ScreenHeight(screenHeight),
	  _FocalPointX(0),
	  _FocalPointY(0),
	  _SpriteAtlas(renderer)
{
	_RenderQueue.reserve(256); // arbitrary, but a decent starting size for total number of rendered sprites?)
}


void
RenderQueue::Enqueue(const SpriteID spriteID, const SDL_Rect& targetRect, const float rotation, const Layer layer)
{
	auto finalTargetRect = targetRect;
	finalTargetRect.x -= _FocalPointX;
	finalTargetRect.y -= _FocalPointY;

	EnqueueScreenSpace(spriteID, finalTargetRect, rotation, layer);
}

void
RenderQueue::Enqueue(const SpriteID spriteID, const float rotation, const Layer layer)
{
	SDL_Rect targetRect;
	targetRect.w = ScreenWidth;
	targetRect.h = ScreenHeight;
	targetRect.x = 0;
	targetRect.y = 0;
	Enqueue(spriteID, targetRect, rotation, layer);
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
	const AABB screenAABB(Vector2::zero(), Vector2(static_cast<float>(ScreenWidth), static_cast<float>(ScreenHeight)));

	// world-to-screenspace
	auto spriteWorldPos = transform.Position;
	spriteWorldPos.x -= _FocalPointX;
	spriteWorldPos.y -= _FocalPointY;

	const auto spriteOBB = OBB(spriteWorldPos, transform.Rotation);

	if(!screenAABB.FullyContains(spriteOBB))
	{
		// Sprite is visible on the opposite side. Determine which!
		const auto spriteAABB = spriteOBB.Bounds();

		/*
			PRETTY PICTURE TO VISUALIZE CASES
				A |  B | C
			  ____|____|____
				D |    | E
			  ____|____|____
			    F |  G | H
				  |    |
		*/

		if(spriteAABB.top < screenAABB.top)
		{
			// Case A - B - C
			DrawAtBottom(transform, screenAABB);

			if(spriteAABB.left < screenAABB.left)
			{
				// Case A
				DrawAtRight(transform, screenAABB);

				// DrawAtBottomRight
				SDL_Rect newPos = transform.Position;
				newPos.y += static_cast<int>(floor(screenAABB.bottom));
				newPos.x += static_cast<int>(floor(screenAABB.right));
				Enqueue(transform.ID, newPos, transform.Rotation, transform.Layer);
			}
			else if(spriteAABB.right > screenAABB.right)
			{
				// Case C
				DrawAtLeft(transform, screenAABB);

				// DrawAtBottomLeft
				SDL_Rect newPos = transform.Position;
				newPos.y += static_cast<int>(floor(screenAABB.bottom));
				newPos.x -= static_cast<int>(floor(screenAABB.right));
				Enqueue(transform.ID, newPos, transform.Rotation, transform.Layer);
			}
		}
		else if(spriteAABB.bottom > screenAABB.bottom)
		{
			// Case F - G - H
			DrawAtTop(transform, screenAABB);
			if(spriteAABB.left < screenAABB.left)
			{
				// Case F
				DrawAtRight(transform, screenAABB);

				// DrawAtTopRight

				auto newPos = transform.Position;
				newPos.y -= static_cast<int>(floor(screenAABB.bottom));
				newPos.x += static_cast<int>(floor(screenAABB.right));
				Enqueue(transform.ID, newPos, transform.Rotation, transform.Layer);
			}
			else if(spriteAABB.right > screenAABB.right)
			{
				// Case H
				DrawAtLeft(transform, screenAABB);

				// DrawAtTopLeft

				auto newPos = transform.Position;
				newPos.y -= static_cast<int>(floor(screenAABB.bottom));
				newPos.x -= static_cast<int>(floor(screenAABB.right));
				Enqueue(transform.ID, newPos, transform.Rotation, transform.Layer);
			}
		}
		else
		{
			// Case D - E
			if(spriteAABB.left < screenAABB.left)
			{
				// Case D
				DrawAtRight(transform, screenAABB);
			}
			else if(spriteAABB.right > screenAABB.right)
			{
				// Case E
				DrawAtLeft(transform, screenAABB);
			}
		}
	}

	// Render at the original position
	Enqueue(transform.ID, transform.Position, transform.Rotation, transform.Layer);
}

void
RenderQueue::SetCameraLocation(const int& x, const int& y)
{
	_FocalPointX = x - ScreenWidth/2;
	_FocalPointY = y - ScreenHeight/2;
}


void
RenderQueue::DrawAtTop(const SpriteTransform& transform, const AABB& screenAABB)
{
	auto newPos = transform.Position;
	newPos.y -= static_cast<int>(floor(screenAABB.bottom));
	Enqueue(transform.ID, newPos, transform.Rotation, transform.Layer);
}

void
RenderQueue::DrawAtBottom(const SpriteTransform& transform, const AABB& screenAABB)
{
	auto newPos = transform.Position;
	newPos.y += static_cast<int>(floor(screenAABB.bottom));
	Enqueue(transform.ID, newPos, transform.Rotation, transform.Layer);
}

void
RenderQueue::DrawAtLeft(const SpriteTransform& transform, const AABB& screenAABB)
{
	auto newPos = transform.Position;
	newPos.x -= static_cast<int>(floor(screenAABB.right));
	Enqueue(transform.ID, newPos, transform.Rotation, transform.Layer);
}

void
RenderQueue::DrawAtRight(const SpriteTransform& transform, const AABB& screenAABB)
{
	auto newPos = transform.Position;
	newPos.x += static_cast<int>(floor(screenAABB.right));
	Enqueue(transform.ID, newPos, transform.Rotation, transform.Layer);
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
RenderQueue::Clear()
{
	_RenderQueue.clear();
}
