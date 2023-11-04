#pragma once

#include <format>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <tuple>

#include "core.h"

using namespace Core;

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
		MapType,
		TypeCount
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
		virtual bool		IsSubscriptable() = 0;
		virtual bool		IsValid() const = 0;
		virtual std::string ToString() = 0;
		virtual std::string ToString() const = 0;
	};

	class TNullValue : public TValue
	{
	public:
		TNullValue() = default;
		bool		IsSubscriptable() override { return false; }
		bool		IsValid() const override { return false; }
		std::string ToString() const override { return "nullptr"; }
	};

	class TBoolValue : public TValue
	{
		bool Value;

	public:
		TBoolValue(bool InValue) : Value(InValue){};
		bool		GetValue() const { return Value; }
		bool		IsSubscriptable() override { return false; }
		bool		IsValid() const override { return true; }
		std::string ToString() override { return Value ? "true" : "false"; }
		std::string ToString() const override { return Value ? "true" : "false"; }

		TBoolValue operator==(const TBoolValue& Other) const;
		TBoolValue operator!=(const TBoolValue& Other) const;

		// Bool -> Bool
		explicit operator bool() const { return Value; }

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
		int			GetValue() const { return Value; }
		bool		IsSubscriptable() override { return false; }
		bool		IsValid() const override { return true; }
		int			Increment() { return Value++; }
		int			Decrement() { return Value--; }
		std::string ToString() override { return std::to_string(Value); }
		std::string ToString() const override { return std::to_string(Value); }

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

		// Int -> Int
		operator int() const { return Value; }

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
		float		GetValue() const { return Value; }
		bool		IsSubscriptable() override { return false; }
		bool		IsValid() const override { return true; }
		std::string ToString() override { return std::to_string(Value); }
		std::string ToString() const override { return std::to_string(Value); }

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

		// Float -> Float
		operator float() const { return Value; }

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
		TStringValue(){};
		TStringValue(const std::string& InValue) : Value(InValue){};
		std::string GetValue() const { return Value; }
		bool		IsSubscriptable() override { return true; }
		bool		IsValid() const override { return Value != ""; }
		std::string ToString() override { return "\"" + Value + "\""; }
		std::string ToString() const override { return ToString(); }

		std::string		   At(int Index) const;
		static std::string Join(const TArray& Iterator, const std::string& Separator);
		static std::string Join(const TArrayValue& Array, const std::string& Separator);

		// String -> String
		operator std::string() const { return Value; }
		operator bool() const { return Value != ""; }
		TStringValue operator+(const TStringValue& Other) const;
	};

	class TArrayValue : public TValue
	{
		TArray Value;

	public:
		TArrayValue(){};
		TArrayValue(const TArray& InValue) : Value(InValue){};
		TArray		GetValue() const { return Value; }
		bool		IsSubscriptable() override { return true; }
		bool		IsValid() const override { return !Value.empty(); }
		std::string ToString() override { return "#[" + TStringValue::Join(Value, ",") + "]"; }
		std::string ToString() const override { return ToString(); }

		void Append(const TObject& InValue) { Value.push_back(InValue); }
		// void	  Remove(int Index) { Value.erase(Value.begin() + Index); }
		void	  Empty() { Value.clear(); }
		TIntValue Size() const { return (int)Value.size(); }
		TObject*  At(int Index);
		bool	  Contains(const TObject& InValue);

		operator bool() const { return !Value.empty(); }
		TObject& operator[](int Index) { return Value[Index]; }
	};

	class TMapValue : public TValue
	{
		TMap Value;

	public:
		TMapValue(const TMap& InValue) : Value(InValue){};
		TMap		GetValue() const { return Value; }
		bool		IsSubscriptable() override { return false; }
		bool		IsValid() const override { return !Value.empty(); }
		std::string ToString() override { return "Map"; }
		std::string ToString() const override { return "Map"; }

		TArrayValue GetKeys() const;
		TArrayValue GetValues() const;

		operator bool() const { return !Value.empty(); }
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
		std::map<int, std::function<void()>> FunctionMap;

		// Constructors
		TObject() noexcept
		{
			Value = nullptr;
			Type = NullType;
		};
		TObject(TObject& Other) noexcept
		{
			*this = Other;
			Type = Other.Type;
		};
		TObject(const TObject& Other) noexcept
		{
			*this = Other;
			Type = Other.Type;
		};
		TObject(TObject&& Other) noexcept
		{
			Value = std::move(Other.Value);
			Type = Other.Type;
		}
		~TObject() noexcept {};
		TObject(bool InValue) noexcept
		{
			Value = std::make_unique<TBoolValue>(InValue);
			Type = BoolType;
		}; // Bool
		TObject(const TBoolValue& InValue) noexcept
		{
			Value = std::make_unique<TBoolValue>(InValue);
			Type = BoolType;
		}; // Bool
		TObject(int InValue) noexcept
		{
			Value = std::make_unique<TIntValue>(InValue);
			Type = IntType;
		}; // Integer
		TObject(const TIntValue& InValue) noexcept
		{
			Value = std::make_unique<TIntValue>(InValue);
			Type = IntType;
		}; // Integer
		TObject(float InValue) noexcept
		{
			Value = std::make_unique<TFloatValue>(InValue);
			Type = FloatType;
		}; // Float
		TObject(const TFloatValue& InValue) noexcept
		{
			Value = std::make_unique<TFloatValue>(InValue);
			Type = FloatType;
		}; // Float
		TObject(const std::string& InValue) noexcept
		{
			Value = std::make_unique<TStringValue>(InValue);
			Type = StringType;
		}; // String
		TObject(char InChar) noexcept
		{
			std::string InValue = std::string(1, InChar);
			Value = std::make_unique<TStringValue>(InValue);
			Type = StringType;
		}
		TObject(const TStringValue& InValue) noexcept
		{
			Value = std::make_unique<TStringValue>(InValue);
			Type = StringType;
		}; // String
		TObject(const TArrayValue& InValue) noexcept
		{
			Value = std::make_unique<TArrayValue>(InValue);
			Type = ArrayType;
		}; // Array
		TObject(const std::initializer_list<TObject>& InValue) noexcept
		{
			Value = std::make_unique<TArrayValue>(InValue);
			Type = ArrayType;
		}
		TObject(const TMapValue& InValue) noexcept
		{
			Value = std::make_unique<TMapValue>(InValue);
			Type = MapType;
		};

		// Methods
		EValueType GetType() { return Type; }

		TBoolValue* AsBool() const
		{
			if (Value == nullptr)
			{
				return nullptr;
			}
			return Cast<TBoolValue>(Value.get());
		}
		TIntValue* AsInt() const
		{
			if (Value == nullptr)
			{
				return nullptr;
			}
			return Cast<TIntValue>(Value.get());
		}
		TFloatValue* AsFloat() const
		{
			if (Value == nullptr)
			{
				return nullptr;
			}
			return Cast<TFloatValue>(Value.get());
		}
		TStringValue* AsString() const
		{
			if (Value == nullptr)
			{
				return nullptr;
			}
			return Cast<TStringValue>(Value.get());
		}
		TArrayValue* AsArray() const
		{
			if (Value == nullptr)
			{
				return nullptr;
			}
			return Cast<TArrayValue>(Value.get());
		}
		TMapValue* AsMap() const
		{
			if (Value == nullptr)
			{
				return nullptr;
			}
			return Cast<TMapValue>(Value.get());
		}

		TBoolValue	 GetBool() const { return *AsBool(); }
		TIntValue	 GetInt() const { return *AsInt(); }
		TFloatValue	 GetFloat() const { return *AsFloat(); }
		TStringValue GetString() const { return *AsString(); }
		TArrayValue	 GetArray() const { return *AsArray(); }
		TMapValue	 GetMap() const { return *AsMap(); }

		bool IsValid() { return Value.get() != nullptr; }
		bool IsValid() const { return Value.get() != nullptr; }

		TObject& At(const TObject& Index)
		{
			TObject Result;
			if (!IsSubscriptable())
			{
				return Result;
			}
			if (!IsValid())
			{
				return Result;
			}

			if (Type == StringType)
			{
				auto StringIndex = Index.AsInt()->GetValue();
				auto StringSize = AsString()->GetValue().size();
				if (StringIndex > StringSize)
				{
					return Result;
				}
				auto String = AsString()->GetValue();
				Result = String.at(StringIndex);
			}
			else if (Type == ArrayType)
			{
				if (Index.AsInt()->GetValue() > GetArray().GetValue().size())
				{
					return Result;
				}
				Result = AsArray()->At(Index.AsInt()->GetValue());
			}
			else if (Type == MapType)
			{
				Result = AsMap()->GetValue()[Index.AsString()->GetValue()];
			}

			return Result;
		}

		TObject& At(const TObject& Index) const { return At(Index); }

		EValueType GetType() const { return Type; }
		bool	   HasKey(const std::string& Key)
		{
			if (Type != MapType)
			{
				throw std::runtime_error("Object is not subscriptable.");
			}
			auto MapType = AsMap()->GetValue();
			return MapType.find(Key) != MapType.end();
		}

		bool		IsSubscriptable() { return Value->IsSubscriptable(); }
		bool		IsSubscriptable() const { return Value->IsSubscriptable(); }
		std::string ToString() { return Value->ToString(); }
		std::string ToString() const { return Value->ToString(); }

		// Operators

		friend std::ostringstream& operator<<(std::ostringstream& Stream, const TObject& Object);

		TObject& operator=(const TObject& Other)
		{
			Type = Other.Type;
			TStringValue S("");
			switch (Other.Type)
			{
				case (IntType) :
				{
					Value = std::make_unique<TIntValue>(Other.GetInt());
					break;
				}
				case (BoolType) :
				{
					Value = std::make_unique<TBoolValue>(Other.GetBool());
					break;
				}
				case (FloatType) :
				{
					Value = std::make_unique<TFloatValue>(Other.GetFloat());
					break;
				}
				case (StringType) :
				{
					S = Other.GetString();
					Value = std::make_unique<TStringValue>(S);
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

			return *this;
		}

		TObject& operator[](const std::string& Key) { At(Key); }

		TObject& operator[](const TStringValue& Key) { return At(Key); }

		TObject& operator[](int Index) { return At(Index); }

		TObject& operator[](const TIntValue& Index) { return At(Index); }

		TObject& operator[](const TObject& Arg) { At(Arg); }

		TObject operator+(const TObject& Other) const;
		TObject operator+=(const TObject& Other) const { return *this + Other; }
		TObject operator-(const TObject& Other) const;
		TObject operator*(const TObject& Other) const;
		TObject operator/(const TObject& Other) const;
		TObject operator<(const TObject& Other) const;
		TObject operator>(const TObject& Other) const;

		TBoolValue operator==(TObject& Other);
		TBoolValue operator==(const TObject& Other) const;
		TBoolValue operator!=(const TObject& Other) const;
		TBoolValue operator!() const;

		TObject& operator++() // Prefix
		{
			TIntValue* V = nullptr;
			switch (Type)
			{
				case IntType :
					V = Cast<TIntValue>(Value.get());
					if (V)
					{
						Value = std::make_unique<TIntValue>(V->Increment());
					}
					return *this;
				default :
					throw std::runtime_error("Cannot increment this value.");
			}
		}
		TObject operator++(int)
		{
			TIntValue* V = nullptr;
			switch (Type)
			{
				case IntType :
					V = Cast<TIntValue>(Value.get());
					if (V)
					{
						Value = std::make_unique<TIntValue>(V->Increment());
					}
					return *this;
				default :
					throw std::runtime_error("Cannot increment this value.");
			}
		} // Postfix

		TObject& operator--() // Prefix
		{
			TIntValue* V = nullptr;
			switch (Type)
			{
				case IntType :
					V = Cast<TIntValue>(Value.get());
					if (V)
					{
						Value = std::make_unique<TIntValue>(V->Decrement());
					}
					return *this;
				default :
					throw std::runtime_error("Cannot increment this value.");
			}
		}
		TObject operator--(int)
		{
			TIntValue* V = nullptr;
			switch (Type)
			{
				case IntType :
					V = Cast<TIntValue>(Value.get());
					if (V)
					{
						Value = std::make_unique<TIntValue>(V->Decrement());
					}
					return *this;
				default :
					throw std::runtime_error("Cannot increment this value.");
			}
		} // Postfix

		operator bool() { return (bool)Value.get(); }
		operator bool() const { return (bool)*this; }
		operator std::string() { return ToString(); }
		operator std::string() const { return ToString(); }
	};

	static bool IsType(const TObject& Value, EValueType Type);

	class TIdentifier
	{
	protected:
		std::string Name;

	public:
		TIdentifier(){};
		virtual ~TIdentifier() = default;
		virtual TObject GetValue() = 0;
	};

	class TLiteral : public TIdentifier
	{
		TObject Value;

	public:
		TLiteral(const TObject& InValue) : Value(InValue){};
		std::string GetName() { return Name; }
		TObject		GetValue() override { return Value; }
	};

	using TObjectPtr = std::unique_ptr<TObject>;
	class TVariable : public TIdentifier
	{
		TObject* Value;

	public:
		TVariable(const std::string& InName, TObject* InValue)
		{
			Name = InName;
			SetValue(InValue);
		}

		TVariable(TVariable& Other) noexcept
		{
			Name = Other.Name;
			*this = Other;
		};
		TVariable(const TVariable& Other) noexcept
		{
			Name = Other.Name;
			*this = Other;
		};

		~TVariable() = default;
		TObject		GetValue() override { return *Value; }
		TObject*	GetValuePtr() { return Value; }
		void		SetValue(TObject* InValue) { Value = InValue; }
		std::string GetName() { return Name; }

		TVariable& operator=(const TVariable& Other)
		{
			Name = Other.Name;
			SetValue(Other.Value);
		}
	};

	using TArguments = std::vector<TIdentifier*>;

	typedef void (*TFunctor)(TArguments* InArguments, bool& bResult);
	class TFunction
	{
		TFunctor Func;

	public:
		TFunction(){};
		TFunction(TFunctor InFunc) { Func = InFunc; }
		bool Invoke(TArguments* Arguments)
		{
			bool bResult = false;
			Func(Arguments, bResult);
			return bResult;
		}
	};
} // namespace Values
