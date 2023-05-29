#include "antpch.h"
#include "SoundGraphPrototype.h"

namespace Ant::SoundGraph
{
	void Prototype::Endpoint::Serialize(StreamWriter* writer, const Endpoint& endpoint)
	{
		writer->WriteRaw((uint32_t)endpoint.EndpointID);

		Serialization::ValueSerializer wrapper;
		endpoint.DefaultValue.serialise(wrapper);
		writer->WriteArray(wrapper.Data);
	}

	void Prototype::Endpoint::Deserialize(StreamReader* reader, Endpoint& endpoint)
	{
		uint32_t id;
		reader->ReadRaw(id);
		endpoint.EndpointID = id;

		std::vector<uint8_t> data;
		reader->ReadArray(data);
		endpoint.DefaultValue = choc::value::Value::deserialise(choc::value::InputData{ data.data(), data.data() + data.size() });
	}

	void Prototype::Serialize(StreamWriter* writer, const Prototype& prototype)
	{
		Serialization::Serialize(writer, prototype);
	}

	void Prototype::Deserialize(StreamReader* reader, Prototype& prototype)
	{
		Serialization::Deserialize(reader, prototype);
	}

	void Prototype::Connection::Serialize(StreamWriter* writer, const Connection& endpoint)
	{
		ANT_CORE_ASSERT(false);
	}

	void Prototype::Connection::Deserialize(StreamReader* reader, Connection& endpoint)
	{
		ANT_CORE_ASSERT(false);

	}

	void Prototype::Node::Serialize(StreamWriter* writer, const Node& endpoint)
	{
		ANT_CORE_ASSERT(false);

	}

	void Prototype::Node::Deserialize(StreamReader* reader, Node& endpoint)
	{
		ANT_CORE_ASSERT(false);

	}

} // namespace Ant::SoundGraph