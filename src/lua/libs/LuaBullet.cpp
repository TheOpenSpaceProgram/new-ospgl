#include "LuaBullet.h"
#include "../BulletLuaData.h"
#include "physics/RigidBodyUserData.h"
#include "universe/vehicle/part/Piece.h"

#pragma warning(push, 0)
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>
#pragma warning(pop)

#include <glm/glm.hpp>
#include "../../physics/glm/BulletGlmCompat.h"

struct RaycastHit
{
	btRigidBody* rg;
	glm::dvec3 pos;
	glm::dvec3 nrm;
};

void LuaBullet::load_to(sol::table& table)
{

	sol::usertype<BulletTransform> transform_ut = table.new_usertype<BulletTransform>("tranform",
		sol::constructors<BulletTransform(glm::dvec3&), BulletTransform(glm::dvec3&, glm::dquat&)>(),
		"pos", &BulletTransform::pos,
		"rot", &BulletTransform::rot,
		"to_mat4", &BulletTransform::to_dmat4);

	table.new_usertype<RaycastHit>("raycast_hit", sol::no_constructor,
		   "pos", &RaycastHit::pos,
		   "nrm", &RaycastHit::nrm,
		   "rg", &RaycastHit::rg);

	table.new_usertype<btDiscreteDynamicsWorld>("world", sol::no_constructor,
		// TODO: Way more control over the raycast from lua!
		"raycast", [](btDiscreteDynamicsWorld* self, glm::dvec3 start, glm::dvec3 end)
		{
			self->updateAabbs();
			self->computeOverlappingPairs();

			btCollisionWorld::AllHitsRayResultCallback cback(
					to_btVector3(start), to_btVector3(end));
			// This flags supposedly increases performance at the cost of slightly reduced precision
			cback.m_flags |= btTriangleRaycastCallback::kF_UseSubSimplexConvexCastRaytest;
			self->rayTest(to_btVector3(start), to_btVector3(end), cback);
			std::vector<RaycastHit> hits;

			if(cback.hasHit())
			{
				size_t hit_count = cback.m_hitPointWorld.size();
				for(size_t i = 0; i < hit_count; i++)
				{
					RaycastHit hit;
					hit.pos = to_dvec3(cback.m_hitPointWorld[i]);
					hit.nrm = to_dvec3(cback.m_hitNormalWorld[i]);
					// If we use anything other than rigid bodies, this may break!
					hit.rg = (btRigidBody*)cback.m_collisionObjects[i];
					hits.push_back(hit);
				}
			}

			return hits;
		});

	// Rigidbody
	sol::usertype<btRigidBody> rigidbody_ut = table.new_usertype<btRigidBody>("rigidbody",
		"new", [](double mass, bool has_mstate, std::shared_ptr<btCollisionShape> col_shape, glm::dvec3 inertia,
				sol::this_state te)
		{
			btMotionState* mstate = nullptr;
			if(has_mstate)
				mstate = new btDefaultMotionState();
			btRigidBody::btRigidBodyConstructionInfo cinfo(mass, mstate, col_shape.get(),
														   to_btVector3(inertia));
			auto rg = std::make_shared<btRigidBody>(cinfo);
			rg->setActivationState(DISABLE_DEACTIVATION);

			auto rg_data = new BulletLuaData(sol::table(te.lua_state(), sol::create));
			rg_data->shape = std::move(col_shape);
			rg_data->mstate = mstate;

			// Create the shared C++ / Lua userdata
			auto rg_udata = new RigidBodyUserData(rg_data);
			rg_data->udata = rg_udata;

			rg->setUserPointer((void*)rg_udata);

			return rg;
		},
		"get_udata_type", [](btRigidBody& self)
		{
			auto ptr = (RigidBodyUserData*)self.getUserPointer();
			if(ptr)
			{
				if(ptr->type == RigidBodyType::LUA)
					return "lua";
				else if(ptr->type == RigidBodyType::PIECE)
					return "piece";
				else if(ptr->type == RigidBodyType::WELDED_GROUP)
					return "welded_group";
				else
					return "none";
			}
			else
				return "none";
	    },
		"get_udata_lua", [](btRigidBody& self)
		{
			auto ptr = (RigidBodyUserData*)self.getUserPointer();
			if(ptr && ptr->type == RigidBodyType::LUA)
				return sol::optional(ptr->as_lua->assoc_table);
			else
				return sol::optional<sol::table>();
		},
	    "get_udata_piece", [](btRigidBody& self)
	    {
		    auto ptr = (RigidBodyUserData*)self.getUserPointer();
		    if(ptr && ptr->type == RigidBodyType::PIECE)
			    return sol::optional(ptr->as_piece);
		    else
			    return sol::optional<Piece*>();
	    },
		"get_udata_wgroup", [](btRigidBody& self)
		{
		  auto ptr = (RigidBodyUserData*)self.getUserPointer();
		  if(ptr && ptr->type == RigidBodyType::WELDED_GROUP)
			  return sol::optional(ptr->as_wgroup);
		  else
			  return sol::optional<WeldedGroup*>();
		},
		"get_collision_shape", [](btRigidBody& self){return self.getCollisionShape();},
		"set_mass_props", [](btRigidBody& self, double mass, glm::dvec3& inertia)
		{
			self.setMassProps(mass, to_btVector3(inertia));
		},
		"is_in_world", &btRigidBody::isInWorld,
		"set_motion_state", &btRigidBody::setMotionState,
		"get_motion_state", [](btRigidBody& self)
		{
			return self.getMotionState();
		},
		"get_aabb", [](btRigidBody& self)
		{
			btVector3 min, max;
			self.getAabb(min, max);
			return std::make_tuple(to_dvec3(min), to_dvec3(max));
		},
		"translate", [](btRigidBody& self, glm::dvec3& trans)
		{
			self.translate(to_btVector3(trans));
		},
		"get_velocity_in_local_point", [](btRigidBody& self, glm::dvec3& point)
		{
			return to_dvec3(self.getVelocityInLocalPoint(to_btVector3(point)));	
		},
		"set_world_transform", [](btRigidBody& self, glm::dmat4 mat)
		{
			self.setWorldTransform(to_btTransform(mat));
		},
		"get_world_transform", [](btRigidBody& self)
	    {
			return to_dmat4(self.getWorldTransform());
	    },
		"set_linear_velocity", [](btRigidBody& self, glm::dvec3& vel)
		{
			self.setLinearVelocity(to_btVector3(vel));
		},
		"set_angular_velocity", [](btRigidBody& self, glm::dvec3& vel)
		{
			self.setAngularVelocity(to_btVector3(vel));
		},
		"get_linear_velocity", [](btRigidBody& self)
		{
			return to_dvec3(self.getLinearVelocity());
		},
		"get_angular_velocity", [](btRigidBody& self)
		{
			return to_dvec3(self.getAngularVelocity());
		},
		"get_com_transform", [](btRigidBody& self)
		{
			return BulletTransform(self.getCenterOfMassTransform());	
		},
		"get_orientation", [](btRigidBody& self)
		{
			return to_dquat(self.getOrientation());
		},
		"get_com_position", [](btRigidBody& self)
		{
			return to_dvec3(self.getCenterOfMassPosition());
		},
		"update_inertia_tensor", &btRigidBody::updateInertiaTensor,
		"clear_forces", &btRigidBody::clearForces,
		"apply_torque_impulse", [](btRigidBody& self, glm::dvec3& torque)
		{
			self.applyTorqueImpulse(to_btVector3(torque));
		},
		"apply_torque", [](btRigidBody& self, glm::dvec3& torque)
		{
			self.applyTorque(to_btVector3(torque));
		},
		"apply_force", [](btRigidBody& self, glm::dvec3& force, glm::dvec3& rel_pos)
		{
			self.applyForce(to_btVector3(force), to_btVector3(rel_pos));
		},
		"apply_central_impulse", [](btRigidBody& self, glm::dvec3 force)
		{
			self.applyCentralImpulse(to_btVector3(force));
		},
		"apply_central_force", [](btRigidBody& self, glm::dvec3 force)
		{
			self.applyCentralForce(to_btVector3(force));
		},
		"apply_impulse", [](btRigidBody& self, glm::dvec3& imp, glm::dvec3& rel_pos)
		{
			self.applyImpulse(to_btVector3(imp), to_btVector3(rel_pos));
		},
		"set_friction", &btRigidBody::setFriction,
		"get_friction", &btRigidBody::getFriction,
		"set_restitution", &btRigidBody::setRestitution,
		"get_restitution", &btRigidBody::getRestitution,
		"set_kinematic", [](btRigidBody& self)
		{
			self.setCollisionFlags(self.getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		},
		"add_to_world", [](btRigidBody* self, btDiscreteDynamicsWorld* world)
		{
			world->addRigidBody(self);
			if(self->getUserPointer())
			{
				auto d = (RigidBodyUserData*)self->getUserPointer();
				if(d->type == RigidBodyType::LUA)
				{
					auto u = d->as_lua;
					if(u->in_world != nullptr)
					{
						// TODO: This may eventually be allowed
						logger->fatal("Tried to add a rigidbody to many worlds, this is not allowed!");
					}
					u->in_world = world;
				}
			}
		},
		"remove_from_world", [](btRigidBody* self, btDiscreteDynamicsWorld* world)
		{
			world->removeRigidBody(self);
			if(self->getUserPointer())
			{
				auto d = (RigidBodyUserData*)self->getUserPointer();
				if(d->type == RigidBodyType::LUA)
				{
					auto u = d->as_lua;
					u->in_world = nullptr;
				}
			}
		},
		"__gc", [](btRigidBody* self)
		{
			if(self->getUserPointer())
			{
				auto d = (RigidBodyUserData*)self->getUserPointer();
				if(d->type == RigidBodyType::LUA)
				{
					auto u = d->as_lua;
					if (u->in_world)
						u->in_world->removeRigidBody(self);
					// Free the shape and motion state
					u->shape.reset();
					delete u->mstate;
					delete u->udata;
					delete u;
					delete d;
				}
			}
			else
			{
				logger->warn("Rigidbody was gargage collected, and had no lua user data. This should not happen");
			}
		}


	);

	// btTypedConstaint is the base for all constraints
	// You cannot actually instantiate it, but you can use the
	// other subclasses which inherit from it
	table.new_usertype<btTypedConstraint>("typed_constraint",
		"get_breaking_impulse_threshold", &btTypedConstraint::getBreakingImpulseThreshold,
		"set_breaking_impulse_threshold", &btTypedConstraint::setBreakingImpulseThreshold,
		"add_to_world", [](btTypedConstraint* self, btDynamicsWorld& world, bool disable_self_collision = false)
		{
			world.addConstraint(self, disable_self_collision);
		},
		"remove_from_world", [](btTypedConstraint* self, btDynamicsWorld& world)
		{
			world.removeConstraint(self);
		},
		"is_enabled", &btTypedConstraint::isEnabled,
		"set_enabled", &btTypedConstraint::setEnabled,
		"get_rigidbody_a", [](btTypedConstraint& self){return self.getRigidBodyA();},
		"get_rigidbody_b", [](btTypedConstraint& self){return self.getRigidBodyB();},
		"set_debug_draw_size", &btTypedConstraint::setDbgDrawSize,
		"get_debug_draw_size", &btTypedConstraint::getDbgDrawSize
	);		

	table.new_usertype<btGeneric6DofConstraint>("generic_6dof_constraint",
		sol::base_classes, sol::bases<btTypedConstraint>(),
		"new", [](btRigidBody& a, btRigidBody& b, BulletTransform& frame_a, BulletTransform frame_b, bool linear_a)
		{
			return btGeneric6DofConstraint(a, b, frame_a.to_btTransform(), frame_b.to_btTransform(), linear_a); 
		},
		"get_axis", [](btGeneric6DofConstraint& self, int axis)
		{
			return to_dvec3(self.getAxis(axis));
		},
		"get_angle", &btGeneric6DofConstraint::getAngle,
		"get_relative_pivot_position", &btGeneric6DofConstraint::getRelativePivotPosition,
		"set_frames", [](btGeneric6DofConstraint& self, BulletTransform& a, BulletTransform& b)
		{
			self.setFrames(a.to_btTransform(), b.to_btTransform());
		},
		"set_linear_lower_limit", [](btGeneric6DofConstraint& self, glm::dvec3 lim)
		{
			self.setLinearLowerLimit(to_btVector3(lim));
		},
		"set_linear_upper_limit", [](btGeneric6DofConstraint& self, glm::dvec3 lim)
		{
			self.setLinearUpperLimit(to_btVector3(lim));
		},
		"get_linear_lower_limit", [](btGeneric6DofConstraint& self)
		{
			btVector3 v;
			self.getLinearLowerLimit(v);
			return to_dvec3(v);
		},
		"get_linear_upper_limit", [](btGeneric6DofConstraint& self)
		{
			btVector3 v;
			self.getLinearUpperLimit(v);
			return to_dvec3(v);
		},
		"set_angular_lower_limit", [](btGeneric6DofConstraint& self, glm::dvec3 lim)
		{
			self.setAngularLowerLimit(to_btVector3(lim));
		},
		"set_angular_upper_limit", [](btGeneric6DofConstraint& self, glm::dvec3 lim)
		{
			self.setAngularUpperLimit(to_btVector3(lim));
		},
		"get_angular_lower_limit", [](btGeneric6DofConstraint& self)
		{
			btVector3 v;
			self.getAngularLowerLimit(v);
			return to_dvec3(v);
		},
		"get_angular_upper_limit", [](btGeneric6DofConstraint& self)
		{
			btVector3 v;
			self.getAngularUpperLimit(v);
			return to_dvec3(v);
		},
		"get_rotational_limit_motor", &btGeneric6DofConstraint::getRotationalLimitMotor,
		"get_translational_limit_motor", &btGeneric6DofConstraint::getTranslationalLimitMotor,
		"set_limit", &btGeneric6DofConstraint::setLimit,
		"is_limited", &btGeneric6DofConstraint::isLimited,
		"get_use_frame_offset", &btGeneric6DofConstraint::getUseFrameOffset,
		"set_use_frame_offset", &btGeneric6DofConstraint::setUseFrameOffset,
		"get_use_linear_reference_frame_a", &btGeneric6DofConstraint::getUseLinearReferenceFrameA,
		"set_use_linear_reference_frame_a", &btGeneric6DofConstraint::setUseLinearReferenceFrameA,
		"set_axis", [](btGeneric6DofConstraint& self, glm::dvec3 axis1, glm::dvec3 axis2)
		{
			self.setAxis(to_btVector3(axis1), to_btVector3(axis2));
		}	
	);


	table.new_usertype<btGeneric6DofSpringConstraint>("generic_6dof_spring_constraint",
		sol::base_classes, sol::bases<btGeneric6DofConstraint, btTypedConstraint>(),
		"new", [](btRigidBody& a, btRigidBody& b, BulletTransform& frame_a, BulletTransform frame_b, bool linear_a)
		{
			return btGeneric6DofSpringConstraint(a, b, frame_a.to_btTransform(), frame_b.to_btTransform(), linear_a); 
		},
		"enable_spring", &btGeneric6DofSpringConstraint::enableSpring,
		"set_stiffness", &btGeneric6DofSpringConstraint::setStiffness,
		"set_damping", &btGeneric6DofSpringConstraint::setDamping,
		"set_equilibrium_point", sol::overload(
			sol::resolve<void(void)>(&btGeneric6DofSpringConstraint::setEquilibriumPoint),
			sol::resolve<void(int)>(&btGeneric6DofSpringConstraint::setEquilibriumPoint),
			sol::resolve<void(int, btScalar)>(&btGeneric6DofSpringConstraint::setEquilibriumPoint)),
		"is_spring_enabled", &btGeneric6DofSpringConstraint::isSpringEnabled,
		"get_stiffness", &btGeneric6DofSpringConstraint::getStiffness,
		"get_damping", &btGeneric6DofSpringConstraint::getDamping,
		"get_equilibrium_point", &btGeneric6DofSpringConstraint::getEquilibriumPoint,
		"set_axis", [](btGeneric6DofSpringConstraint& self, glm::dvec3 axis1, glm::dvec3 axis2)
		{
			self.setAxis(to_btVector3(axis1), to_btVector3(axis2));
		}	
	);
}
