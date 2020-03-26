#include "Part.h"



Part::Part(AssetHandle<PartPrototype>& part_proto)
{
	this->part_proto = part_proto.duplicate();
}


Part::~Part()
{
}
