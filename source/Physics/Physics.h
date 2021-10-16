#pragma once

#include <vector>
#include <set>
#include <future>

#include "../Math/AABB.h"

#include "../ECS/Entity.h"
#include "../ECS/Transform.h"
#include "../ECS/Rigidbody.h"

#include "ColliderType.h"
#include "MoveList.h"

class TransformManager;
class RigidbodyManager;

class Physics
{
public:
	Physics(TransformManager& transformManager, RigidbodyManager& rigidbodyManager, const AABB& screenAABB);

	void Enqueue(const Rigidbody& rb, const float& deltaTime);

	void Simulate(const float& deltaTime);

	struct CollisionListEntry
	{
		Entity A;
		Entity B;
		ColliderType EntityAType;
		ColliderType EntityBType;
		float MassA;
		float MassB;
		float TimeOfCollision;

		bool operator<(const CollisionListEntry& other) const
		{
			return (this->TimeOfCollision < other.TimeOfCollision);
		}

		bool operator==(const CollisionListEntry& other) const
		{
			return A == other.A &&
				B == other.B &&
				fabs(TimeOfCollision - other.TimeOfCollision) < FLT_EPSILON;
		}
	};

	const std::vector<CollisionListEntry>& GetCollisionReport() const
	{
		return _CollisionReport;
	};

private:

	struct ResolvedListEntry
	{
		Entity Entity;
		Vector2 Position;
		Vector2 Velocity;
		float AngularVelocity = 0;
		float Time            = 0;
	};

	// Physics Pipeline

	std::vector<CollisionListEntry> DetectInitialCollisions(MoveList& moveList, const float& deltaTime) const;

	void RemoveDuplicateCollisions();

	void DetectSecondaryCollisions(std::vector<ResolvedListEntry> resolvedThisIteration);

	std::vector<ResolvedListEntry> ResolveUpdatedMovement(const float& deltaTime);

	std::array<ResolvedListEntry, 2> ResolveMove(const float& deltaTime, CollisionListEntry collision) const;

	void FinalizeMoves(const float& deltaTime);

	void End();


	// OBB Collisions

	void ShipVsAsteroid(const MoveList::ColliderRanges& ranges, std::vector<CollisionListEntry>& collisions) const;
	static void OBBVsSpecificAsteroid(const OBB& ship,
	                                  const Entity& shipEntity,
	                                  std::vector<MoveList::Entry>::iterator asteroidBegin,
	                                  std::vector<MoveList::Entry>::iterator asteroidEnd,
	                                  const float& asteroidRadius,
	                                  std::vector<CollisionListEntry>& Collisions);


	// Circle Collisions

	static void BulletVsAsteroid(const MoveList::ColliderRanges& ranges,
	                             std::vector<CollisionListEntry>& collisions,
	                             const float& deltaTime);
	static void AsteroidVsAsteroid(const MoveList::ColliderRanges& Ranges,
	                               std::vector<CollisionListEntry>& Collisions,
	                               const float& DeltaTime);

	static void CircleVsCircles(const MoveList::Entry& Circle,
	                            const float& CircleRadius,
	                            const float& CircleMass,
	                            std::vector<MoveList::Entry>::iterator CirclesBegin,
	                            std::vector<MoveList::Entry>::iterator CirclesEnd,
	                            const float& CirclesMass,
	                            const float& RadiusPlusRadiusSquared,
	                            const float& DeltaTime,
	                            const ColliderType& TypeA,
	                            const ColliderType& TypeB,
	                            std::vector<CollisionListEntry>& Collisions);

	static float GetMassFromColliderType(const ColliderType& type);

	static const int MAX_SOLVER_ITERATIONS = 3;
	inline static const float ASTEROID_MASSES[] { 16.0f, 4.0f, 1.0f };

	TransformManager& _TransformManager;
	RigidbodyManager& _RigidbodyManager;

	const AABB _ScreenAABB;

	static const int CHUNKS_X    = 6;
	static const int CHUNKS_Y    = 4;
	static const int CHUNK_COUNT = CHUNKS_X * CHUNKS_Y;

	const float _ChunkSizeX;
	const float _ChunkSizeY;

	// The entrypoint for the physics system. Entries are enqueued into a MoveList when they
	// request a move from the system during the frame.
	std::array<MoveList, CHUNK_COUNT> _MoveLists;

	// Workers responsible for DetectInitialCollisions
	std::array<std::future<std::vector<CollisionListEntry>>, CHUNK_COUNT> _Workers;

	// Result from DetectInitialCollisions
	std::vector<CollisionListEntry> _CollisionList;

	// A list of all collisions that took place so that gameplay code can react.
	std::vector<CollisionListEntry> _CollisionReport;

	std::vector<ResolvedListEntry> _ResolvedList;

	std::set<Entity> _DirtyList;
};


namespace std
{
template <> struct hash<Physics::CollisionListEntry>
{
	inline size_t operator()(const Physics::CollisionListEntry& entry) const noexcept
	{
		return entry.A.Hash() ^ entry.B.Hash();
	}
};
}
