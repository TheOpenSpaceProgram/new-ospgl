#include "BuildingEntity.h"
#include <util/serializers/glm.h>


glm::dmat4 BuildingEntity::get_model_matrix(bool bullet)
{
	glm::dmat4 mat = glm::dmat4();

	WorldState now = traj.get_state(0, bullet);

	glm::dvec3 pos = now.cartesian.pos;
	glm::dquat rot = now.rotation;

	//debug_drawer->add_line(glm::dvec3(0, 0, 0), pos, glm::vec3(1.0, 1.0, 0.5));

	mat = glm::translate(mat, pos) * glm::toMat4(rot);

	return mat;
}

BuildingEntity::BuildingEntity(AssetHandle<BuildingPrototype>&& proto)
{
	this->proto = std::move(proto);
}

BuildingEntity::BuildingEntity(cpptoml::table& toml) 
{
	this->proto = AssetHandle<BuildingPrototype>(*toml.get_as<std::string>("building"));

	std::string body = *toml.get_as<std::string>("in_body");
	glm::dvec3 rel_pos; SerializeUtil::read_to(toml, rel_pos, "rel_pos");
	glm::dquat rel_rot; SerializeUtil::read_to(toml, rel_rot, "rel_rot");
	this->traj.set_parameters(body, rel_pos, rel_rot);
}


BuildingEntity::~BuildingEntity()
{

}

void BuildingEntity::enable_bullet(btDynamicsWorld* world)
{
	rigid = new btRigidBody(10000000.0, nullptr, proto->collider);

	rigid->setCollisionFlags(rigid->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
	rigid->setFriction(1.0);
	rigid->setRestitution(1.0);
	rigid->setActivationState(DISABLE_DEACTIVATION);

	world->addRigidBody(rigid);
}

void BuildingEntity::disable_bullet(btDynamicsWorld* world)
{

}

void BuildingEntity::init()
{
	traj.setup(get_universe());

	// TODO: Let the game call this
	enable_bullet(get_universe()->bt_world);
}

void BuildingEntity::physics_update(double pdt)
{
	WorldState now = traj.get_state(get_universe()->system.bt, true);
	btTransform trans = btTransform::getIdentity();
	
	trans.setOrigin(to_btVector3(now.cartesian.pos));
	trans.setRotation(to_btQuaternion(now.rotation));
	
	rigid->setWorldTransform(trans);
}

void BuildingEntity::deferred_pass(CameraUniforms& cu)
{
	proto->model->node_by_name["building"]->draw(cu, get_model_matrix(false), true);
}

void BuildingEntity::shadow_pass(ShadowCamera& cu)
{
	proto->model->node_by_name["building"]->draw_shadow(cu, get_model_matrix(false), true);
}
