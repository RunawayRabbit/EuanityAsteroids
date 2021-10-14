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

Physics::Physics(TransformManager& TransformManager, RigidbodyManager& RigidbodyManager, const AABB& ScreenAabb)
	: _TransformManager(TransformManager),
	  _RigidbodyManager(RigidbodyManager),
	  _ScreenAABB(ScreenAabb),
	  _ChunkSizeX(ScreenAabb.max.x / CHUNKS_X),
	  _ChunkSizeY(ScreenAabb.max.y / CHUNKS_Y),
	  _MoveLists()
{
}

void
Physics::Enqueue(const Rigidbody& Rb, const float& DeltaTime)
{
	// Get an AABB for the rigidbody using it's transform

	auto OptionalRBTrans = _TransformManager.Get(Rb.entity);
	if(!OptionalRBTrans.has_value())
	{
		//@TODO: Error check in case of RB with no Transform?
	}

	Transform RbTrans = OptionalRBTrans.value();
	auto RbAabb       = ColliderRadius::GetAABB(Rb.colliderType, RbTrans.pos);

	// Pad the AABB by the velocity, and a small safety margin.
	const auto DeltaPosition = Rb.velocity * DeltaTime;

	const auto Padding = 15.0f;

	RbAabb.min.x = std::min(RbAabb.min.x, RbAabb.min.x + DeltaPosition.x - Padding);
	RbAabb.min.y = std::min(RbAabb.min.y, RbAabb.min.y + DeltaPosition.y - Padding);
	RbAabb.max.x = std::max(RbAabb.max.x, RbAabb.max.x + DeltaPosition.x + Padding);
	RbAabb.max.y = std::max(RbAabb.max.y, RbAabb.max.y + DeltaPosition.y + Padding);

	auto Accumulator = Vector2::zero();
	auto StartTileX  = -1;
	auto StartTileY  = -1;
	while(Accumulator.x < RbAabb.min.x)
	{
		Accumulator.x += _ChunkSizeX;
		++StartTileX;
	}
	while(Accumulator.y < RbAabb.min.y)
	{
		Accumulator.y += _ChunkSizeY;
		++StartTileY;
	}
	// @NOTE: Logic here is a bit tricky. At this point, startTile indicates the
	//  "lowest" tile that the AABB overlaps, and accumulator is sitting in the
	//  maximum corner of that tile.

	// Using the same method, we advance the max accumulator to find the maximum.
	auto EndTileX = StartTileX;
	auto EndTileY = StartTileY;
	while(Accumulator.x < RbAabb.max.x)
	{
		Accumulator.x += _ChunkSizeX;
		++EndTileX;
	}
	while(Accumulator.y < RbAabb.max.y)
	{
		Accumulator.y += _ChunkSizeY;
		++EndTileY;
	}

	// Enqueue the rigidbody into the appropriate moveLists by the chunk it's in.
	for(auto Y = StartTileY; Y <= EndTileY; ++Y)
	{
		// Wrap our Y coordinate if we have to.
		auto WrappedY = RbTrans.pos.y;
		if(Y < 0)
			WrappedY += _ScreenAABB.max.y;
		else if(Y >= CHUNKS_Y)
			WrappedY -= _ScreenAABB.max.y;

		for(auto X = StartTileX; X <= EndTileX; ++X)
		{
			// Wrap the element in X if we have to.
			auto WrappedX = RbTrans.pos.x;
			if(X < 0)
				WrappedX += _ScreenAABB.max.x;
			else if(X >= CHUNKS_X)
				WrappedX -= _ScreenAABB.max.x;

			// Calculate the chunk index and enqueue
			auto Mod              = [](const int A, const int B) { return (B + (A % B)) % B; };
			const auto ChunkIndex = Mod(Y, CHUNKS_Y) * CHUNKS_X + Mod(X, CHUNKS_X);
			_MoveLists[ChunkIndex].Enqueue({ Rb, Vector2(WrappedX, WrappedY) });
		}
	}
}

void
Physics::Simulate(const float& DeltaTime)
{
	_CollisionReport.clear(); // Clear last frame's report.

	// Start our worker threads churning through the Initial collision tests.
	for(auto i      = 0; i < CHUNK_COUNT; ++i)
		_Workers[i] = std::async([this, i, DeltaTime]() -> std::vector<Physics::CollisionListEntry>
		{
			return DetectInitialCollisions(_MoveLists[i], DeltaTime);
		});
	for(auto& Worker : _Workers)
	{
		auto Result = Worker.get();
		_CollisionList.reserve(_CollisionList.size() + Result.size());
		_CollisionList.insert(_CollisionList.end(), Result.begin(), Result.end());
	}

	if(_CollisionList.size() > 0) // Most frames feature zero collisions.
	{
		auto SolverIterations = 0;
		do
		{
			// From each collision, generate new moves based on physical interactions
			auto ResolvedThisIteration = ResolveUpdatedMovement(DeltaTime);

			// Clear the collision list and the dirty set.
			_CollisionReport.reserve(_CollisionReport.size() + _CollisionList.size());
			_CollisionReport.insert(_CollisionReport.end(), _CollisionList.begin(), _CollisionList.end());
			_CollisionList.clear();
			_DirtyList.clear();

			//  Check the NEW elements in resolved list against the full move list, looking for further collisions.
			DetectSecondaryCollisions(ResolvedThisIteration);

			// Step 7.5. Integrate the new resolved entries into the list.
			_ResolvedList.reserve(_ResolvedList.size() + ResolvedThisIteration.size());
			_ResolvedList.insert(_ResolvedList.end(), ResolvedThisIteration.begin(), ResolvedThisIteration.end());

			// Step 8. If the collision list is empty, continue. Otherwise, loop..
			++SolverIterations;
		} while(_CollisionList.size() > 0 && SolverIterations < MAX_SOLVER_ITERATIONS);

		std::sort(_ResolvedList.begin(), _ResolvedList.end(),
		          [](const ResolvedListEntry& a, ResolvedListEntry& b) -> bool
		          {
			          return a.Time < b.Time;
		          });
	}

	FinalizeMoves(DeltaTime);
	End();
}

void
Physics::End()
{
	for(auto& MoveList : _MoveLists)
		MoveList.Clear();

	_CollisionList.clear();
	_ResolvedList.clear();
	_DirtyList.clear();
}

std::vector<Physics::CollisionListEntry>
Physics::DetectInitialCollisions(MoveList& MoveList, const float& DeltaTime) const
{
	std::vector<CollisionListEntry> Collisions;
	if(MoveList.Size() == 0)
		return Collisions;

	// Sort to get our MoveList in order.
	std::sort(MoveList.begin(), MoveList.end(), [](const MoveList::Entry& a, const MoveList::Entry& b) -> bool
	{
		return a.Rb.colliderType < b.Rb.colliderType;
	});

	// The order of these has to match the order of enum ColliderType defined in ColliderType.h.
	MoveList::ColliderRanges Ranges;
	Ranges.ShipBegin = MoveList.begin();
	Ranges.ShipEnd   = Ranges.BulletBegin = Ranges.ShipBegin + MoveList.GetColliderCount(ColliderType::SHIP);
	Ranges.BulletEnd = Ranges.LargeBegin  = Ranges.BulletBegin + MoveList.GetColliderCount(ColliderType::BULLET);
	Ranges.LargeEnd  = Ranges.MediumBegin = Ranges.LargeBegin + MoveList.GetColliderCount(ColliderType::LARGE_ASTEROID);
	Ranges.MediumEnd = Ranges.SmallBegin  = Ranges.MediumBegin + MoveList.GetColliderCount(ColliderType::MEDIUM_ASTEROID);
	Ranges.SmallEnd  = MoveList.end();

	ShipVsAsteroid(Ranges, Collisions);
	BulletVsAsteroid(Ranges, Collisions, DeltaTime);
	AsteroidVsAsteroid(Ranges, Collisions, DeltaTime);

	return Collisions;
}


void
Physics::ShipVsAsteroid(const MoveList::ColliderRanges& ranges, std::vector<CollisionListEntry>& collisions) const
{
	for(auto Ship = ranges.ShipBegin; Ship != ranges.ShipEnd; ++Ship)
	{
		auto OptionalShipTrans = _TransformManager.Get(Ship->Rb.entity);
		if(!OptionalShipTrans.has_value())
			// @TODO: Are you ever going to write that logging module? Because this should be logged.
			continue;
		Transform ShipTrans = OptionalShipTrans.value();
		OBB PlayerObb(ShipTrans.pos, ColliderRadius::Ship, ShipTrans.rot);

		OBBVsSpecificAsteroid(PlayerObb, Ship->Rb.entity, ranges.LargeBegin, ranges.LargeEnd, ColliderRadius::Large, collisions);
		OBBVsSpecificAsteroid(PlayerObb, Ship->Rb.entity, ranges.MediumBegin, ranges.MediumEnd, ColliderRadius::Medium, collisions);
		OBBVsSpecificAsteroid(PlayerObb, Ship->Rb.entity, ranges.SmallBegin, ranges.SmallEnd, ColliderRadius::Small, collisions);
	}
}

void
Physics::OBBVsSpecificAsteroid(const OBB& Ship,
                               const Entity& ShipEntity,
                               const std::vector<MoveList::Entry>::iterator AsteroidBegin,
                               const std::vector<MoveList::Entry>::iterator AsteroidEnd,
                               const float& AsteroidRadius,
                               std::vector<CollisionListEntry>& Collisions)
{
	for(auto Asteroid = AsteroidBegin; Asteroid != AsteroidEnd; ++Asteroid)
	{
		Circle Collider(Asteroid->Pos, AsteroidRadius);
		if(CollisionTests::OBBToCircle(Ship, Collider))
		{
			CollisionListEntry Entry;
			Entry.A           = ShipEntity;
			Entry.EntityAType = ColliderType::SHIP;
			Entry.MassA       = GetMassFromColliderType(ColliderType::SHIP);
			Entry.B           = Asteroid->Rb.entity;
			Entry.EntityBType = Asteroid->Rb.colliderType;
			Entry.MassB       = GetMassFromColliderType(Asteroid->Rb.colliderType);

			Entry.TimeOfCollision = 0.0f; // Made-up.

			Collisions.push_back(Entry);
		}
	}
}

float
Physics::GetMassFromColliderType(const ColliderType& Type)
{
	switch(Type)
	{
		case ColliderType::LARGE_ASTEROID: return ASTEROID_MASSES[0];
		case ColliderType::MEDIUM_ASTEROID: return ASTEROID_MASSES[1];
		case ColliderType::SMOL_ASTEROID: return ASTEROID_MASSES[2];

		case ColliderType::SHIP: return ASTEROID_MASSES[1] / 2;
		default: return 0.0f;
	}
}

void
Physics::BulletVsAsteroid(const MoveList::ColliderRanges& Ranges,
                          std::vector<CollisionListEntry>& Collisions,
                          const float& DeltaTime)
{
	constexpr float BulletMass = 0; // Fuck it, bullets don't have mass. I have decided this.

	for(auto Bullet = Ranges.BulletBegin; Bullet != Ranges.BulletEnd; ++Bullet)
	{
		const auto BulletVsLargeRadius =
			(ColliderRadius::Bullet + ColliderRadius::Large) *
			(ColliderRadius::Bullet + ColliderRadius::Large);
		CircleVsCircles(*Bullet, ColliderRadius::Bullet, BulletMass,
		                Ranges.LargeBegin, Ranges.LargeEnd,
		                ASTEROID_MASSES[0], BulletVsLargeRadius, DeltaTime,
		                ColliderType::BULLET, ColliderType::LARGE_ASTEROID,
		                Collisions);

		const auto BulletVsMediumRadius =
			(ColliderRadius::Bullet + ColliderRadius::Medium) *
			(ColliderRadius::Bullet + ColliderRadius::Medium);
		CircleVsCircles(*Bullet, ColliderRadius::Bullet, BulletMass,
		                Ranges.MediumBegin, Ranges.MediumEnd,
		                ASTEROID_MASSES[1], BulletVsMediumRadius, DeltaTime,
		                ColliderType::BULLET, ColliderType::MEDIUM_ASTEROID,
		                Collisions);

		const auto BulletVsSmallRadius =
			(ColliderRadius::Bullet + ColliderRadius::Small) *
			(ColliderRadius::Bullet + ColliderRadius::Small);

		CircleVsCircles(*Bullet, ColliderRadius::Bullet, BulletMass,
		                Ranges.SmallBegin, Ranges.SmallEnd,
		                ASTEROID_MASSES[2], BulletVsSmallRadius, DeltaTime,
		                ColliderType::BULLET, ColliderType::SMOL_ASTEROID,
		                Collisions);
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
Physics::ResolveUpdatedMovement(const float& DeltaTime)
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
			auto Moves = ResolveMove(DeltaTime, Collision);
			Resolved.push_back(Moves[0]);
			Resolved.push_back(Moves[1]);

			_DirtyList.insert(Collision.A);
			_DirtyList.insert(Collision.B);
		}

	return Resolved;
}


std::array<Physics::ResolvedListEntry, 2>
Physics::ResolveMove(const float& deltaTime, CollisionListEntry collision) const
{
	// This is probably all kinds of wrong, but I have never studied physics
	// so I'm just sort of making this up as I go along, helped with some
	// frankly crappy youtube videos.

	// https://www.youtube.com/watch?v=Dww4ArU5JF8

	Transform* transA =_TransformManager.GetMutable(collision.A).value();
	Rigidbody* rigidA;
	_RigidbodyManager.GetMutable(collision.A, rigidA);

	Transform* transB = _TransformManager.GetMutable(collision.B).value();
	Rigidbody* rigidB;
	_RigidbodyManager.GetMutable(collision.B, rigidB);

	Vector2 startPosA = transA->pos + rigidA->velocity * (collision.TimeOfCollision * deltaTime);
	Vector2 startPosB = transB->pos + rigidB->velocity * (collision.TimeOfCollision * deltaTime);

	Vector2 relPos = startPosB - startPosA;

	// Split the problem into two parts: the component normal to the collision
	// and the component tangential to the collision.

	Vector2 impactNormal = relPos.normalized();
	Vector2 impactTangent = impactNormal.Rot90CW();

	// Conservation of Momentum (tangential)
	// Dot(impactTangent, A.velocity) = Dot(impactTangent, endVelA);
	// Dot(impactTangent, B.velocity) = Dot(impactTangent, endVelB);
	float finalATangent = Dot(rigidA->velocity, impactTangent); // A.vel * cos(theta)
	float finalBTangent = Dot(rigidB->velocity, impactTangent); // B.vel * cos(theta)

	// @NOTE: I am most likely mis-using this term here.
	const float e = 0.95f; // Coefficient of restitution
	// e = B.Velocity - A.Velocity / endVelB - endVelA

	// Conservation of Momentum (normal)
	float normalA = Dot(rigidA->velocity, impactNormal); // A.vel * sin(theta)
	float normalB = Dot(rigidB->velocity, impactNormal); // B.vel * sin(theta)
	// massA * normalA + massB * normalB = massA * finalANormal + massB * finalBNormal;

	// Did the algebra and this is what fell out.

	float finalBNormal =
		normalA * collision.MassA * e -
		normalB * collision.MassA * e +
		normalA * collision.MassA + normalB * collision.MassB;

	finalBNormal /= collision.MassA + collision.MassB;

	float finalANormal = e * normalB - e * normalA + finalBNormal;

	// Enqueue the resolved entry.

	ResolvedListEntry resolvedA;
	resolvedA.AngularVelocity = rigidA->angularVelocity;
	resolvedA.Entity = collision.A;
	resolvedA.Position = transA->pos + (rigidA->velocity * collision.TimeOfCollision * deltaTime);
	resolvedA.Velocity = (impactNormal * finalANormal) + (impactTangent * finalATangent);
	resolvedA.Time = collision.TimeOfCollision;

	ResolvedListEntry resolvedB;
	resolvedB.AngularVelocity = rigidB->angularVelocity;
	resolvedB.Entity = collision.B;
	resolvedB.Position = transB->pos + (rigidB->velocity * collision.TimeOfCollision * deltaTime);
	resolvedB.Velocity = (impactNormal * finalBNormal) + (impactTangent * finalBTangent);
	resolvedB.Time = collision.TimeOfCollision;

	return { resolvedA, resolvedB };
}


void
Physics::FinalizeMoves(const float& DeltaTime)
{
	// Step 9, create a new MoveList set that contains no duplicates.
	std::unordered_set<MoveList::Entry> UniqueMoves;
	for(auto& MoveList : _MoveLists)
		for(auto& Move : MoveList)
			UniqueMoves.insert(Move);


	// Step 9.5.. Iterate MoveList and complete every move.
	for(auto& entry : UniqueMoves)
	{
		auto OptTrans = _TransformManager.GetMutable(entry.Rb.entity);
		if(!OptTrans.has_value())
		{
			// @TODO: Log Error?
		}

		Transform* Trans = OptTrans.value();

		Trans->pos.x =
			Math::Repeat(Trans->pos.x + (entry.Rb.velocity.x * DeltaTime), _ScreenAABB.right);
		Trans->pos.y =
			Math::Repeat(Trans->pos.y + (entry.Rb.velocity.y * DeltaTime), _ScreenAABB.bottom);

		Trans->rot = Math::Repeat(Trans->rot + (entry.Rb.angularVelocity * DeltaTime), 360.0f);
	}

	// Step 10 Iterate ResolvedList and stomp over with revised moves that are legal.
	for(auto& Entry : _ResolvedList)
	{
		auto [entity, position, velocity, angularVelocity, time] = Entry;

		auto OptTrans = _TransformManager.GetMutable(entity);
		if(!OptTrans.has_value())
		{
			// @TODO: Log Error?
		}

		Transform* Trans = OptTrans.value();

		const auto FinalPos = position + (velocity * ((1.0f - Entry.Time) * DeltaTime));

		Trans->pos.x = Math::Repeat(FinalPos.x, _ScreenAABB.right);
		Trans->pos.y = Math::Repeat(FinalPos.y, _ScreenAABB.bottom);

		Rigidbody* Rigid;
		_RigidbodyManager.GetMutable(Entry.Entity, Rigid);
		Rigid->velocity        = velocity;
		Rigid->angularVelocity = angularVelocity;
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
