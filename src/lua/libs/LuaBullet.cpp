#include "LuaBullet.h"

#pragma warning(push, 0)
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#pragma warning(pop)

#include <glm/glm.hpp>
#include "../../physics/glm/BulletGlmCompat.h"

void LuaBullet::load_to(sol::table& table)
{

	sol::usertype<BulletTransform> transform_ut = table.new_usertype<BulletTransform>("tranform",
		sol::constructors<BulletTransform(glm::dvec3&), BulletTransform(glm::dvec3&, glm::dquat&)>(),
		"pos", &BulletTransform::pos,
		"rot", &BulletTransform::rot);

	// Rigidbody
	sol::usertype<btRigidBody> rigidbody_ut = table.new_usertype<btRigidBody>("rigidbody",
		"new", [](double mass, btMotionState* mstate, btCollisionShape* col_shape, glm::dvec3 inertia = glm::dvec3(0, 0, 0))
		{
			btRigidBody rg = btRigidBody(mass, mstate, col_shape, to_btVector3(inertia));	
			rg.setActivationState(DISABLE_DEACTIVATION);			
			return rg;
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
		"get_aabb", [](btRigidBody& self, glm::dvec3& aabb_min, glm::dvec3& aabb_max)
		{
			btVector3 min, max;
			self.getAabb(min, max);
			aabb_min = to_dvec3(min);
			aabb_max = to_dvec3(max);
		},
		"translate", [](btRigidBody& self, glm::dvec3& trans)
		{
			self.translate(to_btVector3(trans));
		},
		"get_velocity_in_local_point", [](btRigidBody& self, glm::dvec3& point)
		{
			return to_dvec3(self.getVelocityInLocalPoint(to_btVector3(point)));	
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
		"apply_impulse", [](btRigidBody& self, glm::dvec3& imp, glm::dvec3& rel_pos)
		{
			self.applyImpulse(to_btVector3(imp), to_btVector3(rel_pos));
		},
		"add_to_world", [](btRigidBody* self, btDynamicsWorld& world)
		{
			world.addRigidBody(self);
		},
		"remove_from_world", [](btRigidBody* self, btDynamicsWorld& world)
		{
			world.removeRigidBody(self);
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
