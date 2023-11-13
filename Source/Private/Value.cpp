#include <functional>

#include "../Public/Value.h"

using namespace Values;

TBoolValue Values::TBoolValue::operator==(const TBoolValue& Other) const
{
    return TBoolValue(Value == Other.GetValue());
}

TBoolValue Values::TBoolValue::operator!=(const TBoolValue& Other) const
{
    return TBoolValue(Value != Other.GetValue());
}

TIntValue Values::TIntValue::operator+(const TIntValue& Other) const
{
    return Value + Other.GetValue();
}

TIntValue Values::TIntValue::operator-(const TIntValue& Other) const
{
    return Value - Other.GetValue();
}

TIntValue Values::TIntValue::operator*(const TIntValue& Other) const
{
    return Value * Other.GetValue();
}

TIntValue Values::TIntValue::operator/(const TIntValue& Other) const
{
    return Value / Other.GetValue();
}

TBoolValue Values::TIntValue::operator==(const TIntValue& Other) const
{
    return Value == Other.GetValue();
}

TBoolValue Values::TIntValue::operator!=(const TIntValue& Other) const
{
    return Value != Other.GetValue();
}

TBoolValue Values::TIntValue::operator<(const TIntValue& Other) const
{
    return Value < Other.GetValue();
}

TBoolValue Values::TIntValue::operator>(const TIntValue& Other) const
{
    return Value > Other.GetValue();
}

TFloatValue Values::TIntValue::operator+(const TFloatValue& Other) const
{
    return Value + Other.GetValue();
}

TFloatValue Values::TIntValue::operator-(const TFloatValue& Other) const
{
    return Value - Other.GetValue();
}

TFloatValue Values::TIntValue::operator*(const TFloatValue& Other) const
{
    return Value * Other.GetValue();
}

TFloatValue Values::TIntValue::operator/(const TFloatValue& Other) const
{
    return Value / Other.GetValue();
}

TBoolValue Values::TIntValue::operator==(const TFloatValue& Other) const
{
    return Value == Other.GetValue();
}

TBoolValue Values::TIntValue::operator!=(const TFloatValue& Other) const
{
    return Value != Other.GetValue();
}

TBoolValue Values::TIntValue::operator<(const TFloatValue& Other) const
{
    return Value < Other.GetValue();
}

TBoolValue Values::TIntValue::operator>(const TFloatValue& Other) const
{
    return Value > Other.GetValue();
}

TFloatValue Values::TFloatValue::operator+(const TFloatValue& Other) const
{
    return Value + Other.GetValue();
}

TFloatValue Values::TFloatValue::operator-(const TFloatValue& Other) const
{
    return Value - Other.GetValue();
}

TFloatValue Values::TFloatValue::operator*(const TFloatValue& Other) const
{
    return Value * Other.GetValue();
}

TFloatValue Values::TFloatValue::operator/(const TFloatValue& Other) const
{
    return Value / Other.GetValue();
}

TBoolValue Values::TFloatValue::operator==(const TFloatValue& Other) const
{
    return Value == Other.GetValue();
}

TBoolValue Values::TFloatValue::operator!=(const TFloatValue& Other) const
{
    return Value != Other.GetValue();
}

TBoolValue Values::TFloatValue::operator<(const TFloatValue& Other) const
{
    return Value < Other.GetValue();
}

TBoolValue Values::TFloatValue::operator>(const TFloatValue& Other) const
{
    return Value > Other.GetValue();
}

TFloatValue Values::TFloatValue::operator+(const TIntValue& Other) const
{
    return Value + Other.GetValue();
}

TFloatValue Values::TFloatValue::operator-(const TIntValue& Other) const
{
    return Value - Other.GetValue();
}

TFloatValue Values::TFloatValue::operator*(const TIntValue& Other) const
{
    return Value * Other.GetValue();
}

TFloatValue Values::TFloatValue::operator/(const TIntValue& Other) const
{
    return Value / Other.GetValue();
}

TBoolValue Values::TFloatValue::operator==(const TIntValue& Other) const
{
    return Value == Other.GetValue();
}

TBoolValue Values::TFloatValue::operator!=(const TIntValue& Other) const
{
    return Value != Other.GetValue();
}

TBoolValue Values::TFloatValue::operator<(const TIntValue& Other) const
{
    return Value < Other.GetValue();
}

TBoolValue Values::TFloatValue::operator>(const TIntValue& Other) const
{
    return Value > Other.GetValue();
}

TBoolValue::operator int() const
{
    return (int)Value;
}

TBoolValue::operator TIntValue() const
{
    return TIntValue(Value);
}

TBoolValue::operator std::string() const
{
    return Value ? "true" : "false";
}

TBoolValue::operator TStringValue() const
{
    return Value ? std::string("true") : std::string("false");
}

TIntValue::operator TFloatValue() const
{
    return (float)*this;
}

TIntValue::operator std::string() const
{
    return std::to_string(Value);
}

TIntValue::operator TStringValue() const
{
    return (std::string) * this;
}

TFloatValue::operator TIntValue() const
{
    return (int)*this;
}

TFloatValue::operator TStringValue() const
{
    return std::to_string(Value);
}

std::string Values::TStringValue::At(int Index) const
{
    auto ThisSize = (int)Value.size();
    if (Index < -ThisSize || Index >= ThisSize)
    {
        auto R = TObject();
        return R;
    }
    if (Index < 0)
    {
        Index = ThisSize - abs(Index);
    }
    return std::string(1, Value.at(Index));
}

std::string Values::TStringValue::Join(const TArray& Iterator, const std::string& Separator)
{
    std::string Result;

    for (size_t I = 0; I < Iterator.size(); ++I)
    {
        Result += Iterator[I].ToString();
        if (I != Iterator.size() - 1)
        {
            Result += Separator + " ";
        }
    }

    return Result;
}

std::string Values::TStringValue::Join(const TArrayValue& Array, const std::string& Separator)
{
    return Join(Array.GetValue(), Separator);
}

TStringValue Values::TStringValue::operator+(const TStringValue& Other) const
{
    return TStringValue(Value + Other.GetValue());
}

TObject* Values::TArrayValue::At(int Index)
{
    auto ThisSize = Size().GetValue();
    if (Index < -ThisSize || Index >= ThisSize)
    {
        return nullptr;
    }
    if (Index < 0)
    {
        Index = ThisSize - abs(Index);
    }
    return &Value[Index];
}

bool Values::TArrayValue::Contains(const TObject& InValue)
{
    for (int Index = 0; Index < Value.size(); Index++)
    {
        if (Value[Index] == InValue)
        {
            return true;
        }
    }

    return false;
}

TArrayValue Values::TMapValue::GetKeys() const
{
    TArray Keys;
    for (const auto& [K, V] : Value)
    {
        Keys.push_back(K);
    }
    return TArrayValue(Keys);
}

TArrayValue Values::TMapValue::GetValues() const
{
    TArray Values;
    for (const auto& [K, V] : Value)
    {
        Values.push_back(V);
    }
    return TArrayValue(Values);
}

bool Values::TMapValue::HasKey(const std::string& Key) const
{
    return Value.find(Key) != Value.end();
}

TObject Values::TObject::operator+(const TObject& Other) const
{
    switch (Type)
    {
        case IntType :
        {
            switch (Other.Type)
            {
                case IntType :
                    return AsInt()->GetValue() + Other.AsInt()->GetValue();
                case FloatType :
                    return AsInt()->GetValue() + Other.AsFloat()->GetValue();
                default :
                    break;
            }
            break;
        }
        case FloatType :
        {
            switch (Other.Type)
            {
                case IntType :
                    return AsFloat()->GetValue() + Other.AsInt()->GetValue();
                case FloatType :
                    return AsFloat()->GetValue() + Other.AsFloat()->GetValue();
                default :
                    break;
            }
            break;
        }
        case StringType :
        {
            return AsString()->GetValue() + Other.AsString()->GetValue();
        }
        default :
            break;
    }
    return TObject();
}

TObject Values::TObject::operator-(const TObject& Other) const
{
    switch (Type)
    {
        case IntType :
        {
            switch (Other.Type)
            {
                case IntType :
                    return AsInt()->GetValue() - Other.AsInt()->GetValue();
                case FloatType :
                    return AsInt()->GetValue() - Other.AsFloat()->GetValue();
                default :
                    break;
            }
            break;
        }
        case FloatType :
        {
            switch (Other.Type)
            {
                case IntType :
                    return AsFloat()->GetValue() - Other.AsInt()->GetValue();
                case FloatType :
                    return AsFloat()->GetValue() - Other.AsFloat()->GetValue();
                default :
                    break;
            }
            break;
        }
        default :
            break;
    }
    return TObject();
}

TObject Values::TObject::operator*(const TObject& Other) const
{
    switch (Type)
    {
        case IntType :
        {
            switch (Other.Type)
            {
                case IntType :
                    return AsInt()->GetValue() * Other.AsInt()->GetValue();
                case FloatType :
                    return AsInt()->GetValue() * Other.AsFloat()->GetValue();
                default :
                    break;
            }
            break;
        }
        case FloatType :
        {
            switch (Other.Type)
            {
                case IntType :
                    return AsFloat()->GetValue() * Other.AsInt()->GetValue();
                case FloatType :
                    return AsFloat()->GetValue() * Other.AsFloat()->GetValue();
                default :
                    break;
            }
            break;
        }
        default :
            break;
    }
    return TObject();
}

TObject Values::TObject::operator/(const TObject& Other) const
{
    switch (Type)
    {
        case IntType :
        {
            switch (Other.Type)
            {
                case IntType :
                    return AsInt()->GetValue() / Other.AsInt()->GetValue();
                case FloatType :
                    return AsInt()->GetValue() / Other.AsFloat()->GetValue();
                default :
                    break;
            }
            break;
        }
        case FloatType :
        {
            switch (Other.Type)
            {
                case IntType :
                    return AsFloat()->GetValue() / Other.AsInt()->GetValue();
                case FloatType :
                    return AsFloat()->GetValue() / Other.AsFloat()->GetValue();
                default :
                    break;
            }
            break;
        }
        default :
            break;
    }
    return TObject();
}

#define TOBJECT_COMPARE_OP_BODY(X)                                            \
    if (Type == Other.Type)                                                   \
    {                                                                         \
        switch (Type)                                                         \
        {                                                                     \
            case BoolType :                                                   \
                return GetBool().GetValue() X Other.GetBool().GetValue();     \
            case IntType :                                                    \
                return GetInt().GetValue() X Other.GetInt().GetValue();       \
            case FloatType :                                                  \
                return GetFloat().GetValue() X Other.GetFloat().GetValue();   \
            case StringType :                                                 \
                return GetString().GetValue() X Other.GetString().GetValue(); \
            default :                                                         \
                return TObject();                                             \
        }                                                                     \
    }

TObject Values::TObject::operator<(const TObject& Other) const { TOBJECT_COMPARE_OP_BODY(<) }

TObject Values::TObject::operator>(const TObject& Other) const
{
    TOBJECT_COMPARE_OP_BODY(>)
}

// bool Values::TObject::operator==(TObject& Other)
//{
//	if (GetType() != Other.GetType())
//	{
//		return false;
//	}
//
//	switch (GetType())
//	{
//		case BoolType :
//			return AsBool()->GetValue() == Other.AsBool()->GetValue();
//		case IntType :
//			return AsInt()->GetValue() == Other.AsInt()->GetValue();
//		case FloatType :
//			return AsFloat()->GetValue() == Other.AsFloat()->GetValue();
//		case StringType :
//			return AsString()->GetValue() == Other.AsString()->GetValue();
//	}
//
//	return false;
// }

bool Values::TObject::operator==(const TObject& Other) const
{
    if (GetType() != Other.GetType())
    {
        return false;
    }

    switch (GetType())
    {
        case BoolType :
            return AsBool()->GetValue() == Other.AsBool()->GetValue();
        case IntType :
            return AsInt()->GetValue() == Other.AsInt()->GetValue();
        case FloatType :
            return AsFloat()->GetValue() == Other.AsFloat()->GetValue();
        case StringType :
            return AsString()->GetValue() == Other.AsString()->GetValue();
    }

    return false;
}

bool Values::TObject::operator!=(const TObject& Other) const
{
    return !(*this == Other);
}

bool Values::TObject::operator!() const
{
    return !AsBool()->GetValue();
}

bool Values::IsType(const TObject& Value, EValueType Type)
{
    return Value.GetType() == Type;
}
