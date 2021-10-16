#include <unordered_set>
#include <algorithm> // for min and max

#include "Physics.h"
#include "ColliderType.h"
#include "CollisionTests.h"

#include "../ECS/RigidbodyManager.h"
#include "../ECS/TransformManager.h"

#include "../Math/Math.h"
#include "../Math/OBB.h"
#include "../Math/Vector2.h"

Physics::Physics(TransformManager& transformManager, RigidbodyManager& rigidbodyManager, const AABB& screenAABB)
	: _TransformManager(transformManager),
	  _RigidbodyManager(rigidbodyManager),
	  _ScreenAABB(screenAABB),
	  _ChunkSizeX(screenAABB.max.x / CHUNKS_X),
	  _ChunkSizeY(screenAABB.max.y / CHUNKS_Y),
	  _MoveLists()
{
}

void
Physics::Enqueue(const Rigidbody& rb, const float& deltaTime)
{
	// Get an AABB for the rigidbody using it's transform

	auto optionalRbTrans = _TransformManager.Get(rb.entity);
	if(!optionalRbTrans.has_value())
	{
		//@TODO: Error check in case of RB with no Transform?
	}

	const auto rbTrans = optionalRbTrans.value();
	auto rbAABB        = ColliderRadius::GetAABB(rb.colliderType, rbTrans.pos);

	// Pad the AABB by the velocity, and a small safety margin.
	const auto deltaPosition = rb.velocity * deltaTime;

	const auto padding = 15.0f;

	rbAABB.min.x = std::min(rbAABB.min.x, rbAABB.min.x + deltaPosition.x - padding);
	rbAABB.min.y = std::min(rbAABB.min.y, rbAABB.min.y + deltaPosition.y - padding);
	rbAABB.max.x = std::max(rbAABB.max.x, rbAABB.max.x + deltaPosition.x + padding);
	rbAABB.max.y = std::max(rbAABB.max.y, rbAABB.max.y + deltaPosition.y + padding);

	auto accumulator = Vector2::zero();
	auto startTileX  = -1;
	auto startTileY  = -1;
	while(accumulator.x < rbAABB.min.x)
	{
		accumulator.x += _ChunkSizeX;
		++startTileX;
	}
	while(accumulator.y < rbAABB.min.y)
	{
		accumulator.y += _ChunkSizeY;
		++startTileY;
	}
	// @NOTE: Logic here is a bit tricky. At this point, startTile indicates the
	//  "lowest" tile that the AABB overlaps, and accumulator is sitting in the
	//  maximum corner of that tile.

	// Using the same method, we advance the max accumulator to find the maximum.
	auto endTileX = startTileX;
	auto endTileY = startTileY;
	while(accumulator.x < rbAABB.max.x)
	{
		accumulator.x += _ChunkSizeX;
		++endTileX;
	}
	while(accumulator.y < rbAABB.max.y)
	{
		accumulator.y += _ChunkSizeY;
		++endTileY;
	}

	// Enqueue the rigidbody into the appropriate moveLists by the chunk it's in.
	for(auto y = startTileY; y <= endTileY; ++y)
	{
		// Wrap our Y coordinate if we have to.
		auto wrappedY = rbTrans.pos.y;
		if(y < 0)
			wrappedY += _ScreenAABB.max.y;
		else if(y >= CHUNKS_Y)
			wrappedY -= _ScreenAABB.max.y;

		for(auto X = startTileX; X <= endTileX; ++X)
		{
			// Wrap the element in X if we have to.
			auto wrappedX = rbTrans.pos.x;
			if(X < 0)
				wrappedX += _ScreenAABB.max.x;
			else if(X >= CHUNKS_X)
				wrappedX -= _ScreenAABB.max.x;

			// Calculate the chunk index and enqueue
			auto Mod              = [](const int A, const int B) { return (B + (A % B)) % B; };
			const auto chunkIndex = Mod(y, CHUNKS_Y) * CHUNKS_X + Mod(X, CHUNKS_X);
			_MoveLists[chunkIndex].Enqueue({ rb, Vector2(wrappedX, wrappedY) });
		}
	}
}

void
Physics::Simulate(const float& deltaTime)
{
	_CollisionReport.clear(); // Clear last frame's report.

	// Start our worker threads churning through the Initial collision tests.
	for(auto i = 0; i < CHUNK_COUNT; ++i)

		_Workers[i] = std::async([this, i, deltaTime]() -> std::vector<CollisionListEntry>
		{
			return DetectInitialCollisions(_MoveLists[i], deltaTime);
		});

	for(auto& worker : _Workers)
	{
		auto result = worker.get();
		_CollisionList.reserve(_CollisionList.size() + result.size());
		_CollisionList.insert(_CollisionList.end(), result.begin(), result.end());
	}

	RemoveDuplicateCollisions();

	if(_CollisionList.size() > 0) // Most frames feature zero collisions.
	{
		auto solverIterations = 0;
		do
		{
			// From each collision, generate new moves based on physical interactions
			auto resolvedThisIteration = ResolveUpdatedMovement(deltaTime);

			// Clear the collision list and the dirty set.
			_CollisionReport.reserve(_CollisionReport.size() + _CollisionList.size());
			_CollisionReport.insert(_CollisionReport.end(), _CollisionList.begin(), _CollisionList.end());
			_CollisionList.clear();
			_DirtyList.clear();

			//  Check the NEW elements in resolved list against the full move list, looking for further collisions.
			DetectSecondaryCollisions(resolvedThisIteration);

			// Step 7.5. Integrate the new resolved entries into the list.
			_ResolvedList.reserve(_ResolvedList.size() + resolvedThisIteration.size());
			_ResolvedList.insert(_ResolvedList.end(), resolvedThisIteration.begin(), resolvedThisIteration.end());

			// Step 8. If the collision list is empty, continue. Otherwise, loop..
			++solverIterations;
		} while(_CollisionList.size() > 0 && solverIterations < MAX_SOLVER_ITERATIONS);

		std::sort(_ResolvedList.begin(), _ResolvedList.end(),
		          [](const ResolvedListEntry& a, ResolvedListEntry& b) -> bool
		          {
			          return a.Time < b.Time;
		          });
	}

	FinalizeMoves(deltaTime);
	End();
}

void
Physics::End()
{
	for(auto& moveList : _MoveLists)
		moveList.Clear();

	_CollisionList.clear();
	_ResolvedList.clear();
	_DirtyList.clear();
}

std::vector<Physics::CollisionListEntry>
Physics::DetectInitialCollisions(MoveList& moveList, const float& deltaTime) const
{
	std::vector<CollisionListEntry> collisions;
	if(moveList.Size() == 0)
		return collisions;

	// Sort to get our MoveList in order.
	std::sort(moveList.begin(), moveList.end(), [](const MoveList::Entry& a, const MoveList::Entry& b) -> bool
	{
		return a.Rb.colliderType < b.Rb.colliderType;
	});

	// The order of these has to match the order of enum ColliderType defined in ColliderType.h.
	MoveList::ColliderRanges ranges;
	ranges.ShipBegin = moveList.begin();
	ranges.ShipEnd   = ranges.BulletBegin = ranges.ShipBegin + moveList.GetColliderCount(ColliderType::SHIP);
	ranges.BulletEnd = ranges.LargeBegin  = ranges.BulletBegin + moveList.GetColliderCount(ColliderType::BULLET);
	ranges.LargeEnd  = ranges.MediumBegin = ranges.LargeBegin + moveList.GetColliderCount(ColliderType::LARGE_ASTEROID);
	ranges.MediumEnd = ranges.SmallBegin  = ranges.MediumBegin + moveList.GetColliderCount(ColliderType::MEDIUM_ASTEROID);
	ranges.SmallEnd  = moveList.end();

	ShipVsAsteroid(ranges, collisions);
	BulletVsAsteroid(ranges, collisions, deltaTime);
	AsteroidVsAsteroid(ranges, collisions, deltaTime);

	return collisions;
}

void
Physics::RemoveDuplicateCollisions()
{
	//@TODO: Does this mean we should be storing the collisions as sets to begin with? How does that perform when we join the threads at the start?
	//@NOTE: In testing this actually turned out to be the fastest method.
	std::unordered_set<CollisionListEntry> tempSet;

	tempSet.reserve(_CollisionList.size());

	for(auto& collisionEntry : _CollisionList)
		tempSet.insert(collisionEntry);

	_CollisionList.assign(tempSet.begin(), tempSet.end());

	//@NOTE: @BUGFIX: This sort is "necessary" to prevent render order issues in the case where asteroids overlap.
	// in the case of "perfect" physics, we shouldn't ever have overlaps. But we decided to allow them and to be as
	// graceful as we possibly can be when it *does* happen.
	sort(_CollisionList.begin(), _CollisionList.end());
}


void
Physics::ShipVsAsteroid(const MoveList::ColliderRanges& ranges, std::vector<CollisionListEntry>& collisions) const
{
	for(auto ship = ranges.ShipBegin; ship != ranges.ShipEnd; ++ship)
	{
		auto optionalShipTrans = _TransformManager.Get(ship->Rb.entity);
		if(!optionalShipTrans.has_value())
			// @TODO: Are you ever going to write that logging module? Because this should be logged.
			continue;
		auto [shipPos, shipRot] = optionalShipTrans.value();
		OBB playerOBB(shipPos, ColliderRadius::Ship, shipRot);

		OBBVsSpecificAsteroid(playerOBB, ship->Rb.entity, ranges.LargeBegin, ranges.LargeEnd, ColliderRadius::Large, collisions);
		OBBVsSpecificAsteroid(playerOBB, ship->Rb.entity, ranges.MediumBegin, ranges.MediumEnd, ColliderRadius::Medium, collisions);
		OBBVsSpecificAsteroid(playerOBB, ship->Rb.entity, ranges.SmallBegin, ranges.SmallEnd, ColliderRadius::Small, collisions);
	}
}

void
Physics::OBBVsSpecificAsteroid(const OBB& ship,
                               const Entity& shipEntity,
                               const std::vector<MoveList::Entry>::iterator asteroidBegin,
                               const std::vector<MoveList::Entry>::iterator asteroidEnd,
                               const float& asteroidRadius,
                               std::vector<CollisionListEntry>& Collisions)
{
	for(auto Asteroid = asteroidBegin; Asteroid != asteroidEnd; ++Asteroid)
	{
		Circle collider(Asteroid->Pos, asteroidRadius);
		if(CollisionTests::OBBToCircle(ship, collider))
		{
			CollisionListEntry entry;
			entry.A           = shipEntity;
			entry.EntityAType = ColliderType::SHIP;
			entry.MassA       = GetMassFromColliderType(ColliderType::SHIP);
			entry.B           = Asteroid->Rb.entity;
			entry.EntityBType = Asteroid->Rb.colliderType;
			entry.MassB       = GetMassFromColliderType(Asteroid->Rb.colliderType);

			entry.TimeOfCollision = 0.0f; // Made-up.

			Collisions.push_back(entry);
		}
	}
}

float
Physics::GetMassFromColliderType(const ColliderType& type)
{
	switch(type)
	{
		case ColliderType::LARGE_ASTEROID: return ASTEROID_MASSES[0];
		case ColliderType::MEDIUM_ASTEROID: return ASTEROID_MASSES[1];
		case ColliderType::SMOL_ASTEROID: return ASTEROID_MASSES[2];

		case ColliderType::SHIP: return ASTEROID_MASSES[1] / 2;
		default: return 0.0f;
	}
}

void
Physics::BulletVsAsteroid(const MoveList::ColliderRanges& ranges,
                          std::vector<CollisionListEntry>& collisions,
                          const float& deltaTime)
{
	constexpr float bulletMass = 0; // Fuck it, bullets don't have mass. I have decided this.

	for(auto bullet = ranges.BulletBegin; bullet != ranges.BulletEnd; ++bullet)
	{
		const auto bulletVsLargeRadius =
			(ColliderRadius::Bullet + ColliderRadius::Large) *
			(ColliderRadius::Bullet + ColliderRadius::Large);
		CircleVsCircles(*bullet, ColliderRadius::Bullet, bulletMass,
		                ranges.LargeBegin, ranges.LargeEnd,
		                ASTEROID_MASSES[0], bulletVsLargeRadius, deltaTime,
		                ColliderType::BULLET, ColliderType::LARGE_ASTEROID,
		                collisions);

		const auto BulletVsMediumRadius =
			(ColliderRadius::Bullet + ColliderRadius::Medium) *
			(ColliderRadius::Bullet + ColliderRadius::Medium);
		CircleVsCircles(*bullet, ColliderRadius::Bullet, bulletMass,
		                ranges.MediumBegin, ranges.MediumEnd,
		                ASTEROID_MASSES[1], BulletVsMediumRadius, deltaTime,
		                ColliderType::BULLET, ColliderType::MEDIUM_ASTEROID,
		                collisions);

		const auto BulletVsSmallRadius =
			(ColliderRadius::Bullet + ColliderRadius::Small) *
			(ColliderRadius::Bullet + ColliderRadius::Small);

		CircleVsCircles(*bullet, ColliderRadius::Bullet, bulletMass,
		                ranges.SmallBegin, ranges.SmallEnd,
		                ASTEROID_MASSES[2], BulletVsSmallRadius, deltaTime,
		                ColliderType::BULLET, ColliderType::SMOL_ASTEROID,
		                collisions);
	}
}

void
Physics::AsteroidVsAsteroid(const MoveList::ColliderRanges& Ranges,
                            std::vector<CollisionListEntry>& Collisions,
                            const float& DeltaTime)
{
	for(auto Large = Ranges.LargeBegin; Large != Ranges.LargeEnd; ++Large)
	{
		// Large Vs Large
		const auto LargeVsLargeSqRadius =
			(ColliderRadius::Large + ColliderRadius::Large) *
			(ColliderRadius::Large + ColliderRadius::Large);

		// @NOTE: starting the range at large+1 guarantees that we don't check A against itself,
		// and that we don't repeat test pairs that have already been computed.
		CircleVsCircles(*Large, ColliderRadius::Large, ASTEROID_MASSES[0],
		                Large + 1, Ranges.LargeEnd,
		                ASTEROID_MASSES[0], LargeVsLargeSqRadius, DeltaTime,
		                ColliderType::LARGE_ASTEROID, ColliderType::LARGE_ASTEROID,
		                Collisions);


		// Large Vs Medium
		const auto LargeVsMediumSqRadius =
			(ColliderRadius::Large + ColliderRadius::Medium) *
			(ColliderRadius::Large + ColliderRadius::Medium);

		CircleVsCircles(*Large, ColliderRadius::Large, ASTEROID_MASSES[0],
		                Ranges.MediumBegin, Ranges.MediumEnd,
		                ASTEROID_MASSES[1], LargeVsMediumSqRadius, DeltaTime,
		                ColliderType::LARGE_ASTEROID, ColliderType::MEDIUM_ASTEROID,
		                Collisions);


		// Large Vs Small
		const auto LargeVsSmallSqRadius =
			(ColliderRadius::Large + ColliderRadius::Small) *
			(ColliderRadius::Large + ColliderRadius::Small);

		CircleVsCircles(*Large, ColliderRadius::Large, ASTEROID_MASSES[0],
		                Ranges.SmallBegin, Ranges.SmallEnd,
		                ASTEROID_MASSES[2], LargeVsSmallSqRadius, DeltaTime,
		                ColliderType::LARGE_ASTEROID, ColliderType::SMOL_ASTEROID,
		                Collisions);
	}

	for(auto Medium = Ranges.MediumBegin; Medium != Ranges.MediumEnd; ++Medium)
	{
		// Medium Vs Medium
		const auto MediumVsMediumSqRadius =
			(ColliderRadius::Medium + ColliderRadius::Medium) *
			(ColliderRadius::Medium + ColliderRadius::Medium);

		// @NOTE: starting the range at medium+1 guarantees that we don't check A against itself,
		// and that we don't repeat test pairs that have already been computed.
		CircleVsCircles(*Medium, ColliderRadius::Medium, ASTEROID_MASSES[1],
		                Medium + 1, Ranges.MediumEnd,
		                ASTEROID_MASSES[1], MediumVsMediumSqRadius, DeltaTime,
		                ColliderType::MEDIUM_ASTEROID, ColliderType::MEDIUM_ASTEROID,
		                Collisions);


		// Medium Vs Small
		const auto MediumVsSmallSqRadius =
			(ColliderRadius::Medium + ColliderRadius::Small) *
			(ColliderRadius::Medium + ColliderRadius::Small);

		CircleVsCircles(*Medium, ColliderRadius::Medium, ASTEROID_MASSES[1],
		                Ranges.SmallBegin, Ranges.SmallEnd,
		                ASTEROID_MASSES[2], MediumVsSmallSqRadius, DeltaTime,
		                ColliderType::MEDIUM_ASTEROID, ColliderType::SMOL_ASTEROID,
		                Collisions);
	}


	for(auto Small = Ranges.SmallBegin; Small != Ranges.SmallEnd; ++Small)
	{
		// Small Vs Small
		const auto SmallVsSmallSqRadius =
			(ColliderRadius::Small + ColliderRadius::Small) *
			(ColliderRadius::Small + ColliderRadius::Small);

		// @NOTE: starting the range at small+1 guarantees that we don't check A against itself,
		// and that we don't repeat test pairs that have already been computed.
		CircleVsCircles(*Small, ColliderRadius::Small, ASTEROID_MASSES[2],
		                Small + 1, Ranges.SmallEnd,
		                ASTEROID_MASSES[2], SmallVsSmallSqRadius, DeltaTime,
		                ColliderType::SMOL_ASTEROID, ColliderType::SMOL_ASTEROID,
		                Collisions);
	}
}


void
Physics::CircleVsCircles(const MoveList::Entry& Circle,
                         const float& CircleRadius,
                         const float& CircleMass,
                         const std::vector<MoveList::Entry>::iterator CirclesBegin,
                         const std::vector<MoveList::Entry>::iterator CirclesEnd,
                         const float& CirclesMass,
                         const float& RadiusPlusRadiusSquared,
                         const float& DeltaTime,
                         const ColliderType& TypeA,
                         const ColliderType& TypeB,
                         std::vector<CollisionListEntry>& Collisions)
{
	// Check our first circle against every circle in the range that we were passed
	for(auto CircleB = CirclesBegin; CircleB != CirclesEnd; ++CircleB)
	{
		if(Circle.Rb.entity == CircleB->Rb.entity)
			continue;

		float TimeOfCollision;
		if(CollisionTests::SweptCircleToCircle(
			Circle.Pos, Circle.Rb.velocity,
			CircleB->Pos, CircleB->Rb.velocity,
			CircleRadius, RadiusPlusRadiusSquared, DeltaTime, TimeOfCollision))
		{
			CollisionListEntry Col;
			Col.A           = Circle.Rb.entity;
			Col.EntityAType = TypeA;
			Col.MassA       = CircleMass;

			Col.B           = CircleB->Rb.entity;
			Col.EntityBType = TypeB;
			Col.MassB       = CirclesMass;

			Col.TimeOfCollision = TimeOfCollision;
			Collisions.push_back(Col);
		}
	}
}


std::vector<Physics::ResolvedListEntry>
Physics::ResolveUpdatedMovement(const float& deltaTime)
{
	std::vector<ResolvedListEntry> Resolved;

	// Start with the earliest collision and resolve the new motion and add the entities that collided to a "dirty set".
	std::sort(_CollisionList.begin(), _CollisionList.end());
	for(auto& Collision : _CollisionList)
		// God I really want std::set.contains()...
		// "If dirtyList doesn't contain either A or B..
		if(_DirtyList.find(Collision.A) == _DirtyList.end() &&
			_DirtyList.find(Collision.B) == _DirtyList.end())
		{
			// Continue resolving collisions, but skip any that involve objects in the dirty set.
			auto Moves = ResolveMove(deltaTime, Collision);
			Resolved.push_back(Moves[0]);
			Resolved.push_back(Moves[1]);

			_DirtyList.insert(Collision.A);
			_DirtyList.insert(Collision.B);
		}

	return Resolved;
}


std::array<Physics::ResolvedListEntry, 2>
Physics::ResolveMove(const float& deltaTime, const CollisionListEntry collision) const
{
	// This is probably all kinds of wrong, but I have never studied physics
	// so I'm just sort of making this up as I go along, helped with some
	// frankly crappy youtube videos.

	// https://www.youtube.com/watch?v=Dww4ArU5JF8

	const auto transA = _TransformManager.GetMutable(collision.A).value();
	Rigidbody* rigidA;
	_RigidbodyManager.GetMutable(collision.A, rigidA);

	const auto transB = _TransformManager.GetMutable(collision.B).value();
	Rigidbody* rigidB;
	_RigidbodyManager.GetMutable(collision.B, rigidB);

	const auto startPosA = transA->pos + rigidA->velocity * (collision.TimeOfCollision * deltaTime);
	const auto startPosB = transB->pos + rigidB->velocity * (collision.TimeOfCollision * deltaTime);

	auto relPos = startPosB - startPosA;

	// Split the problem into two parts: the component normal to the collision
	// and the component tangential to the collision.

	const auto impactNormal  = relPos.normalized();
	const auto impactTangent = impactNormal.Rot90CW();

	// Conservation of Momentum (tangential)
	// Dot(impactTangent, A.velocity) = Dot(impactTangent, endVelA);
	// Dot(impactTangent, B.velocity) = Dot(impactTangent, endVelB);
	const auto finalATangent = Dot(rigidA->velocity, impactTangent); // A.vel * cos(theta)
	const auto finalBTangent = Dot(rigidB->velocity, impactTangent); // B.vel * cos(theta)

	// @NOTE: I am most likely mis-using this term here.
	const float e = 0.95f; // Coefficient of restitution
	// e = B.Velocity - A.Velocity / endVelB - endVelA

	// Conservation of Momentum (normal)
	const auto normalA = Dot(rigidA->velocity, impactNormal); // A.vel * sin(theta)
	const auto normalB = Dot(rigidB->velocity, impactNormal); // B.vel * sin(theta)
	// massA * normalA + massB * normalB = massA * finalANormal + massB * finalBNormal;

	// Did the algebra and this is what fell out.

	auto finalBNormal =
		normalA * collision.MassA * e -
		normalB * collision.MassA * e +
		normalA * collision.MassA + normalB * collision.MassB;

	finalBNormal /= collision.MassA + collision.MassB;

	const auto finalANormal = e * normalB - e * normalA + finalBNormal;

	// Enqueue the resolved entry.

	ResolvedListEntry resolvedA;
	resolvedA.AngularVelocity = rigidA->angularVelocity;
	resolvedA.Entity          = collision.A;
	resolvedA.Position        = transA->pos + (rigidA->velocity * collision.TimeOfCollision * deltaTime);
	resolvedA.Velocity        = (impactNormal * finalANormal) + (impactTangent * finalATangent);
	resolvedA.Time            = collision.TimeOfCollision;

	ResolvedListEntry resolvedB;
	resolvedB.AngularVelocity = rigidB->angularVelocity;
	resolvedB.Entity          = collision.B;
	resolvedB.Position        = transB->pos + (rigidB->velocity * collision.TimeOfCollision * deltaTime);
	resolvedB.Velocity        = (impactNormal * finalBNormal) + (impactTangent * finalBTangent);
	resolvedB.Time            = collision.TimeOfCollision;

	return { resolvedA, resolvedB };
}


void
Physics::FinalizeMoves(const float& deltaTime)
{
	// Step 9, create a new MoveList set that contains no duplicates.
	std::unordered_set<MoveList::Entry> uniqueMoves;
	for(auto& moveList : _MoveLists)
		for(auto& move : moveList)
			uniqueMoves.insert(move);


	// Step 9.5.. Iterate MoveList and complete every move.
	for(const auto& [rigidbody, position] : uniqueMoves)
	{
		auto optTrans = _TransformManager.GetMutable(rigidbody.entity);
		if(!optTrans.has_value())
		{
			// @TODO: Log Error?
		}

		const auto trans = optTrans.value();

		trans->pos.x =
			Math::Repeat(trans->pos.x + (rigidbody.velocity.x * deltaTime), _ScreenAABB.right);
		trans->pos.y =
			Math::Repeat(trans->pos.y + (rigidbody.velocity.y * deltaTime), _ScreenAABB.bottom);

		trans->rot = Math::Repeat(trans->rot + (rigidbody.angularVelocity * deltaTime), 360.0f);
	}

	// Step 10 Iterate ResolvedList and stomp over with revised moves that are legal.
	for(auto& entry : _ResolvedList)
	{
		auto [entity, position, velocity, angularVelocity, time] = entry;

		auto optTrans = _TransformManager.GetMutable(entity);
		if(!optTrans.has_value())
		{
			// @TODO: Log Error?
		}

		const auto trans = optTrans.value();

		const auto finalPos = position + (velocity * ((1.0f - entry.Time) * deltaTime));

		trans->pos.x = Math::Repeat(finalPos.x, _ScreenAABB.right);
		trans->pos.y = Math::Repeat(finalPos.y, _ScreenAABB.bottom);

		Rigidbody* rigid;
		_RigidbodyManager.GetMutable(entry.Entity, rigid);
		rigid->velocity        = velocity;
		rigid->angularVelocity = angularVelocity;
	}

	// Sort the Report, ready for other game systems to query.
	std::sort(_CollisionReport.begin(), _CollisionReport.end(),
	          [](const CollisionListEntry& A, const CollisionListEntry& B) -> bool
	          {
		          return A.EntityAType < B.EntityAType;
	          });
}

void
Physics::DetectSecondaryCollisions(const std::vector<Physics::ResolvedListEntry> ResolvedThisIteration)
{
	// Step 7. Check the NEW elements in resolved list against the full move list, looking for further collisions.

	// @TODO:
	//	Iterate ResolvedThisIteration
}
