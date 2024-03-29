#pragma once

#include <format>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <functional>

#include "Core.h"
#include "Logging.h"

using namespace Core;

namespace Values
{
    enum EValueType
    {
        Void,
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

    using TArray = std::vector<TObject>;
    using TMap = std::map<std::string, TObject>;

    class TValue
    {
    public:
        virtual ~TValue() = default;
        virtual bool IsSubscriptable() const = 0;
        virtual bool IsValid() const = 0;
        virtual std::string ToString() = 0;
        virtual std::string ToString() const = 0;
    };

    class TNullValue : public TValue
    {
    public:
        TNullValue() = default;
        bool IsSubscriptable() const override { return false; }
        bool IsValid() const override { return false; }
        std::string ToString() const override { return "nullptr"; }
    };

    class TBoolValue : public TValue
    {
        bool Value;

    public:
        TBoolValue(bool InValue)
            : Value(InValue)
        {
        }
        bool GetValue() const { return Value; }
        void SetValue(bool NewValue) { Value = NewValue; }
        bool IsSubscriptable() const override { return false; }
        bool IsValid() const override { return true; }
        std::string ToString() override { return Value ? "true" : "false"; }
        std::string ToString() const override { return Value ? "true" : "false"; }

        TBoolValue operator==(const TBoolValue& Other) const;
        TBoolValue operator!=(const TBoolValue& Other) const;

        // Bool -> Bool
        operator bool() const { return Value; }

        // Bool -> Int
        operator int() const;
        operator TIntValue() const;

        // Bool -> String
        operator std::string() const;
        operator TStringValue() const;
    };

    class TIntValue : public TValue
    {
        int Value;

    public:
        TIntValue(int InValue)
            : Value(InValue)
        {
        }
        int GetValue() const { return Value; }
        void SetValue(int NewValue) { Value = NewValue; }
        bool IsSubscriptable() const override { return false; }
        bool IsValid() const override { return true; }
        int Increment() { return Value++; }
        int Decrement() { return Value--; }
        std::string ToString() override { return std::to_string(Value); }
        std::string ToString() const override { return std::to_string(Value); }

        // Operators

        TIntValue operator+(const TIntValue& Other) const;
        TIntValue operator-(const TIntValue& Other) const;
        TIntValue operator*(const TIntValue& Other) const;
        TIntValue operator/(const TIntValue& Other) const;
        TBoolValue operator==(const TIntValue& Other) const;
        TBoolValue operator!=(const TIntValue& Other) const;
        TBoolValue operator<(const TIntValue& Other) const;
        TBoolValue operator>(const TIntValue& Other) const;

        TFloatValue operator+(const TFloatValue& Other) const;
        TFloatValue operator-(const TFloatValue& Other) const;
        TFloatValue operator*(const TFloatValue& Other) const;
        TFloatValue operator/(const TFloatValue& Other) const;
        TBoolValue operator==(const TFloatValue& Other) const;
        TBoolValue operator!=(const TFloatValue& Other) const;
        TBoolValue operator<(const TFloatValue& Other) const;
        TBoolValue operator>(const TFloatValue& Other) const;

        // Casting

        // Int -> Int
        operator int() const { return Value; }

        // Int -> Bool
        operator bool() const { return Value; }

        // Int -> Float
        operator float() const { return static_cast<float>(Value); }
        operator TFloatValue() const;

        // Int -> String
        operator std::string() const;
        operator TStringValue() const;
    };

    class TFloatValue : public TValue
    {
        float Value;

    public:
        TFloatValue(float InValue)
            : Value(InValue)
        {
        }
        float GetValue() const { return Value; }
        void SetValue(float NewValue) { Value = NewValue; }
        bool IsSubscriptable() const override { return false; }
        bool IsValid() const override { return true; }
        std::string ToString() override { return std::to_string(Value); }
        std::string ToString() const override { return std::to_string(Value); }

        // Operators
        // Float | Float
        TFloatValue operator+(const TFloatValue& Other) const;
        TFloatValue operator-(const TFloatValue& Other) const;
        TFloatValue operator*(const TFloatValue& Other) const;
        TFloatValue operator/(const TFloatValue& Other) const;
        TBoolValue operator==(const TFloatValue& Other) const;
        TBoolValue operator!=(const TFloatValue& Other) const;
        TBoolValue operator<(const TFloatValue& Other) const;
        TBoolValue operator>(const TFloatValue& Other) const;

        // Float | Int
        TFloatValue operator+(const TIntValue& Other) const;
        TFloatValue operator-(const TIntValue& Other) const;
        TFloatValue operator*(const TIntValue& Other) const;
        TFloatValue operator/(const TIntValue& Other) const;
        TBoolValue operator==(const TIntValue& Other) const;
        TBoolValue operator!=(const TIntValue& Other) const;
        TBoolValue operator<(const TIntValue& Other) const;
        TBoolValue operator>(const TIntValue& Other) const;

        // Casting

        // Float -> Float
        operator float() const { return Value; }

        // Float -> Int
        operator int() const { return static_cast<int>(Value); }
        operator TIntValue() const;

        // Float -> String
        operator std::string() const { return std::to_string(Value); }
        operator TStringValue() const;
    };

    class TStringValue : public TValue
    {
        std::string Value;

    public:
        TStringValue() = default;
        TStringValue(const std::string& InValue)
            : Value(InValue)
        {
        }
        std::string GetValue() const { return Value; }
        void SetValue(const std::string& NewValue) { Value = NewValue; }
        bool IsSubscriptable() const override { return true; }
        bool IsValid() const override { return !Value.empty(); }
        std::string ToString() override { return Value; }
        std::string ToString() const override { return ToString(); }

        std::string At(int Index) const;
        static std::string Join(const TArray& Iterator, const std::string& Separator);
        static std::string Join(const TArrayValue& Array, const std::string& Separator);

        // String -> String
        operator std::string() const { return Value; }
        operator const std::string() const { return Value; }
        operator bool() const { return !Value.empty(); }
        TStringValue operator+(const TStringValue& Other) const;
    };

    class TArrayValue : public TValue
    {
        TArray Value;

    public:
        TArrayValue() = default;
        TArrayValue(const TArray& InValue)
            : Value(InValue)
        {
        }
        TArray GetValue() const { return Value; }
        bool IsSubscriptable() const override { return true; }
        bool IsValid() const override { return true; }
        std::string ToString() override { return "#[" + TStringValue::Join(Value, ",") + "]"; }
        std::string ToString() const override { return ToString(); }

        void Append(const TObject& InValue) { Value.push_back(InValue); }
        // void	  Remove(int Index) { Value.erase(Value.begin() + Index); }
        void Empty() { Value.clear(); }
        TIntValue Size() const { return TIntValue(static_cast<int>(Value.size())); }
        TObject* At(int Index);
        bool Contains(const TObject& InValue);

        operator bool() const { return !Value.empty(); }
        TObject& operator[](int Index) { return Value[Index]; }
    };

    class TMapValue : public TValue
    {
        TMap Value;

    public:
        TMapValue(const TMap& InValue)
            : Value(InValue)
        {
        }
        TMap GetValue() const { return Value; }
        bool IsSubscriptable() const override { return false; }
        bool IsValid() const override { return true; }
        std::string ToString() override { return "Map"; }
        std::string ToString() const override { return "Map"; }

        TArrayValue GetKeys() const;
        TArrayValue GetValues() const;
        bool HasKey(const std::string& Key) const;
        TObject* At(const std::string& Key) { return &Value[Key]; }
        TObject* At(const TStringValue& Key) { return &Value[Key.GetValue()]; }

        explicit operator bool() const { return !Value.empty(); }
        TObject& operator[](const std::string& Key) { return Value[Key]; }
    };

    class TObject
    {
        std::unique_ptr<TValue> Value;
        EValueType Type = NullType;

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
            explicit Iterator(const Pointer InPtr)
                : Ptr(InPtr)
            {
            }

            Reference operator*() const { return *Ptr; }

            Pointer operator->() const { return Ptr; }

            // Prefix increment
            Iterator& operator++()
            {
                Ptr++;
                return *this;
            }

            // Postfix increment
            Iterator operator++(int)
            {
                const Iterator Tmp = *this;
                ++(*this);
                return Tmp;
            }

            friend bool operator==(const Iterator& Left, const Iterator& Right) { return Left.Ptr == Right.Ptr; }
            friend bool operator!=(const Iterator& Left, const Iterator& Right) { return Left.Ptr != Right.Ptr; }
        };

    public:
        std::map<int, std::function<void()>> FunctionMap;

        // Constructors
        TObject() noexcept
        {
            Value = nullptr;
            Type = NullType;
        }
        TObject(TObject& Other) noexcept
        {
            *this = Other;
            Type = Other.Type;
        }
        TObject(const TObject& Other) noexcept
        {
            *this = Other;
            Type = Other.Type;
        }
        TObject(TObject&& Other) noexcept
        {
            Value = std::move(Other.Value);
            Type = Other.Type;
        }
        ~TObject() noexcept = default;
        TObject(bool InValue) noexcept
        {
            Value = std::make_unique<TBoolValue>(InValue);
            Type = BoolType;
        } // Bool
        TObject(const TBoolValue& InValue) noexcept
        {
            Value = std::make_unique<TBoolValue>(InValue);
            Type = BoolType;
        } // Bool
        TObject(int InValue) noexcept
        {
            Value = std::make_unique<TIntValue>(InValue);
            Type = IntType;
        } // Integer
        TObject(const TIntValue& InValue) noexcept
        {
            Value = std::make_unique<TIntValue>(InValue);
            Type = IntType;
        } // Integer
        TObject(float InValue) noexcept
        {
            Value = std::make_unique<TFloatValue>(InValue);
            Type = FloatType;
        } // Float
        TObject(const TFloatValue& InValue) noexcept
        {
            Value = std::make_unique<TFloatValue>(InValue);
            Type = FloatType;
        } // Float
        TObject(const std::string& InValue) noexcept
        {
            Value = std::make_unique<TStringValue>(InValue);
            Type = StringType;
        } // String
        TObject(char InChar) noexcept
        {
            auto InValue = std::string(1, InChar);
            Value = std::make_unique<TStringValue>(InValue);
            Type = StringType;
        }
        TObject(const char* InChar) noexcept
        {
            auto InValue = std::string(1, *InChar);
            Value = std::make_unique<TStringValue>(InValue);
            Type = StringType;
        }
        TObject(const TStringValue& InValue) noexcept
        {
            Value = std::make_unique<TStringValue>(InValue);
            Type = StringType;
        } // String
        TObject(const TArrayValue& InValue) noexcept
        {
            Value = std::make_unique<TArrayValue>(InValue);
            Type = ArrayType;
        } // Array
        TObject(const std::initializer_list<TObject>& InValue) noexcept
        {
            Value = std::make_unique<TArrayValue>(InValue);
            Type = ArrayType;
        }
        TObject(const TMapValue& InValue) noexcept
        {
            Value = std::make_unique<TMapValue>(InValue);
            Type = MapType;
        }

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

        TBoolValue GetBool() const { return *AsBool(); }
        TIntValue GetInt() const { return *AsInt(); }
        TFloatValue GetFloat() const { return *AsFloat(); }
        TStringValue GetString() const { return *AsString(); }
        TArrayValue GetArray() const { return *AsArray(); }
        TMapValue GetMap() const { return *AsMap(); }

        bool IsValid() { return Value->IsValid(); }
        bool IsValid() const { return Value->IsValid(); }

        TObject At(const TObject& Index)
        {
            if (!IsSubscriptable())
            {
                return {};
            }
            if (!IsValid())
            {
                return {};
            }

            if (Type == StringType)
            {
                int StringIndex = Index.AsInt()->GetValue();
                size_t StringSize = AsString()->GetValue().size();
                if (StringIndex > StringSize)
                {
                    return {};
                }
                const std::string String = AsString()->GetValue();
                const char* Char = String.data() + StringIndex;
                return TObject(Char);
            }
            if (Type == ArrayType)
            {
                if (Index.AsInt()->GetValue() > GetArray().GetValue().size())
                {
                    return {};
                }
                int ArrayIndex = Index.AsInt()->GetValue();
                return TObject(AsArray()->At(ArrayIndex)->Value.get());
            }
            if (Type == MapType)
            {
                const std::string MapKey = Index.AsString()->GetValue();
                return TObject(AsMap()->At(MapKey)->Value.get());
            }
            return TObject();
        }

        TObject& At(const TObject& Index) const { return At(Index); }

        EValueType GetType() const { return Type; }

        bool IsSubscriptable() { return Value->IsSubscriptable(); }
        bool IsSubscriptable() const { return Value->IsSubscriptable(); }
        std::string ToString() { return Value->ToString(); }
        std::string ToString() const { return Value->ToString(); }

        // Operators

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

        TObject operator[](const std::string& Key)
        {
            return At(TObject(Key));
        }

        TObject& operator[](const TStringValue& Key) const { return *AsMap()->At(Key); }

        TObject& operator[](int Index) const { return *AsArray()->At(Index); }

        TObject& operator[](const TIntValue& Index) { return *AsArray()->At(Index); }

        TObject& operator[](const TObject& Arg) const { return At(Arg); }

        TObject operator+(const TObject& Other) const;
        TObject operator+=(const TObject& Other) const { return *this + Other; }
        TObject operator-(const TObject& Other) const;
        TObject operator*(const TObject& Other) const;
        TObject operator/(const TObject& Other) const;
        TObject operator<(const TObject& Other) const;
        TObject operator>(const TObject& Other) const;

        // bool operator==(TObject& Other);
        bool operator==(const TObject& Other) const;
        bool operator!=(const TObject& Other) const;
        bool operator!() const;

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

        operator bool() { return static_cast<bool>(Value.get()); }
        operator bool() const { return static_cast<bool>(*this); }
        operator std::string() { return ToString(); }
        operator std::string() const { return ToString(); }
    };

    static bool IsType(const TObject& Value, EValueType Type);

    class TIdentifier
    {
    protected:
        std::string Name;

    public:
        TIdentifier() = default;
        virtual ~TIdentifier() = default;
        virtual bool IsValid() = 0;
        virtual TObject GetValue() = 0;
        virtual TObject* GetValuePtr() = 0;
        virtual std::string ToString() = 0;
    };

    class TLiteral : public TIdentifier
    {
        TObject Value;

    public:
        TLiteral(TObject InValue)
            : Value(std::move(InValue))
        {
        }
        std::string GetName() { return Name; }
        bool IsValid() override { return Value.GetType() != NullType; }
        TObject GetValue() override { return Value; }
        TObject* GetValuePtr() override { return &Value; }
        std::string ToString() override
        {
            if (IsValid())
            {
                return GetValue().ToString();
            }
            else
            {
                return "Value is nullptr.";
            }
        }
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
            : TIdentifier(Other)
        {
            Name = Other.Name;
            *this = Other;
        }
        TVariable(const TVariable& Other) noexcept
            : TIdentifier(Other)
        {
            Name = Other.Name;
            *this = Other;
        }

        ~TVariable() override = default;
        bool IsValid() override { return Value != nullptr; }
        TObject GetValue() override { return *Value; }
        TObject* GetValuePtr() override { return Value; }
        void SetValue(TObject* InValue) { Value = InValue; }
        std::string GetName() { return Name; }
        std::string ToString() override
        {
            if (IsValid())
            {
                return std::format("{}, {}", GetName(), GetValue().ToString());
            }
            else
            {
                return std::format("{} is undefined.", GetName());
            }
        }

        TVariable& operator=(const TVariable& Other)
        {
            Name = Other.Name;
            SetValue(Other.Value);
            return *this;
        }
    };

    using TArguments = std::vector<std::shared_ptr<TIdentifier>>;

    using TFunctor = void(TArguments* InArguments, TObject* ReturnValue, bool& bResult);
    class TFunction
    {
        TFunctor* Func = nullptr;

    public:
        TFunction() = default;
        TFunction(TFunctor* InFunc) { Func = InFunc; }
        bool Invoke(TArguments* Arguments, TObject* ReturnValue) const
        {
            // The function _should_ be defined by now, but in case it isn't...
            if (Func == nullptr)
            {
                Logging::Error("Function is not defined.");
                return false;
            }

            // Execute the function, passing the result by reference
            bool bResult = false;
            Func(Arguments, ReturnValue, bResult);
            return bResult;
        }
    };
} // namespace Values
