#pragma once

#include <Arduino.h>
#include <functional>

enum EditableKind {
    kInteger,
    kUInt8,
    kDouble,
    kDoubletime,
    kCString,
    kFloat,
    kEnum
};

class Parameter {
    public:
        // Constructor for general parameters with string getters/setters
        Parameter(
            const char* id,
            const char* displayName,
            EditableKind type,
            int section,
            int position,
            std::function<double()> getter,
            std::function<void(double)> setter,
            double minValue,
            double maxValue,
            bool hasHelpText = false,
            const char* helpText = "",
            std::function<bool()> showCondition = [] { return true; },
            std::function<String()> stringGetter = nullptr,
            std::function<void(const String&)> stringSetter = nullptr,
            void* globalVariablePointer = nullptr);

        // Constructor for string parameters
        Parameter(
            const char* id,
            const char* displayName,
            EditableKind type,
            int section,
            int position,
            const std::function<String()>& stringGetter,
            const std::function<void(const String&)>& stringSetter,
            double maxLength,
            bool hasHelpText = false,
            const char* helpText = "",
            const std::function<bool()>& showCondition = [] { return true; },
            void* globalVariablePointer = nullptr);

        // Constructor for numeric parameters (no string getter/setter)
        Parameter(
            const char* id,
            const char* displayName,
            EditableKind type,
            int section,
            int position,
            std::function<double()> getter,
            std::function<void(double)> setter,
            double minValue,
            double maxValue,
            bool hasHelpText = false,
            const char* helpText = "",
            std::function<bool()> showCondition = [] { return true; },
            void* globalVariablePointer = nullptr);

        // Constructor for boolean parameters (using kUInt8 type with 0/1 values)
        Parameter(
            const char* id,
            const char* displayName,
            EditableKind type,
            int section,
            int position,
            const std::function<bool()>& boolGetter,
            const std::function<void(bool)>& boolSetter,
            bool hasHelpText = false,
            const char* helpText = "",
            const std::function<bool()>& showCondition = [] { return true; },
            void* globalVariablePointer = nullptr);

        // Constructor for enum parameters
        Parameter(
            const char* id,
            const char* displayName,
            EditableKind type,
            int section,
            int position,
            const std::function<double()>& getter,
            const std::function<void(double)>& setter,
            const char* const* enumOptions,
            size_t enumCount,
            bool hasHelpText = false,
            const char* helpText = "",
            const std::function<bool()>& showCondition = [] { return true; },
            void* globalVariablePointer = nullptr);

        [[nodiscard]] const char* getId() const;
        [[nodiscard]] const char* getDisplayName() const;
        [[nodiscard]] EditableKind getType() const;
        [[nodiscard]] int getSection() const;
        [[nodiscard]] int getPosition() const;
        [[nodiscard]] double getValue() const;
        void setValue(double value) const;
        [[nodiscard]] bool getBoolValue() const;
        [[nodiscard]] int getIntValue() const;
        [[nodiscard]] float getFloatValue() const;
        [[nodiscard]] uint8_t getUInt8Value() const;
        [[nodiscard]] String getStringValue() const;
        void setStringValue(const String& value) const;
        [[nodiscard]] double getMinValue() const;
        [[nodiscard]] double getMaxValue() const;
        [[nodiscard]] bool hasHelpText() const;
        [[nodiscard]] const char* getHelpText() const;
        [[nodiscard]] bool shouldShow() const;
        [[nodiscard]] String getFormattedValue() const;
        [[nodiscard]] const char* const* getEnumOptions() const;
        [[nodiscard]] size_t getEnumCount() const;
        [[nodiscard]] bool isEnum() const;
        [[nodiscard]] String getEnumDisplayValue() const;
        [[nodiscard]] void* getGlobalVariablePointer() const;
        void setGlobalVariablePointer(void* ptr);
        void syncToGlobalVariable(double value) const;
        void syncToGlobalVariable(const String& value) const;

    private:
        const char* _id;
        const char* _displayName;
        EditableKind _type;
        int _section;
        int _position;
        std::function<double()> _getter;
        std::function<void(double)> _setter;
        const char* const* _enumOptions;
        size_t _enumCount;
        double _minValue;
        double _maxValue;
        bool _hasHelpText;
        const char* _helpText;
        std::function<bool()> _showCondition;
        std::function<String()> _stringGetter;
        std::function<void(const String&)> _stringSetter;
        void* _globalVariablePointer;
};