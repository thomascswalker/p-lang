#pragma once

#include <format>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "core.h"

namespace Values
{
	enum EValueType
	{
		NullType,
		BoolType,
		IntType,
		FloatType,
		StringType,
		ArrayType,
		MapType
	};

	class TObject;
	class TValue;
	class TBoolValue;
	class TIntValue;
	class TFloatValue;
	class TStringValue;
	class TArrayValue;
	class TMapValue;

	typedef std::vector<TObject>		   TArray;
	typedef std::map<std::string, TObject> TMap;

	class TValue
	{
	public:
		virtual ~TValue() = default;
	};

	class TNullValue : public TValue
	{
	public:
		TNullValue() = default;
	};

	class TBoolValue : public TValue
	{
		bool Value;

	public:
		TBoolValue(bool InValue) : Value(InValue){};
		bool GetValue() const { return Value; }

		TBoolValue operator==(const TBoolValue& Other) const;
		TBoolValue operator!=(const TBoolValue& Other) const;

		// Bool -> Int
		explicit operator int() const;
		explicit operator TIntValue() const;

		// Bool -> String
		explicit operator std::string() const;
		explicit operator TStringValue() const;
	};

	class TIntValue : public TValue
	{
		int Value;

	public:
		TIntValue(int InValue) : Value(InValue){};
		int GetValue() const { return Value; }

		// Operators

		TIntValue  operator+(const TIntValue& Other) const;
		TIntValue  operator-(const TIntValue& Other) const;
		TIntValue  operator*(const TIntValue& Other) const;
		TIntValue  operator/(const TIntValue& Other) const;
		TBoolValue operator==(const TIntValue& Other) const;
		TBoolValue operator!=(const TIntValue& Other) const;
		TBoolValue operator<(const TIntValue& Other) const;
		TBoolValue operator>(const TIntValue& Other) const;

		TFloatValue operator+(const TFloatValue& Other) const;
		TFloatValue operator-(const TFloatValue& Other) const;
		TFloatValue operator*(const TFloatValue& Other) const;
		TFloatValue operator/(const TFloatValue& Other) const;
		TBoolValue	operator==(const TFloatValue& Other) const;
		TBoolValue	operator!=(const TFloatValue& Other) const;
		TBoolValue	operator<(const TFloatValue& Other) const;
		TBoolValue	operator>(const TFloatValue& Other) const;

		// Casting

		// Int -> Bool
		operator bool() const { return Value; }

		// Int -> Float
		operator float() const { return (float)Value; }
		operator TFloatValue() const;

		// Int -> String
		operator std::string() const;
		operator TStringValue() const;
	};

	class TFloatValue : public TValue
	{
		float Value;

	public:
		TFloatValue(float InValue) : Value(InValue){};
		float GetValue() const { return Value; }

		// Operators
		// Float | Float
		TFloatValue operator+(const TFloatValue& Other) const;
		TFloatValue operator-(const TFloatValue& Other) const;
		TFloatValue operator*(const TFloatValue& Other) const;
		TFloatValue operator/(const TFloatValue& Other) const;
		TBoolValue	operator==(const TFloatValue& Other) const;
		TBoolValue	operator!=(const TFloatValue& Other) const;
		TBoolValue	operator<(const TFloatValue& Other) const;
		TBoolValue	operator>(const TFloatValue& Other) const;

		// Float | Int
		TFloatValue operator+(const TIntValue& Other) const;
		TFloatValue operator-(const TIntValue& Other) const;
		TFloatValue operator*(const TIntValue& Other) const;
		TFloatValue operator/(const TIntValue& Other) const;
		TBoolValue	operator==(const TIntValue& Other) const;
		TBoolValue	operator!=(const TIntValue& Other) const;
		TBoolValue	operator<(const TIntValue& Other) const;
		TBoolValue	operator>(const TIntValue& Other) const;

		// Casting

		// Float -> Int
		operator int() const { return (int)Value; }
		operator TIntValue() const;

		// Float -> String
		operator std::string() const { return std::to_string(Value); }
		operator TStringValue() const;
	};

	class TStringValue : public TValue
	{
		std::string Value;

	public:
		TStringValue(const std::string& InValue) : Value(InValue){};
		std::string GetValue() const { return Value; }

		operator std::string() const { return Value; }
		TStringValue operator+(const TStringValue& Other) const;
	};

	class TArrayValue : public TValue
	{
		TArray Value;

	public:
		TArrayValue(const TArray& InValue) : Value(InValue){};
		TArray GetValue() const { return Value; }

		TObject& operator[](int Index) { return Value[Index]; }
	};

	class TMapValue : public TValue
	{
		TMap Value;

	public:
		TMapValue(const TMap& InValue) : Value(InValue){};
		TMap GetValue() const { return Value; }

		TObject& operator[](const std::string& Key) { return Value[Key]; }
	};

	class TObject
	{
		std::unique_ptr<TValue> Value;
		EValueType				Type;

		struct Iterator
		{
			using Category = std::forward_iterator_tag;
			using DifferenceType = std::ptrdiff_t;
			using ValueType = TObject;
			using Pointer = TObject*;
			using Reference = TObject&;

		private:
			Pointer Ptr;

		public:
			explicit Iterator(Pointer InPtr) : Ptr(InPtr){};

			Reference operator*() const { return *Ptr; }

			Pointer operator->() { return Ptr; }

			// Prefix increment
			Iterator& operator++()
			{
				Ptr++;
				return *this;
			}

			// Postfix increment
			Iterator operator++(int)
			{
				Iterator Tmp = *this;
				++(*this);
				return Tmp;
			}

			friend bool operator==(const Iterator& Left, const Iterator& Right) { return Left.Ptr == Right.Ptr; };

			friend bool operator!=(const Iterator& Left, const Iterator& Right) { return Left.Ptr != Right.Ptr; };
		};

	public:
		// Constructors
		TObject()
		{
			Value = nullptr;
			Type = NullType;
		};
		TObject(const TObject& Other)
		{
			*this = Other;
			Type = Other.Type;
		};
		TObject(bool InValue) { Value = std::make_unique<TBoolValue>(InValue); };				  // Bool
		TObject(int InValue) { Value = std::make_unique<TIntValue>(InValue); };					  // Integer
		TObject(float InValue) { Value = std::make_unique<TFloatValue>(InValue); };				  // Float
		TObject(const std::string& InValue) { Value = std::make_unique<TStringValue>(InValue); }; // String
		TObject(const TArrayValue& InValue) { Value = std::make_unique<TArrayValue>(InValue); };  // Array
		TObject(const TMapValue& InValue) { Value = std::make_unique<TMapValue>(InValue); };	  // Map

		// Methods
		EValueType GetType() { return Type; }

		TBoolValue&	  AsBool() const { return *Core::Cast<TBoolValue>(Value.get()); }
		TIntValue&	  AsInt() const { return *Core::Cast<TIntValue>(Value.get()); }
		TFloatValue&  AsFloat() const { return *Core::Cast<TFloatValue>(Value.get()); }
		TStringValue& AsString() const { return *Core::Cast<TStringValue>(Value.get()); }
		TArrayValue&  AsArray() const { return *Core::Cast<TArrayValue>(Value.get()); }
		TMapValue&	  AsMap() const { return *Core::Cast<TMapValue>(Value.get()); }

		TBoolValue	 GetBool() const { return AsBool().GetValue(); }
		TIntValue	 GetInt() const { return AsInt().GetValue(); }
		TFloatValue	 GetFloat() const { return AsFloat().GetValue(); }
		TStringValue GetString() const { return AsString().GetValue(); }
		TArrayValue	 GetArray() const { return AsArray().GetValue(); }
		TMapValue	 GetMap() const { return AsMap().GetValue(); }

		bool HasKey(const std::string& Key)
		{
			if (Type != MapType)
			{
				throw std::runtime_error("Object is not subscriptable.");
			}
			auto MapType = AsMap().GetValue();
			return MapType.find(Key) != MapType.end();
		}

		// Operators
		TObject& operator=(const TObject& Other)
		{
			switch (Other.Type)
			{
				case (BoolType) :
				{
					Value = std::make_unique<TBoolValue>(Other.GetBool());
					break;
				}
				case (IntType) :
				{
					Value = std::make_unique<TIntValue>(Other.GetInt());
					break;
				}
				case (FloatType) :
				{
					Value = std::make_unique<TFloatValue>(Other.GetFloat());
					break;
				}
				case (StringType) :
				{
					Value = std::make_unique<TStringValue>(Other.GetString());
					break;
				}
				case (ArrayType) :
				{
					Value = std::make_unique<TArrayValue>(Other.GetArray());
					break;
				}
				case (MapType) :
				{
					Value = std::make_unique<TMapValue>(Other.GetMap());
					break;
				}
				default :
				{
					break;
				}
			}
			Type = Other.Type;
			return *this;
		}

		TObject& operator[](const std::string& Key)
		{
			if (Type != MapType || Value == nullptr)
			{
				throw std::runtime_error("Invalid type.");
			}
			if (!HasKey(Key))
			{
				throw std::runtime_error(std::format("Missing key: {}", Key));
			}
		}

		TObject& operator[](int Index)
		{
			if (Type != ArrayType || Value == nullptr)
			{
				throw std::runtime_error("Invalid type.");
			}
			TArrayValue ArrayType = AsArray();
			if (ArrayType.GetValue().size() < Index)
			{
				throw std::runtime_error("Index out of bounds.");
			}
			return ArrayType[Index];
		}
	};
} // namespace Values
