#include "antpch.h"
#include "ContactListener.h"
#include "PhysicsActor.h"
#include "PhysicsJoints.h"

#include "Ant/Scene/Scene.h"
#include "Ant/Scene/Entity.h"
#include "Ant/Scripts/ScriptEngine.h"

namespace Ant{


	void ContactListener::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
	{
		for (uint32_t i = 0; i < count; i++)
		{
			physx::PxJoint* nativeJoint = (physx::PxJoint*)constraints[i].externalReference;
			Ref<JointBase> joint = (JointBase*)nativeJoint->userData;

			Entity entity = joint->GetEntity();
			Entity connectedEntity = joint->GetConnectedEntity();

			if (!entity || !connectedEntity)
				return;

			auto callJointBreakMethod = [](Entity entity, const glm::vec3& linearForce, const glm::vec3& angularForce)
			{
				if (!entity.HasComponent<ScriptComponent>() || !ScriptEngine::IsModuleValid(entity.GetComponent<ScriptComponent>().ScriptClassHandle))
					return;

				ScriptEngine::CallMethod(entity.GetComponent<ScriptComponent>().ManagedInstance, "OnJointBreakInternal", linearForce, angularForce);
			};

			glm::vec3 linearForce = joint->GetLastReportedLinearForce();
			glm::vec3 angularForce = joint->GetLastReportedAngularForce();
			callJointBreakMethod(entity, linearForce, angularForce);
			callJointBreakMethod(connectedEntity, linearForce, angularForce);
		}
	}

	void ContactListener::onWake(physx::PxActor** actors, physx::PxU32 count)
	{
		for (uint32_t i = 0; i < count; i++)
		{
			physx::PxActor& physxActor = *actors[i];
			Ref<PhysicsActorBase> actor = (PhysicsActorBase*)physxActor.userData;
			ANT_CORE_INFO("PhysX Actor waking up: ID: {0}, Name: {1}", actor->GetEntity().GetUUID(), actor->GetEntity().GetComponent<TagComponent>().Tag);
		}
	}

	void ContactListener::onSleep(physx::PxActor** actors, physx::PxU32 count)
	{
		for (uint32_t i = 0; i < count; i++)
		{
			physx::PxActor& physxActor = *actors[i];
			Ref<PhysicsActorBase> actor = (PhysicsActorBase*)physxActor.userData;
			ANT_CORE_INFO("PhysX Actor going to sleep: ID: {0}, Name: {1}", actor->GetEntity().GetUUID(), actor->GetEntity().GetComponent<TagComponent>().Tag);
		}
	}

	void ContactListener::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
	{
		if (!ScriptEngine::GetSceneContext()->IsPlaying())
			return;

		auto removedActorA = pairHeader.flags & physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_0;
		auto removedActorB = pairHeader.flags & physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_1;

		if (removedActorA || removedActorB)
			return;

		Ref<PhysicsActorBase> actorA = (PhysicsActorBase*)pairHeader.actors[0]->userData;
		Ref<PhysicsActorBase> actorB = (PhysicsActorBase*)pairHeader.actors[1]->userData;

		if (!actorA || !actorB)
			return;

		Entity entityA = actorA->GetEntity();
		Entity entityB = actorB->GetEntity();

		if (!entityA || !entityB)
			return;

		auto callCollisionMethod = [](const char* methodName, Entity a, Entity b)
		{
			if (!a.HasComponent<ScriptComponent>())
				return;

			const auto& sc = a.GetComponent<ScriptComponent>();
			if (!ScriptEngine::IsModuleValid(sc.ScriptClassHandle) || !ScriptEngine::IsEntityInstantiated(a))
				return;

			ScriptEngine::CallMethod(sc.ManagedInstance, methodName, b.GetUUID());
		};

		if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH)
		{
			callCollisionMethod("OnCollisionBeginInternal", entityA, entityB);
			callCollisionMethod("OnCollisionBeginInternal", entityB, entityA);
		}
		else if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH)
		{
			callCollisionMethod("OnCollisionEndInternal", entityA, entityB);
			callCollisionMethod("OnCollisionEndInternal", entityB, entityA);
		}
	}

	void ContactListener::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
	{
		if (!ScriptEngine::GetSceneContext()->IsPlaying())
			return;

		for (uint32_t i = 0; i < count; i++)
		{
			if (pairs[i].flags & (physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
				continue;

			Ref<PhysicsActorBase> triggerActor = (PhysicsActorBase*)pairs[i].triggerActor->userData;
			Ref<PhysicsActorBase> otherActor = (PhysicsActorBase*)pairs[i].otherActor->userData;

			if (!triggerActor || !otherActor)
				continue;

			Entity triggerEntity = triggerActor->GetEntity();
			Entity otherEntity = otherActor->GetEntity();

			if (!triggerEntity || !otherEntity)
				continue;

			auto callTriggerMethod = [](const char* methodName, Entity a, Entity b)
			{
				if (!a.HasComponent<ScriptComponent>())
					return;

				const auto& sc = a.GetComponent<ScriptComponent>();

				if (!ScriptEngine::IsModuleValid(sc.ScriptClassHandle) || !ScriptEngine::IsEntityInstantiated(a))
					return;

				ScriptEngine::CallMethod(a.GetComponent<ScriptComponent>().ManagedInstance, methodName, b.GetUUID());
			};

			if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				callTriggerMethod("OnTriggerBeginInternal", triggerEntity, otherEntity);
				callTriggerMethod("OnTriggerBeginInternal", otherEntity, triggerEntity);
			}
			else if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				callTriggerMethod("OnTriggerEndInternal", triggerEntity, otherEntity);
				callTriggerMethod("OnTriggerEndInternal", otherEntity, triggerEntity);
			}
		}
	}

	void ContactListener::onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count)
	{
		PX_UNUSED(bodyBuffer);
		PX_UNUSED(poseBuffer);
		PX_UNUSED(count);
	}

}