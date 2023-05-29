#include "antpch.h"
#include "PropertySet.h"

#include "yaml-cpp/yaml.h"
#include "Ant/Utilities/YAMLSerializationHelpers.h"
#include "Ant/Utilities/SerializationMacros.h"

#include "text/choc_StringUtilities.h"
#include "text/choc_JSON.h"

namespace Ant::Utils
{

    PropertySet::PropertySet() = default;
    PropertySet::~PropertySet() = default;
    PropertySet::PropertySet(PropertySet&&) = default;
    PropertySet& PropertySet::operator= (PropertySet&&) = default;

    PropertySet::PropertySet(const PropertySet& other)
    {
        if (other.properties != nullptr)
            properties = std::make_unique<std::vector<Property>>(*other.properties);
    }

    PropertySet& PropertySet::operator= (const PropertySet& other)
    {
        if (other.properties != nullptr)
            properties = std::make_unique<std::vector<Property>>(*other.properties);

        return *this;
    }

    static std::string GetTypeName(const choc::value::Value& v)
    {
        bool isArray = v.isArray();
        choc::value::Type type = isArray ? v.getType().getElementType() : v.getType();

        if (type.isFloat())  return "Float";
        else if (type.isInt32())  return "Int";
        else if (type.isBool())   return "Bool";
        else if (type.isString()) return "String";
        else if (type.isInt64())  return "AssetHandle"; //? might not be desireable in all cases to reserve int64 to AssetHandles
        else if (type.isObject())
        {
            if (isArray && v[0].hasObjectMember("TypeName"))
                return v[0]["TypeName"].get<std::string>();
            else if (v.hasObjectMember("TypeName"))
                return v["TypeName"].get<std::string>();
        }

        return "invalid";
    }

    static std::string PropertyToString(const PropertySet::Property& prop)
    {
        auto desc = choc::text::addDoubleQuotes(prop.name) + ": "
            + "{"
            + choc::text::addDoubleQuotes("Type") + ": " + choc::text::addDoubleQuotes(std::string(GetTypeName(prop.value))) + ", "
            + choc::text::addDoubleQuotes("Value") + ": ";

        return desc + choc::json::toString(prop.value) + "}";

        //const auto& type = prop.value.getType();

        ////assert(type.isPrimitive() || type.isString());

        //if (type.isString())
        //    return desc + choc::json::getEscapedQuotedString(prop.value.getString());

        //if (type.isFloat())     return desc + choc::json::doubleToString(prop.value.getFloat64());
        //if (type.isInt())   return desc + std::to_string(prop.value.getInt64());

        //return desc + prop.value.getDescription();
    }

    // TODO: this is taken from soul (ISC), move to some sort of utility header
    // Join array of object with a function to stringify each element
    template <typename Array, typename StringifyFn>
    static std::string JoinStrings(const Array& strings, std::string_view separator, StringifyFn&& stringify)
    {
        if (strings.empty())
            return {};

        std::string s(stringify(strings.front()));

        for (size_t i = 1; i < strings.size(); ++i)
        {
            s += separator;
            s += stringify(strings[i]);
        }

        return s;
    }

    static std::string PropertySetToString(const std::vector<PropertySet::Property>* properties)
    {
        if (properties == nullptr || properties->empty())
            return {};

        auto content = JoinStrings(*properties, ", ", [&](auto& p) { return PropertyToString(p); });

        return "{ " + content + " }";
    }

    bool PropertySet::IsEmpty() const { return Size() == 0; }
    size_t PropertySet::Size() const { return properties == nullptr ? 0 : properties->size(); }

    choc::value::Value PropertySet::GetValue(std::string_view name) const
    {
        ANT_CORE_ASSERT(!name.empty());
        return GetValue(name, {});
    }

    choc::value::Value PropertySet::GetValue(std::string_view name, const choc::value::Value& defaultReturnValue) const
    {
        ANT_CORE_ASSERT(!name.empty());

        if (properties != nullptr)
            for (auto& p : *properties)
                if (p.name == name)
                    return p.value;

        return defaultReturnValue;
    }

    bool PropertySet::HasValue(std::string_view name) const
    {
        ANT_CORE_ASSERT(!name.empty());

        if (properties != nullptr)
            for (auto& p : *properties)
                if (p.name == name) // TODO: this should be replaced with faster option, like hash, or string ptr
                    return true;

        return false;
    }

    bool PropertySet::GetBool(std::string_view name, bool defaultValue) const
    {
        auto v = GetValue(name);
        return !v.isVoid() ? v.getView().get<bool>() : defaultValue;
    }

    float PropertySet::GetFloat(std::string_view name, float defaultValue) const
    {
        auto v = GetValue(name);
        return v.getType().isFloat() || v.getType().isInt() ? v.getView().get<float>() : defaultValue;
    }

    double PropertySet::GetDouble(std::string_view name, double defaultValue) const
    {
        auto v = GetValue(name);
        return v.getType().isFloat() || v.getType().isInt() ? v.getView().get<double>() : defaultValue;
    }

    int32_t PropertySet::GetInt(std::string_view name, int32_t defaultValue) const
    {
        auto v = GetValue(name);
        return v.getType().isFloat() || v.getType().isInt() ? v.getView().get<int32_t>() : defaultValue;
    }

    int64_t PropertySet::GetInt64(std::string_view name, int64_t defaultValue) const
    {
        auto v = GetValue(name);
        return v.getType().isFloat() || v.getType().isInt() ? v.getView().get<int64_t>() : defaultValue;
    }

    std::string PropertySet::GetString(std::string_view name, std::string_view defaultValue) const
    {
        auto v = GetValue(name);

        if (!v.isVoid())
        {
            return std::string(v.getView().get<std::string>());

            /*struct UnquotedPrinter : public soul::ValuePrinter
            {
                std::ostringstream out;

                void print(std::string_view s) override { out << s; }
                void printStringLiteral(soul::StringDictionary::Handle h) override { print(dictionary->getStringForHandle(h)); }
            };

            UnquotedPrinter p;
            p.dictionary = std::addressof(dictionary);
            v.print(p);
            return p.out.str();*/
        }

        return std::string(defaultValue);
    }

    //static void replaceStringLiterals(choc::value::Value & v, soul::SubElementPath path, const soul::StringDictionary & sourceDictionary, soul::StringDictionary & destDictionary)
    //{
    //    auto value = v.getSubElement(path);
    //
    //    if (value.getType().isStringLiteral())
    //    {
    //        auto s = sourceDictionary.getStringForHandle(value.getStringLiteral());
    //        v.modifySubElementInPlace(path, choc::value::Value::createStringLiteral(destDictionary.getHandleForString(s)));
    //    }
    //    else if (value.getType().isFixedSizeArray())
    //    {
    //        for (size_t i = 0; i < value.getType().getArraySize(); ++i)
    //            replaceStringLiterals(v, path + i, sourceDictionary, destDictionary);
    //    }
    //    else if (value.getType().isStruct())
    //    {
    //        for (size_t i = 0; i < value.getType().getStructRef().getNumMembers(); ++i)
    //            replaceStringLiterals(v, path + i, sourceDictionary, destDictionary);
    //    }
    //}

    void PropertySet::SetInternal(std::string_view name, choc::value::Value newValue)
    {
        ANT_CORE_ASSERT(!name.empty());

        if (properties == nullptr)
        {
            properties = std::make_unique<std::vector<Property>>();
        }
        else
        {
            for (auto& p : *properties)
            {
                if (p.name == name)
                {
                    p.value = std::move(newValue);
                    return;
                }
            }
        }

        properties->push_back({ std::string(name), std::move(newValue) });
    }

    void PropertySet::Set(std::string_view name, choc::value::Value newValue)
    {
        //replaceStringLiterals(newValue, {}, sourceDictionary, dictionary);
        SetInternal(name, std::move(newValue));
    }

    void PropertySet::Set(std::string_view name, int32_t value) { SetInternal(name, choc::value::createInt32(value)); }
    void PropertySet::Set(std::string_view name, int64_t value) { SetInternal(name, choc::value::createInt64(value)); }
    void PropertySet::Set(std::string_view name, float value) { SetInternal(name, choc::value::Value(value)); }
    void PropertySet::Set(std::string_view name, double value) { SetInternal(name, choc::value::Value(value)); }
    void PropertySet::Set(std::string_view name, bool value) { SetInternal(name, choc::value::Value(value)); }
    void PropertySet::Set(std::string_view name, const char* value) { Set(name, std::string(value)); }
    void PropertySet::Set(std::string_view name, std::string_view value) { SetInternal(name, choc::value::createString(value)); }

    void PropertySet::Set(std::string_view name, const choc::value::ValueView& value)
    {
        if (value.isInt32())    return Set(std::string(name), value.getInt32());
        if (value.isInt64())    return Set(std::string(name), value.getInt64());
        if (value.isFloat32())  return Set(std::string(name), value.getFloat32());
        if (value.isFloat64())  return Set(std::string(name), value.getFloat64());
        if (value.isBool())     return Set(std::string(name), value.getBool());
        if (value.isString())   return Set(std::string(name), std::string(value.getString()));

        return Set(name, choc::value::Value(value));

        ANT_CORE_ASSERT(false); // other types not currently handled..
    }

    // TODO: this is taken from soul (ISC), move to some sort of utility header
    template <typename Vector, typename Predicate>
    inline bool RemoveIf(Vector& v, Predicate&& pred)
    {
        auto oldEnd = std::end(v);
        auto newEnd = std::remove_if(std::begin(v), oldEnd, pred);

        if (newEnd == oldEnd)
            return false;

        v.erase(newEnd, oldEnd);
        return true;
    }

    void PropertySet::Remove(std::string_view name)
    {
        ANT_CORE_ASSERT(!name.empty());

        if (properties != nullptr)
            RemoveIf(*properties, [&](const Property& p) { return p.name == name; });
    }

    std::vector<std::string> PropertySet::GetNames() const
    {
        std::vector<std::string> result;

        if (properties != nullptr)
        {
            result.reserve(properties->size());

            for (auto& p : *properties)
                result.push_back(p.name);
        }

        return result;
    }

    //choc::value::Value PropertySet::toExternalValue() const
    //{
    //    auto o = choc::value::createObject("PropertySet");
    //
    //    if (properties != nullptr)
    //        for (auto& p : *properties)
    //            o.addMember(p.name, p.value.toExternalValue(ConstantTable(), dictionary));
    //
    //    return o;
    //}


    template<typename T>
    void ParseValueOrArray(PropertySet& ps, std::string_view name, const choc::value::ValueView& value)
    {
        if (value["Value"].isArray())
        {
            if constexpr (std::is_same_v<T, std::string>)
            {
                //? array of strings might cause issues at some point
                choc::value::Value valueArray = choc::value::createArray(value["Value"].size(), [&value](uint32_t i)
                    { return value["Value"][i]; });
                ps.Set(std::string(name), std::move(valueArray));
            }
            else
            {
                choc::value::Value valueArray = choc::value::createArray(value["Value"].size(), [&value](uint32_t i)
                    { return value["Value"][i].get<T>(); });
                ps.Set(std::string(name), std::move(valueArray));
            }
        }
        else
        {
            if constexpr (std::is_same_v<T, std::string>)
                ps.Set(std::string(name), choc::value::Value(value["Value"].getString()));
            else
                ps.Set(std::string(name), choc::value::Value(value["Value"].get<T>()));
        }
    }

#if 0 //? old stuff, AssetHandles are now stored ast int64_t
    static choc::value::Value CreateAssetRefObject(uint64_t handle = 0)
    {
        return choc::value::createObject("AssetHandle",
            "TypeName", "AssetHandle",
            "Value", std::to_string(handle));
    }
#endif

    choc::value::Value ParseCustomValueType(const choc::value::ValueView& value)
    {
        if (value.isVoid())
        {
            ANT_CORE_WARN("Failed to deserialize custom value type, value is not an object.");
            return choc::value::Value(value);
        }
        if (value["TypeName"].isVoid())
        {
            ANT_CORE_ASSERT(false, "Failed to deserialize custom value type, missing \"TypeName\" property.");
            return {};
        }

        choc::value::Value customObject = choc::value::createObject(value["TypeName"].get<std::string>());
        if (value.isObject())
        {
            for (uint32_t i = 0; i < value.size(); i++)
            {
                choc::value::MemberNameAndValue nameValue = value.getObjectMemberAt(i);
                customObject.addMember(nameValue.name, nameValue.value);
            }
        }
        else
        {
            ANT_CORE_ASSERT(false, "Failed to load custom value type. It must be serialized as object.")
        }

        return customObject;
    };

    void ParseCustomObject(PropertySet& ps, std::string_view name, const choc::value::ValueView& value)
    {
        if (value["Value"].isArray())
        {
            // TODO: implement arrays of custom value types
            ANT_CORE_ASSERT(false);
#if 0 
            choc::value::ValueView inArray = value["Value"];
            std::string type = value["Type"].get<std::string>();
            const uint32_t inArraySize = inArray.size();

            ANT_CORE_ASSERT(type == "AssetHandle", "The only custom type is supported for now is \"AssetHandle\".");
            ANT_CORE_ASSERT(inArraySize > 0);

            choc::value::Value valueArray = choc::value::createEmptyArray();

            for (int i = 0; i < inArraySize; ++i)
            {
                choc::value::Value vobj = CreateAssetRefObject();
                vobj["Value"].set(inArray[i]["Value"].get<std::string>());
                valueArray.addArrayElement(vobj);
            }
            ANT_CORE_ASSERT(valueArray.size() == inArraySize)

                ps.Set(std::string(name), valueArray);

            auto et = ps.GetValue(std::string(name)).getType().getElementType();
#endif
        }
        else
            ps.Set(std::string(name), ParseCustomValueType(value["Value"]));
    }

    PropertySet PropertySet::FromJSON(const choc::value::ValueView& v)
    {
        ANT_CORE_ASSERT(false, "Should use ..ExternalValue() serialization instead.");

        //? old way of restoring from JSON string

        PropertySet a;

        //if (v.isObjectWithClassName(objectClassName /*"PropertySet"*/))
        {
            v.visitObjectMembers([&](std::string_view name, const choc::value::ValueView& value)
                {
                    // Need to store type as a property of each value object
                    // because .json can't distinguish between Int and Float if the value is 0.0,
                    // it just knows type "number"
                    if (value.isObject() && value.hasObjectMember("Type"))
                    {
                        std::string type(value["Type"].getString());

                        if (type == "Float")
                            ParseValueOrArray<float>(a, name, value);
                        else if (type == "Int")
                            ParseValueOrArray<int32_t>(a, name, value);
                        else if (type == "Bool")
                            ParseValueOrArray<bool>(a, name, value);
                        else if (type == "AssetHandle")                 //? might not be desireable in all cases to reserve int64 to AssetHandles
                            ParseValueOrArray<int64_t>(a, name, value);
                        else if (type == "String")
                            ParseValueOrArray<std::string>(a, name, value);
                        else
                            ParseCustomObject(a, name, value);
                    }
                    else
                        a.Set(std::string(name), value);
                });
        }

        return a;
    }

    std::string PropertySet::ToJSON() const { ANT_CORE_ASSERT(false, "Should use ..ExternalValue() serialization instead."); return PropertySetToString(properties.get()); }

    choc::value::Value PropertySet::ToExternalValue() const
    {
        choc::value::Value out = choc::value::createObject("PropertySet");

        if (properties)
        {
            for (const auto& prop : *properties.get())
            {
                out.addMember(prop.name, prop.value);
            }
        }

        return out;
    }

    PropertySet PropertySet::FromExternalValue(const choc::value::ValueView& deserialized)
    {
        PropertySet properties;

        if (!deserialized.isObjectWithClassName("PropertySet"))
            return properties;

        for (decltype(deserialized.size()) i = 0; i < deserialized.size(); ++i)
        {
            const auto member = deserialized.getObjectMemberAt(i);
            properties.Set(member.name, member.value);
        }

        return properties;
    }

} // namespace Ant::Utils