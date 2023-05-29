#pragma once

#include "Ant/Editor/NodeGraphEditor/Nodes.h"
#include "Ant/Reflection/TypeDescriptor.h"
#include "choc/containers/choc_Value.h"

#include <tuple>

namespace Ant::Nodes {

	class SGTypes final
	{
	public:
		enum ESGPinType : int
		{
			Flow,
			Bool,
			Int,
			Float,
			Audio,
			String,
			Object,
			Enum
		};

		// TODO: consider removing TSG prefix, SGTypes namespace might be safe enough
		struct TSGFlow {};
		struct TSGAudio { const float Underlying = 0.0f; };
		struct TSGEnum { int Underlying = -1; };

		template<int EType, class TValueType, ImU32 Colour = IM_COL32_WHITE>
		using SGPinDescr = PinDescr<EType, TValueType, Colour, ESGPinType>;

		using TSGPinTypes = std::tuple
			<
			SGPinDescr<ESGPinType::Flow, TSGFlow, IM_COL32(200, 200, 200, 255)>,
			SGPinDescr<ESGPinType::Bool, bool, IM_COL32(220, 48, 48, 255)>,
			SGPinDescr<ESGPinType::Int, int, IM_COL32(68, 201, 156, 255)>,
			SGPinDescr<ESGPinType::Float, float, IM_COL32(147, 226, 74, 255)>,
			SGPinDescr<ESGPinType::Audio, TSGAudio, IM_COL32(102, 204, 163, 255)>,
			SGPinDescr<ESGPinType::String, std::string, IM_COL32(194, 75, 227, 255)>,
			SGPinDescr<ESGPinType::Object, UUID, IM_COL32(51, 150, 215, 255)>,
			SGPinDescr<ESGPinType::Enum, TSGEnum, IM_COL32(2, 92, 48, 255)>
			>;

		static inline const TSGPinTypes s_SGPinDefaultValues
		{
			TSGFlow{},
			false,
			0,
			0.0f,
			TSGAudio{},
			"",
			0,
			TSGEnum{}
		};

		// This is how to declare graph specific pin types
		template<int EType>
		using TPin = TPinType<TSGPinTypes, EType>;

		template<typename TValueType>
		static constexpr auto GetPinForValueType()
		{
			return Type::for_each_tuple(s_SGPinDefaultValues, [](auto pinDescr)
				{
					if constexpr (std::is_same_v<decltype(pinDescr)::value_type, TValueType>)
						return pinDescr;
				});
		}

		template<typename T>
		static constexpr auto ConstructPin(T);

		template<typename TMemberPtr>
		static constexpr auto ConstructPin(TMemberPtr member) -> decltype(GetPinForValueType<Type::member_pointer::return_type<TMemberPtr>::type>())
		{
			using TValue = typename Type::member_pointer::return_type<TMemberPtr>::type;
			return GetPinForValueType<TValue>();
		}

		template<int EType, typename TMemberPtr>
		static constexpr auto ConstructPin(TMemberPtr member)
		{
			return TPin<EType>();
		}

		[[nodiscard]] static Pin* CreatePinForType(ESGPinType type, std::string_view pinName = "")
		{
			Pin* newPin = nullptr;

			Type::for_each_tuple(s_SGPinDefaultValues, [&newPin, type](auto pinDescr)
				{
					using TDescr = TPin<decltype(pinDescr)::pin_type>;

					if (TDescr::pin_type == (int)type)
						newPin = new TDescr(pinDescr/*.DefaultValue*/);
				});

			if (newPin)
				newPin->Name = pinName;

			return newPin;
		}

		static constexpr ImU32 GetPinColour(ESGPinType type)
		{
			// Default color
			ImU32 colour = IM_COL32(220, 220, 220, 255);

			Type::for_each_tuple(s_SGPinDefaultValues,
				[&](auto&& pinDescr) mutable
				{
					using TDescr = typename std::remove_cvref_t<decltype(pinDescr)>;

					if (TDescr::pin_type == type)
						colour = TDescr::colour;
				});

			return colour;
		}

		static ESGPinType GetPinTypeForValue(const choc::value::ValueView value)
		{
			ESGPinType pinType = ESGPinType::Flow;

			choc::value::Type type = value.isArray() ? value.getType().getElementType() : value.getType();

			if (type.isObject())
			{
				if (type.isObjectWithClassName(type::type_name<TSGFlow>()))
				{
					pinType = ESGPinType::Flow;
				}
				else if (type.isObjectWithClassName(type::type_name<UUID>())) // AssetHandle
				{
					pinType = ESGPinType::Object;
				}
				else if (value.hasObjectMember("Type"))
				{
					ANT_CORE_WARN("Found pin with custom type {}, need to handle it!", value["Type"].getString());
				}
			}
			else if (type.isFloat())  pinType = ESGPinType::Float;
			else if (type.isInt32())  pinType = ESGPinType::Int;
			else if (type.isBool())   pinType = ESGPinType::Bool;
			else if (type.isString()) pinType = ESGPinType::String;
			else if (type.isVoid())	  pinType = ESGPinType::Flow;	// Trigger
			else if (type.isInt64())
			{
				pinType = ESGPinType::Object;
				ANT_CORE_ASSERT(false, "Shouldn't be using Int64 Value for Object type in SoundGraph.");
			}
			else ANT_CORE_ASSERT(false);

			return pinType;
		}

		// TODO: extend with Implementation interface
		//=================================================================
		//template</*typename TGraph, */typename TInputsList, typename TOutputsList>
		struct SGNode : Node
		{
			using Node::Node;
			//! if constexpr
#if 0
			using TInputs = TInputsList;
			using TOutputs = TOutputsList;
			using TTopology = MyNode<TInputs, TOutputs>;

			constexpr SGNode(TInputs&& inputs, TOutputs&& outputs)
				: Ins(std::forward<TInputs>(inputs))
				, Outs(std::forward<TOutputs>(outputs))
			{
			}

			TInputs Ins;
			TOutputs Outs;

			constexpr auto GetIn(size_t index) const { return std::get<index>(Ins); }
			constexpr auto GetOut(size_t index) const { return std::get<index>(Outs); }

			constexpr size_t GetInputCount() const { return std::tuple_size_v<TInputs>; }
			constexpr size_t GetOutputCount() const { return std::tuple_size_v<TOutputs>; }
#endif
			virtual int GetTypeID() const override { return 0; }

		};
	};


} // namespace Ant::Nodes

namespace Ant {

	template<>
	static choc::value::Value ValueFrom(const Nodes::SGTypes::TSGFlow& obj)
	{
		return choc::value::createObject(type::type_name< Nodes::SGTypes::TSGFlow>());
	}

	template<>
	static std::optional<Nodes::SGTypes::TSGFlow> CustomValueTo<Nodes::SGTypes::TSGFlow>(choc::value::ValueView customValueObject)
	{
		if (customValueObject.isObjectWithClassName(type::type_name<Nodes::SGTypes::TSGFlow>()))
			return Nodes::SGTypes::TSGFlow();
		else
			return {};
	}

	template<>
	static choc::value::Value ValueFrom(const Nodes::SGTypes::TSGEnum& obj)
	{
		return choc::value::createObject(type::type_name<Nodes::SGTypes::TSGEnum>(),
			"Value", obj.Underlying);
	}

	template<>
	static std::optional<Nodes::SGTypes::TSGEnum> CustomValueTo<Nodes::SGTypes::TSGEnum>(choc::value::ValueView customValueObject)
	{
		if (customValueObject.isObjectWithClassName(type::type_name<Nodes::SGTypes::TSGEnum>()))
			return Nodes::SGTypes::TSGEnum{ customValueObject["Value"].get<int>() };
		else
			return {};
	}


} // namespace Ant