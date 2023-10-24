#include <functional>
#include "value.h"

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

TStringValue Values::TStringValue::operator+(const TStringValue& Other) const
{
	return TStringValue(Value + Other.GetValue());
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
					return AsInt().GetValue() + Other.AsInt().GetValue();
				case FloatType :
					return AsInt().GetValue() + Other.AsFloat().GetValue();
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
					return AsFloat().GetValue() + Other.AsInt().GetValue();
				case FloatType :
					return AsFloat().GetValue() + Other.AsFloat().GetValue();
				default :
					break;
			}
			break;
		}
		case StringType :
		{
			return AsString().GetValue() + Other.AsString().GetValue();
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
					return AsInt().GetValue() - Other.AsInt().GetValue();
				case FloatType :
					return AsInt().GetValue() - Other.AsFloat().GetValue();
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
					return AsFloat().GetValue() - Other.AsInt().GetValue();
				case FloatType :
					return AsFloat().GetValue() - Other.AsFloat().GetValue();
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
					return AsInt().GetValue() * Other.AsInt().GetValue();
				case FloatType :
					return AsInt().GetValue() * Other.AsFloat().GetValue();
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
					return AsFloat().GetValue() * Other.AsInt().GetValue();
				case FloatType :
					return AsFloat().GetValue() * Other.AsFloat().GetValue();
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
					return AsInt().GetValue() / Other.AsInt().GetValue();
				case FloatType :
					return AsInt().GetValue() / Other.AsFloat().GetValue();
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
					return AsFloat().GetValue() / Other.AsInt().GetValue();
				case FloatType :
					return AsFloat().GetValue() / Other.AsFloat().GetValue();
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

TObject Values::TObject::operator>(const TObject& Other) const { TOBJECT_COMPARE_OP_BODY(>) }

TObject Values::TObject::operator==(const TObject& Other) const { TOBJECT_COMPARE_OP_BODY(==) }

TObject Values::TObject::operator!=(const TObject& Other) const
{
	TOBJECT_COMPARE_OP_BODY(!=)
}
