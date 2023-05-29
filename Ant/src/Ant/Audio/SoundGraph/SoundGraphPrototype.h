﻿#pragma once

#include "Ant/Core/UUID.h"
#include "Ant/Core/Identifier.h"
#include "containers/choc_Value.h"

#include "Ant/Serialization/SerializationImpl.h"

namespace Ant
{
	class StreamWriter;
	class StreamReader;
}

namespace Ant::SoundGraph {

	//==============================================================================
	/// SoundGraph Prototype, used to construct instances of SoundGraph for playback
	struct Prototype : public RefCounted
	{
		std::string DebugName;
		UUID ID;

		struct Endpoint
		{
			Identifier EndpointID;
			choc::value::Value DefaultValue;

			Endpoint(Identifier ID, choc::value::ValueView defaultValue)
				: EndpointID(ID), DefaultValue(defaultValue) {}

			Endpoint()
				: EndpointID(0), DefaultValue() {}

			static void Serialize(Ant::StreamWriter* writer, const Endpoint& endpoint);
			static void Deserialize(Ant::StreamReader* reader, Endpoint& endpoint);
		};

		//==============================================================================
		/// Graph IO
		std::vector<Endpoint> Inputs, Outputs;

		// TODO: this should be removed and output channels should have hardcoded ids
		std::vector<Identifier> OutputChannelIDs;

		std::vector<Endpoint> LocalVariablePlugs;

		//==============================================================================
		/// Nodes
		struct Node
		{
			Identifier NodeTypeID; // Used to call Factory to create the right node type
			UUID ID;
			std::vector<Endpoint> DefaultValuePlugs;

			Node(Identifier typeID, UUID uniqueID) : NodeTypeID(typeID), ID(uniqueID) {}
			Node() : NodeTypeID(0), ID(0) {}

			static void Serialize(Ant::StreamWriter* writer, const Node& endpoint);
			static void Deserialize(Ant::StreamReader* reader, Node& endpoint);
		};

		std::vector<Node> Nodes;

		//==============================================================================
		/// Connections
		struct Connection
		{
			enum EType
			{
				NodeValue_NodeValue = 0,
				NodeEvent_NodeEvent = 1,
				GraphValue_NodeValue = 2,
				GraphEvent_NodeEvent = 3,
				NodeValue_GraphValue = 4,
				NodeEvent_GraphEvent = 5,
				LocalVariable_NodeValue = 6,
			};

			struct Endpoint
			{
				UUID NodeID;
				Identifier EndpointID;
			};

			Endpoint Source, Destination;
			EType Type;

			Connection(Endpoint&& source, Endpoint&& destination, EType connectionType)
				: Source(source), Destination(destination), Type(connectionType)
			{}

			Connection()
				: Source{ 0, 0 }, Destination{ 0, 0 }, Type(NodeValue_NodeValue) {}

			static void Serialize(Ant::StreamWriter* writer, const Connection& endpoint);
			static void Deserialize(Ant::StreamReader* reader, Connection& endpoint);
		};

		// Used to create a copy of the graph
		std::vector<Connection> Connections;

		static void Serialize(Ant::StreamWriter* writer, const Prototype& prototype);
		static void Deserialize(Ant::StreamReader* reader, Prototype& prototype);
	};

} // namspace Ant::SoundGraph

using Prototype = Ant::SoundGraph::Prototype;

SERIALIZABLE(Prototype,
	&Prototype::DebugName,
	&Prototype::ID,
	&Prototype::Inputs,
	&Prototype::Outputs,
	&Prototype::OutputChannelIDs,
	&Prototype::LocalVariablePlugs,
	&Prototype::Nodes,
	&Prototype::Connections);

SERIALIZABLE(Prototype::Node,
	&Prototype::Node::NodeTypeID,
	&Prototype::Node::ID,
	&Prototype::Node::DefaultValuePlugs);

namespace Ant::Serialization
{
	using PEndpoint = Prototype::Endpoint;

	struct ValueSerializer
	{
		std::vector<uint8_t> Data;
		void write(const void* data, size_t size)
		{
			Data.insert(Data.end(), (const uint8_t*)data, (const uint8_t*)data + size);
		}
	};
#if 1
	namespace Impl
	{
		//=============================================================================
		// Serialization
		template<>
		static inline bool SerializeImpl(Ant::StreamWriter* writer, const choc::value::Value& v)
		{
			struct ValueSerializer wrapper;
			v.serialise(wrapper);

			writer->WriteArray(wrapper.Data);
			return true;
		}


		template<>
		static inline bool SerializeImpl(Ant::StreamWriter* writer, const Prototype::Connection& v)
		{
			writer->WriteRaw(v);
			return true;
		}

		//=============================================================================
		// Deserialization

		template<>
		static inline bool DeserializeImpl(Ant::StreamReader* reader, choc::value::Value& v)
		{
			std::vector<uint8_t> data;
			reader->ReadArray(data);
			choc::value::InputData inputData{ data.data(), data.data() + data.size() };
			v = choc::value::Value::deserialise(inputData);
			return true;
		}

		template<>
		static inline bool DeserializeImpl(Ant::StreamReader* reader, Prototype::Connection& v)
		{
			reader->ReadRaw(v);
			return true;
		}
	} // namespace Impl
#endif
}
