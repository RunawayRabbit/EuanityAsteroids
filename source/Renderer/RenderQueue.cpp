#pragma once

#include <algorithm>

#include "RenderQueue.h"
#include "SpriteID.h"
#include "Sprite.h"
#include "SpriteTransform.h"

#include "..\Math\OBB.h"
#include "..\Math\AABB.h"


RenderQueue::RenderQueue(Renderer& renderer, int screenWidth, int screenHeight) :
	spriteAtlas(renderer),
	screenWidth(screenWidth),
	screenHeight(screenHeight)
{
	renderQueue.reserve(128); // arbitrary, but a decent starting size for total number of rendered sprites?)
}


void RenderQueue::Enqueue(SpriteID spriteID, const SDL_Rect& targetRect, const float rotation, const RenderQueue::Layer layer)
{
	//@NOTE: I used to do some complex queueing here where sorted insertion became O(log k) where k is the number of layers in use.
	// It turns out that having an O(1) insert and doing the sort at the end is faster, since we enqueue far more often than we
	// fetch.
	Sprite sprite = spriteAtlas.Get(spriteID);

	Element el;
	el.tex = sprite.texture;
	el.srcRect = sprite.source;
	el.dstRect = targetRect;
	el.angle = rotation;
	el.layer = layer;

	renderQueue.push_back(el);
}

void RenderQueue::Enqueue(SpriteID spriteID, const float rotation, const RenderQueue::Layer layer)
{
	SDL_Rect targetRect;
	targetRect.w = screenWidth;
	targetRect.h = screenHeight;
	targetRect.x = 0;
	targetRect.y = 0;
	Enqueue(spriteID, targetRect, rotation, layer);
}

void RenderQueue::EnqueueLooped(const SpriteTransform& transform)
{
	AABB screenAABB(Vector2::zero(), Vector2((float)screenWidth, (float)screenHeight));
	OBB spriteOBB = OBB(transform.position, transform.rotation);
	if (!screenAABB.FullyContains(spriteOBB))
	{
		// Sprite is visible on the opposite side. Determine which!
		AABB spriteAABB = spriteOBB.Bounds();

		/*
			PRETTY PICTURE TO VISUALIZE CASES
				A |  B | C
			  ____|____|____
				D |    | E
			  ____|____|____
			    F |  G | H
				  |    |
		*/

		if (spriteAABB.top < screenAABB.top)
		{
			// Case A - B - C
			DrawAtBottom(transform, screenAABB);

			if (spriteAABB.left < screenAABB.left)
			{
				// Case A
				DrawAtRight(transform, screenAABB);

				// DrawAtBottomRight
				SDL_Rect newPos = transform.position;
				newPos.y += (int)floor(screenAABB.bottom);
				newPos.x += (int)floor(screenAABB.right);
				Enqueue(transform.id, newPos, transform.rotation, transform.layer);
			}
			else if (spriteAABB.right > screenAABB.right)
			{
				// Case C
				DrawAtLeft(transform, screenAABB);

				// DrawAtBottomLeft
				SDL_Rect newPos = transform.position;
				newPos.y += (int)floor(screenAABB.bottom);
				newPos.x -= (int)floor(screenAABB.right);
				Enqueue(transform.id, newPos, transform.rotation, transform.layer);
			}
		}
		else if (spriteAABB.bottom > screenAABB.bottom)
		{
			// Case F - G - H
			DrawAtTop(transform, screenAABB);
			if (spriteAABB.left < screenAABB.left)
			{
				// Case F
				DrawAtRight(transform, screenAABB);

				// DrawAtTopRight

				SDL_Rect newPos = transform.position;
				newPos.y -= (int)floor(screenAABB.bottom);
				newPos.x += (int)floor(screenAABB.right);
				Enqueue(transform.id, newPos, transform.rotation, transform.layer);
			}
			else if (spriteAABB.right > screenAABB.right)
			{
				// Case H
				DrawAtLeft(transform, screenAABB);

				// DrawAtTopLeft

				SDL_Rect newPos = transform.position;
				newPos.y -= (int)floor(screenAABB.bottom);
				newPos.x -= (int)floor(screenAABB.right);
				Enqueue(transform.id, newPos, transform.rotation, transform.layer);
			}
		}
		else
		{
			// Case D - E
			if (spriteAABB.left < screenAABB.left)
			{
				// Case D
				DrawAtRight(transform, screenAABB);
			}
			else if (spriteAABB.right > screenAABB.right)
			{
				// Case E
				DrawAtLeft(transform, screenAABB);
			}
		}
	}

	// Render at the original position
	Enqueue(transform.id, transform.position, transform.rotation, transform.layer);
}


void RenderQueue::DrawAtTop(const SpriteTransform& transform, const AABB& screenAABB)
{
	SDL_Rect newPos = transform.position;
	newPos.y -= (int)floor(screenAABB.bottom);
	Enqueue(transform.id, newPos, transform.rotation, transform.layer);

}
void RenderQueue::DrawAtBottom(const SpriteTransform& transform, const AABB& screenAABB)
{
	SDL_Rect newPos = transform.position;
	newPos.y += (int)floor(screenAABB.bottom);
	Enqueue(transform.id, newPos, transform.rotation, transform.layer);
}

void RenderQueue::DrawAtLeft(const SpriteTransform& transform, const AABB& screenAABB)
{
	SDL_Rect newPos = transform.position;
	newPos.x -= (int)floor(screenAABB.right);
	Enqueue(transform.id, newPos, transform.rotation, transform.layer);
}
void RenderQueue::DrawAtRight(const SpriteTransform& transform, const AABB& screenAABB)
{
	SDL_Rect newPos = transform.position;
	newPos.x += (int)floor(screenAABB.right);
	Enqueue(transform.id, newPos, transform.rotation, transform.layer);
}



// @TODO: Consider locking here and unlocking on clear. Rename methods to be explicit if you do so.
const std::vector<RenderQueue::Element>& RenderQueue::GetRenderQueue()
{
	std::sort(renderQueue.begin(), renderQueue.end(),
		[](const RenderQueue::Element& a, const RenderQueue::Element& b) -> bool
		{
			return a.layer < b.layer;
		});

	return renderQueue;
}

void RenderQueue::Clear()
{
	renderQueue.clear();
}