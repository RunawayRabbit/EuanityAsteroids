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
	Physics(TransformManager& TransformManager, RigidbodyManager& RigidbodyManager, const AABB& ScreenAabb);

	void Enqueue(const Rigidbody& Rb, const float& DeltaTime);
	void Simulate(const float& DeltaTime);

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
		float Time = 0;
	};

	// Physics Pipeline

	std::vector<Physics::CollisionListEntry> DetectInitialCollisions(MoveList& moveList, const float& deltaTime) const;
	void DetectSecondaryCollisions(const std::vector<Physics::ResolvedListEntry> resolvedThisIteration);
	std::vector<Physics::ResolvedListEntry> ResolveUpdatedMovement(const float& DeltaTime);
	std::array<Physics::ResolvedListEntry, 2> ResolveMove(const float& deltaTime, CollisionListEntry collision) const;
	void FinalizeMoves(const float& deltaTime);
	void End();


	// OBB Collisions

	void ShipVsAsteroid(const MoveList::ColliderRanges& ranges, std::vector<CollisionListEntry>& collisions) const;
	static void OBBVsSpecificAsteroid(const OBB& Ship, const Entity& ShipEntity, std::vector<MoveList::Entry>::iterator AsteroidBegin,
	                                  std::vector<MoveList::Entry>::iterator AsteroidEnd, const float& AsteroidRadius, std::vector<CollisionListEntry>& Collisions);


	// Circle Collisions

	static void BulletVsAsteroid(const MoveList::ColliderRanges& Ranges, std::vector<CollisionListEntry>& Collisions, const float& DeltaTime);
	static void AsteroidVsAsteroid(const MoveList::ColliderRanges& Ranges, std::vector<CollisionListEntry>& Collisions, const float& DeltaTime);

	static void CircleVsCircles(const MoveList::Entry& Circle, const float& CircleRadius, const float& CircleMass,
	                            std::vector<MoveList::Entry>::iterator CirclesBegin, std::vector<MoveList::Entry>::iterator CirclesEnd,
	                            const float& CirclesMass, const float& RadiusPlusRadiusSquared, const float& DeltaTime, const ColliderType& TypeA,
	                            const ColliderType& TypeB, std::vector<CollisionListEntry>& Collisions);

	static float GetMassFromColliderType(const ColliderType& Type);

	static const int MAX_SOLVER_ITERATIONS = 3;
	inline static const float ASTEROID_MASSES[]{ 16.0f, 4.0f, 1.0f };

	TransformManager& _TransformManager;
	RigidbodyManager& _RigidbodyManager;

	const AABB _ScreenAABB;

	static const int CHUNKS_X = 8;
	static const int CHUNKS_Y = 6;
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
