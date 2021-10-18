#pragma once

#include <algorithm>

#include "RenderQueue.h"
#include "SpriteID.h"
#include "Sprite.h"
#include "SpriteTransform.h"

#include "../Math/AABB.h"


RenderQueue::RenderQueue(Renderer& renderer, Camera& camera, const int screenWidth, const int screenHeight)
	: _GameFieldWidth(screenWidth),
	  _GameFieldHeight(screenHeight),
	  _Camera(camera),
	  _SpriteAtlas(renderer)
{
	_RenderQueue.reserve(256); // arbitrary, but a decent starting size for total number of rendered sprites?)
}

void
RenderQueue::Enqueue(const SpriteID spriteID, const float rotation, const Layer layer)
{
	SDL_Rect targetRect;
	targetRect.w = _GameFieldWidth;
	targetRect.h = _GameFieldHeight;
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
#if 1

	// THINKY McTHINKFACE
	/*
	 *	How to place a looped box within a camera box.
	 *
	 *	OBSERVATION 1:
	 *		The camera space could, in theory, be crazy. It could
	 *		be -WorldWidth*10 to +WorldWidth*10.
	 *
	 *	OBSERVATION 2:
	 *		More likely, it'll range in x * WorldWidth where
	 *		-1 < x < 1.
	 *
	 *	OBSERVATION 3:
	 *		The problem is the same in 1 dimension as it is in 2.
	 *
	 *	OBSERVATION 4:
	 *		We can probably do some kind of progressive scan.
	 *
	 *	OBSERVATION 5:
	 *		That progressive scan probably starts by stepping
	 *		SpriteMax.X in decrements of WorldWidth until
	 *		SpriteMax.X < CameraMin.X.
	 *
	 *	OBSERVATION 6:
	 *		We then follow this algorithm:
	 *			Increment SpriteMax and SpriteMin by WorldWidth
	 *			If SpriteMin is less than CameraMax, then the
	 *				sprite is on screen and needs to be drawn.
	 *			If SpriteMin is greater than CameraMax, break.
	 *			Otherwise, loop.
	 *
	 *	OBSERVATION 7:
	 *		2D-izing this problem means storing a list of
	 *		X-Ranges and a list of Y-Ranges, and then blitting
	 *		at every available combination of those.
	 *
	 *	OBSERVATION 8:
	 *		This doesn't support scaling (yet), but I think the
	 *		approach is one I can add to later.
	 *
	 */

	const auto cameraAABB = _Camera.GetCameraView();
	//@TODO: This doesn't change between draws, only between frames. Cache it?
	const auto cameraScale = _Camera.GetCameraScale();

	auto spriteStartMinX = static_cast<float>(transform.Position.x);
	auto spriteStartMaxX = static_cast<float>(transform.Position.w) + spriteStartMinX;

	auto spriteMinY = static_cast<float>(transform.Position.y);
	auto spriteMaxY = static_cast<float>(transform.Position.h) + spriteMinY;

	// Scroll backwards until we go "too far".
	//We step forward one at the beginning of the next step.
	while(spriteMaxY > cameraAABB.top)
	{
		spriteMaxY -= _GameFieldHeight;
		spriteMinY -= _GameFieldHeight;
	}

	while(spriteStartMaxX > cameraAABB.left)
	{
		spriteStartMaxX -= _GameFieldWidth;
		spriteStartMinX -= _GameFieldWidth;
	}

	while(true)
	{
		auto newPos = transform.Position;

		spriteMinY += _GameFieldHeight;

		if(spriteMinY < cameraAABB.bottom)
		{
			auto spriteMinX = spriteStartMinX;

			while(true)
			{
				spriteMinX += _GameFieldWidth;

				if(spriteMinX < cameraAABB.right)
				{
					const auto spriteMin = _Camera.WorldToCamera(Vector2(spriteMinX, spriteMinY));

					newPos.x = spriteMin.x;
					newPos.y = spriteMin.y;
					newPos.w *= cameraScale;
					newPos.h *= cameraScale;
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


#else

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
#endif
}


void
RenderQueue::DrawAtTop(const SpriteTransform& transform, const AABB& screenAABB)
{
	auto newPos = transform.Position;
	newPos.y -= static_cast<int>(floor(screenAABB.bottom));
	EnqueueScreenSpace(transform.ID, newPos, transform.Rotation, transform.Layer);
}

void
RenderQueue::DrawAtBottom(const SpriteTransform& transform, const AABB& screenAABB)
{
	auto newPos = transform.Position;
	newPos.y += static_cast<int>(floor(screenAABB.bottom));
	EnqueueScreenSpace(transform.ID, newPos, transform.Rotation, transform.Layer);
}

void
RenderQueue::DrawAtLeft(const SpriteTransform& transform, const AABB& screenAABB)
{
	auto newPos = transform.Position;
	newPos.x -= static_cast<int>(floor(screenAABB.right));
	EnqueueScreenSpace(transform.ID, newPos, transform.Rotation, transform.Layer);
}

void
RenderQueue::DrawAtRight(const SpriteTransform& transform, const AABB& screenAABB)
{
	auto newPos = transform.Position;
	newPos.x += static_cast<int>(floor(screenAABB.right));
	EnqueueScreenSpace(transform.ID, newPos, transform.Rotation, transform.Layer);
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
