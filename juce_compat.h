#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#ifndef jassert
#define jassert(expr) do { if (!(expr)) { std::abort(); } } while (0)
#endif

#ifndef jassertfalse
#define jassertfalse jassert(false)
#endif

#ifndef JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR
#define JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Class) \
    Class(const Class&) = delete; \
    Class& operator=(const Class&) = delete;
#endif

#ifndef JUCE_DECLARE_NON_COPYABLE
#define JUCE_DECLARE_NON_COPYABLE(Class) \
    Class(const Class&) = delete; \
    Class& operator=(const Class&) = delete;
#endif

#ifndef JUCE_USE_CXX11_NULLPTR
#define JUCE_USE_CXX11_NULLPTR 1
#endif

#ifndef JUCE_COMPILER_SUPPORTS_NULLPTR
#define JUCE_COMPILER_SUPPORTS_NULLPTR 1
#endif

#ifndef JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
#define JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS 1
#endif

#ifndef JUCE_COMPILER_SUPPORTS_LAMBDAS
#define JUCE_COMPILER_SUPPORTS_LAMBDAS 1
#endif

#ifndef JUCE_COMPILER_SUPPORTS_VARIADIC_TEMPLATES
#define JUCE_COMPILER_SUPPORTS_VARIADIC_TEMPLATES 1
#endif

#ifndef JUCE_MODAL_LOOPS_PERMITTED
#define JUCE_MODAL_LOOPS_PERMITTED 0
#endif

namespace juce
{
    using uint = unsigned int;
    using uchar = unsigned char;
    using ulong = unsigned long;
    using int64 = std::int64_t;
    using uint64 = std::uint64_t;

    template <typename T>
    inline T jlimit(T a, T b, T v)
    {
        if (a > b)
            std::swap(a, b);
        return std::max(a, std::min(b, v));
    }

    template <typename T>
    inline int roundToInt(T v)
    {
        return static_cast<int>(std::lround(static_cast<double>(v)));
    }

    template <typename T>
    inline bool approximatelyEqual(T a, T b)
    {
        using D = typename std::conditional<
            sizeof(T) <= sizeof(float), float, double>::type;
        const D da = static_cast<D>(a);
        const D db = static_cast<D>(b);
        return std::abs(da - db) <= std::numeric_limits<D>::epsilon() * 16.0;
    }

    inline bool approximatelyEqual(float a, float b, float tolerance = 1.0e-5f)
    {
        return std::abs(a - b) <= tolerance;
    }

    inline bool approximatelyEqual(double a, double b, double tolerance = 1.0e-10)
    {
        return std::abs(a - b) <= tolerance;
    }

    inline std::string toStdString(const std::string& s) { return s; }

    // -------------------------------------------------------------
    // String
    // -------------------------------------------------------------

    class String
    {
    public:
        String() = default;
        String(const char* s) : value_(s ? s : "") {}
        String(const std::string& s) : value_(s) {}
        String(std::string&& s) : value_(std::move(s)) {}
        String(char c) : value_(1, c) {}
        String(int v) : value_(std::to_string(v)) {}
        String(unsigned int v) : value_(std::to_string(v)) {}
        String(long v) : value_(std::to_string(v)) {}
        String(long long v) : value_(std::to_string(v)) {}
        String(float v, int precision = 6)
        {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(precision) << v;
            value_ = oss.str();
        }
        String(double v, int precision = 6)
        {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(precision) << v;
            value_ = oss.str();
        }
        String(bool b) : value_(b ? "true" : "false") {}

        const char* c_str() const noexcept { return value_.c_str(); }
        std::string toStdString() const { return value_; }
        std::string toString() const { return value_; }

        bool isEmpty() const noexcept { return value_.empty(); }
        bool isNotEmpty() const noexcept { return !value_.empty(); }

        int length() const noexcept { return static_cast<int>(value_.size()); }
        void clear() { value_.clear(); }

        String& operator+=(const String& rhs)
        {
            value_ += rhs.value_;
            return *this;
        }

        String& operator+=(const char* rhs)
        {
            value_ += rhs ? rhs : "";
            return *this;
        }

        String& operator+=(char rhs)
        {
            value_ += rhs;
            return *this;
        }

        friend String operator+(String lhs, const String& rhs)
        {
            lhs += rhs;
            return lhs;
        }

        friend String operator+(const String& lhs, const char* rhs)
        {
            return String(lhs.value_ + (rhs ? rhs : ""));
        }

        friend String operator+(const String& lhs, char rhs)
        {
            return String(lhs.value_ + rhs);
        }

        bool equalsIgnoreCase(const String& other) const
        {
            if (value_.size() != other.value_.size())
                return false;

            return std::equal(
                value_.begin(), value_.end(),
                other.value_.begin(),
                [](char a, char b) {
                    return std::tolower(static_cast<unsigned char>(a)) ==
                           std::tolower(static_cast<unsigned char>(b));
                });
        }

        bool equals(const String& other) const noexcept { return value_ == other.value_; }

        bool startsWith(const String& prefix) const noexcept
        {
            return value_.compare(0, prefix.value_.size(), prefix.value_) == 0;
        }

        bool endsWith(const String& suffix) const noexcept
        {
            if (suffix.value_.size() > value_.size())
                return false;
            return value_.compare(value_.size() - suffix.value_.size(), suffix.value_.size(), suffix.value_) == 0;
        }

        bool contains(const String& text) const
        {
            return value_.find(text.value_) != std::string::npos;
        }

        String replace(const String& from, const String& to) const
        {
            std::string out = value_;
            std::size_t pos = 0;

            while ((pos = out.find(from.value_, pos)) != std::string::npos)
            {
                out.replace(pos, from.value_.size(), to.value_);
                pos += to.value_.size();
            }

            return String(out);
        }

        String substring(int start, int end = -1) const
        {
            const auto s = static_cast<std::size_t>(start < 0 ? 0 : start);
            const auto e = end < 0 ? value_.size() : static_cast<std::size_t>(end);

            if (s >= value_.size())
                return String();

            const auto e2 = std::min(e, value_.size());
            return String(value_.substr(s, e2 - s));
        }

        String trim() const
        {
            const auto begin = value_.find_first_not_of(" \t\r\n");
            if (begin == std::string::npos)
                return String();
            const auto end = value_.find_last_not_of(" \t\r\n");
            return String(value_.substr(begin, end - begin + 1));
        }

        String toLowerCase() const
        {
            std::string out = value_;
            std::transform(out.begin(), out.end(), out.begin(),
                           [](unsigned char c) {
                               return static_cast<char>(std::tolower(c));
                           });
            return String(out);
        }

        String toUpperCase() const
        {
            std::string out = value_;
            std::transform(out.begin(), out.end(), out.begin(),
                           [](unsigned char c) {
                               return static_cast<char>(std::toupper(c));
                           });
            return String(out);
        }

        int getIntValue() const
        {
            return std::stoi(value_);
        }

        float getFloatValue() const
        {
            return std::stof(value_);
        }

        double getDoubleValue() const
        {
            return std::stod(value_);
        }

        bool getBoolValue() const
        {
            std::string s = toLowerCase().toStdString();
            if (s == "true" || s == "1" || s == "yes" || s == "on") return true;
            if (s == "false" || s == "0" || s == "no" || s == "off") return false;
            return false;
        }

        friend bool operator==(const String& a, const String& b) noexcept
        {
            return a.value_ == b.value_;
        }

        friend bool operator!=(const String& a, const String& b) noexcept
        {
            return a.value_ != b.value_;
        }

        friend bool operator<(const String& a, const String& b) noexcept
        {
            return a.value_ < b.value_;
        }

        explicit operator const std::string&() const noexcept { return value_; }
        explicit operator std::string() const { return value_; }

    private:
        std::string value_;
    };

    inline String operator+(const String& lhs, const String& rhs)
    {
        return String(lhs.toStdString() + rhs.toStdString());
    }

    // -------------------------------------------------------------
    // StringArray
    // -------------------------------------------------------------

    class StringArray : public std::vector<String>
    {
    public:
        StringArray() = default;
        StringArray(std::initializer_list<String> init) : std::vector<String>(init) {}

        void add(const String& s) { emplace_back(s); }
        void add(const char* s) { emplace_back(String(s)); }
        void add(const StringArray& other) { insert(end(), other.begin(), other.end()); }

        bool contains(const String& s) const
        {
            for (const auto& x : *this)
                if (x == s) return true;
            return false;
        }

        String join(const String& separator) const
        {
            std::string out;
            for (size_t i = 0; i < size(); ++i)
            {
                if (i != 0)
                    out += separator.toStdString();
                out += at(i).toStdString();
            }
            return String(out);
        }

        int size() const { return static_cast<int>(std::vector<String>::size()); }
    };

    // -------------------------------------------------------------
    // Array<T>
    // -------------------------------------------------------------

    template <typename T>
    class Array
    {
    public:
        Array() = default;
        Array(std::initializer_list<T> init) : data_(init) {}

        void add(const T& v) { data_.push_back(v); }
        void add(T&& v) { data_.push_back(std::move(v)); }

        template <typename U>
        void addArray(const Array<U>& other)
        {
            for (const auto& x : other)
                data_.push_back(static_cast<T>(x));
        }

        void clear() { data_.clear(); }
        bool isEmpty() const noexcept { return data_.empty(); }
        int size() const noexcept { return static_cast<int>(data_.size()); }
        int getNumElements() const noexcept { return static_cast<int>(data_.size()); }

        T& operator[](int index) { return data_.at(index); }
        const T& operator[](int index) const { return data_.at(index); }
        T& at(int index) { return data_.at(index); }
        const T& at(int index) const { return data_.at(index); }

        typename std::vector<T>::iterator begin() { return data_.begin(); }
        typename std::vector<T>::iterator end() { return data_.end(); }
        typename std::vector<T>::const_iterator begin() const { return data_.begin(); }
        typename std::vector<T>::const_iterator end() const { return data_.end(); }
        typename std::vector<T>::const_iterator cbegin() const { return data_.cbegin(); }
        typename std::vector<T>::const_iterator cend() const { return data_.cend(); }

        bool contains(const T& value) const
        {
            return std::find(data_.begin(), data_.end(), value) != data_.end();
        }

        int indexOf(const T& value) const
        {
            auto it = std::find(data_.begin(), data_.end(), value);
            return it == data_.end() ? -1 : static_cast<int>(std::distance(data_.begin(), it));
        }

        void remove(int index)
        {
            if (index >= 0 && index < static_cast<int>(data_.size()))
                data_.erase(data_.begin() + index);
        }

        void removeFirst()
        {
            if (!data_.empty())
                data_.erase(data_.begin());
        }

        void removeLast()
        {
            if (!data_.empty())
                data_.erase(data_.end() - 1);
        }

        void insert(int index, const T& value)
        {
            data_.insert(data_.begin() + index, value);
        }

        std::vector<T>& asStdVector() { return data_; }
        const std::vector<T>& asStdVector() const { return data_; }

    private:
        std::vector<T> data_;
    };

    // -------------------------------------------------------------
    // var and VariantConverter
    // -------------------------------------------------------------

    class var
    {
    public:
        using storage_t = std::variant<
            std::monostate,
            int,
            int64,
            double,
            float,
            bool,
            std::string,
            String
        >;

        var() = default;
        var(const var&) = default;
        var(var&&) noexcept = default;
        var& operator=(const var&) = default;
        var& operator=(var&&) noexcept = default;

        var(const std::string& s) : value_(s) {}
        var(const String& s) : value_(s.toStdString()) {}
        var(int v) : value_(v) {}
        var(double v) : value_(v) {}
        var(float v) : value_(v) {}
        var(bool v) : value_(v) {}

        bool isVoid() const noexcept { return std::holds_alternative<std::monostate>(value_); }
        bool isString() const noexcept { return std::holds_alternative<std::string>(value_) || std::holds_alternative<String>(value_); }
        bool isInt() const noexcept { return std::holds_alternative<int>(value_) || std::holds_alternative<int64>(value_); }
        bool isDouble() const noexcept { return std::holds_alternative<double>(value_) || std::holds_alternative<float>(value_); }
        bool isBool() const noexcept { return std::holds_alternative<bool>(value_); }

        int toInt() const
        {
            if (std::holds_alternative<int>(value_)) return std::get<int>(value_);
            if (std::holds_alternative<int64>(value_)) return static_cast<int>(std::get<int64>(value_));
            if (std::holds_alternative<double>(value_)) return static_cast<int>(std::get<double>(value_));
            if (std::holds_alternative<float>(value_)) return static_cast<int>(std::get<float>(value_));
            if (std::holds_alternative<bool>(value_)) return std::get<bool>(value_) ? 1 : 0;
            if (std::holds_alternative<std::string>(value_)) return std::stoi(std::get<std::string>(value_));
            if (std::holds_alternative<String>(value_)) return std::stoi(std::get<String>(value_).toStdString());
            return 0;
        }

        double toDouble() const
        {
            if (std::holds_alternative<int>(value_)) return static_cast<double>(std::get<int>(value_));
            if (std::holds_alternative<int64>(value_)) return static_cast<double>(std::get<int64>(value_));
            if (std::holds_alternative<double>(value_)) return std::get<double>(value_);
            if (std::holds_alternative<float>(value_)) return static_cast<double>(std::get<float>(value_));
            if (std::holds_alternative<bool>(value_)) return std::get<bool>(value_) ? 1.0 : 0.0;
            if (std::holds_alternative<std::string>(value_)) return std::stod(std::get<std::string>(value_));
            if (std::holds_alternative<String>(value_)) return std::stod(std::get<String>(value_).toStdString());
            return 0.0;
        }

        float toFloat() const
        {
            return static_cast<float>(toDouble());
        }

        bool toBool() const
        {
            if (std::holds_alternative<bool>(value_)) return std::get<bool>(value_);
            if (std::holds_alternative<int>(value_)) return std::get<int>(value_) != 0;
            if (std::holds_alternative<int64>(value_)) return std::get<int64>(value_) != 0;
            if (std::holds_alternative<double>(value_)) return std::get<double>(value_) != 0.0;
            if (std::holds_alternative<float>(value_)) return std::get<float>(value_) != 0.0f;
            if (std::holds_alternative<std::string>(value_)) return std::string("true") == std::get<std::string>(value_);
            if (std::holds_alternative<String>(value_)) return String("true") == std::get<String>(value_);
            return false;
        }

        String toString() const
        {
            if (std::holds_alternative<std::monostate>(value_))
                return String();
            if (std::holds_alternative<int>(value_))
                return String(std::get<int>(value_));
            if (std::holds_alternative<int64>(value_))
                return String(static_cast<int>(std::get<int64>(value_)));
            if (std::holds_alternative<double>(value_))
                return String(std::get<double>(value_), 10);
            if (std::holds_alternative<float>(value_))
                return String(std::get<float>(value_), 10);
            if (std::holds_alternative<bool>(value_))
                return String(std::get<bool>(value_));
            if (std::holds_alternative<std::string>(value_))
                return String(std::get<std::string>(value_));
            if (std::holds_alternative<String>(value_))
                return std::get<String>(value_);
            return String();
        }

        template <typename T>
        T get() const
        {
            if constexpr (std::is_same_v<T, int>)
                return static_cast<T>(toInt());
            else if constexpr (std::is_same_v<T, float>)
                return static_cast<T>(toFloat());
            else if constexpr (std::is_same_v<T, double>)
                return static_cast<T>(toDouble());
            else if constexpr (std::is_same_v<T, bool>)
                return static_cast<T>(toBool());
            else if constexpr (std::is_same_v<T, String>)
                return toString();
            else if constexpr (std::is_same_v<T, std::string>)
                return toString().toStdString();
            else
                return T{};
        }

        friend bool operator==(const var& a, const var& b)
        {
            return a.value_ == b.value_;
        }

        friend bool operator!=(const var& a, const var& b)
        {
            return a.value_ != b.value_;
        }

    private:
        storage_t value_;
    };

    template <typename T>
    struct VariantConverter
    {
        static T fromVar(const var& v) { return T(v.toString().toStdString()); }
        static var toVar(const T& v) { return var(v); }
    };

    template <>
    struct VariantConverter<String>
    {
        static String fromVar(const var& v) { return v.toString(); }
        static var toVar(const String& v) { return var(v); }
    };

    template <>
    struct VariantConverter<int>
    {
        static int fromVar(const var& v) { return v.toInt(); }
        static var toVar(const int& v) { return var(v); }
    };

    template <>
    struct VariantConverter<float>
    {
        static float fromVar(const var& v) { return v.toFloat(); }
        static var toVar(const float& v) { return var(v); }
    };

    template <>
    struct VariantConverter<double>
    {
        static double fromVar(const var& v) { return v.toDouble(); }
        static var toVar(const double& v) { return var(v); }
    };

    template <>
    struct VariantConverter<bool>
    {
        static bool fromVar(const var& v) { return v.toBool(); }
        static var toVar(const bool& v) { return var(v); }
    };

    // -------------------------------------------------------------
    // ValueTree
    // -------------------------------------------------------------

    class ValueTree
    {
    public:
        ValueTree() = default;
        explicit ValueTree(const String& type) : type_(type) {}
        ValueTree(const char* type) : type_(type) {}

        bool isValid() const noexcept { return !type_.isEmpty(); }
        bool hasType(const String& t) const noexcept { return type_ == t; }
        const String& getType() const noexcept { return type_; }

        void appendChild(const ValueTree& child, void* /*insertBefore*/ = nullptr)
        {
            children_.push_back(child);
        }

        ValueTree getChildWithName(const String& name) const
        {
            for (const auto& c : children_)
            {
                if (c.getType() == name)
                    return c;
            }
            return ValueTree();
        }

        ValueTree getChild(int index) const
        {
            if (index < 0 || index >= static_cast<int>(children_.size()))
                return ValueTree();
            return children_[static_cast<std::size_t>(index)];
        }

        int getNumChildren() const noexcept { return static_cast<int>(children_.size()); }

        void setProperty(const String& key, const var& value)
        {
            properties_[key.toStdString()] = value;
        }

        var getProperty(const String& key) const
        {
            auto it = properties_.find(key.toStdString());
            if (it == properties_.end())
                return var();
            return it->second;
        }

        bool hasProperty(const String& key) const
        {
            return properties_.find(key.toStdString()) != properties_.end();
        }

        void copyPropertiesFrom(const ValueTree& other, bool /*preserveExisting*/ = false)
        {
            properties_ = other.properties_;
        }

    private:
        String type_;
        std::vector<ValueTree> children_;
        std::unordered_map<std::string, var> properties_;
    };

    // -------------------------------------------------------------
    // AudioBuffer<T>
    // -------------------------------------------------------------

    template <typename T>
    class AudioBuffer
    {
    public:
        AudioBuffer() = default;
        AudioBuffer(int numChannels, int numSamples)
        {
            setSize(numChannels, numSamples, false);
        }

        void setSize(int numChannels, int numSamples, bool /*keepExisting*/ = true)
        {
            channels_.resize(numChannels);
            for (auto& ch : channels_)
                ch.assign(numSamples, T{});
            numSamples_ = numSamples;
        }

        void clear()
        {
            for (auto& ch : channels_)
                std::fill(ch.begin(), ch.end(), T{});
        }

        int getNumChannels() const noexcept { return static_cast<int>(channels_.size()); }
        int getNumSamples() const noexcept { return numSamples_; }

        T* getWritePointer(int channel, int startSample = 0)
        {
            return channels_.at(channel).data() + startSample;
        }

        const T* getReadPointer(int channel, int startSample = 0) const
        {
            return channels_.at(channel).data() + startSample;
        }

        void makeCopyOf(const AudioBuffer& other)
        {
            channels_ = other.channels_;
            numSamples_ = other.numSamples_;
        }

        void copyFrom(int destChannel, int destStartSample, const T* src, int numSamples)
        {
            if (destChannel < 0 || destChannel >= static_cast<int>(channels_.size()))
                return;
            auto& ch = channels_.at(destChannel);
            std::copy(src, src + numSamples, ch.begin() + destStartSample);
        }

        void addFrom(int destChannel, int destStartSample, const T* src, int numSamples)
        {
            if (destChannel < 0 || destChannel >= static_cast<int>(channels_.size()))
                return;

            auto& ch = channels_.at(destChannel);
            for (int i = 0; i < numSamples; ++i)
                ch[destStartSample + i] += src[i];
        }

        void setValue(int channel, int sampleIndex, T value)
        {
            channels_.at(channel).at(sampleIndex) = value;
        }

        T getValue(int channel, int sampleIndex) const
        {
            return channels_.at(channel).at(sampleIndex);
        }

        void setData(int channel, const std::vector<T>& data)
        {
            channels_.at(channel) = data;
        }

        std::vector<T>& getChannel(int channel)
        {
            return channels_.at(channel);
        }

        const std::vector<T>& getChannel(int channel) const
        {
            return channels_.at(channel);
        }

    private:
        std::vector<std::vector<T>> channels_;
        int numSamples_ = 0;
    };

    // -------------------------------------------------------------
    // MidiMessage / MidiBuffer
    // -------------------------------------------------------------

    class MidiMessage
    {
    public:
        MidiMessage() = default;
        MidiMessage(std::initializer_list<uint8_t> data) : data_(data) {}
        explicit MidiMessage(const std::vector<uint8_t>& data) : data_(data) {}

        std::vector<uint8_t> getData() const { return data_; }
        size_t getDataSize() const { return data_.size(); }

        bool isNoteOn(int channel = 0) const
        {
            return data_.size() >= 3 && (data_[0] & 0xF0) == 0x90 && ((data_[0] & 0x0F) == channel);
        }

        bool isNoteOff(int channel = 0) const
        {
            return data_.size() >= 3 && (data_[0] & 0xF0) == 0x80 && ((data_[0] & 0x0F) == channel);
        }

        bool isController(int channel = 0) const
        {
            return data_.size() >= 3 && (data_[0] & 0xF0) == 0xB0 && ((data_[0] & 0x0F) == channel);
        }

    private:
        std::vector<uint8_t> data_;
    };

    class MidiBuffer
    {
    public:
        struct Event
        {
            int samplePosition = 0;
            MidiMessage message;
        };

        void clear() { events_.clear(); }
        void addEvent(const MidiMessage& message, int samplePosition)
        {
            events_.push_back({samplePosition, message});
        }

        int getNumEvents() const noexcept { return static_cast<int>(events_.size()); }

        template <typename Callback>
        void forEach(const Callback& cb) const
        {
            for (const auto& e : events_)
                cb(e.message, e.samplePosition);
        }

        const std::vector<Event>& events() const noexcept { return events_; }

    private:
        std::vector<Event> events_;
    };

    // -------------------------------------------------------------
    // SmoothedValue
    // -------------------------------------------------------------

    namespace ValueSmoothingTypes
    {
        enum
        {
            Linear = 0,
            Multiplicative = 1
        };
    }

    template <typename ValueType, int SmoothingType = ValueSmoothingTypes::Linear>
    class SmoothedValue
    {
    public:
        SmoothedValue() = default;
        SmoothedValue(ValueType initialValue)
            : current_(initialValue), target_(initialValue) {}

        void setCurrentAndTargetValue(ValueType value)
        {
            current_ = value;
            target_ = value;
        }

        void setTarget(ValueType value)
        {
            target_ = value;
        }

        void setValue(ValueType value)
        {
            current_ = value;
        }

        void reset(ValueType value = ValueType{})
        {
            current_ = value;
            target_ = value;
        }

        void process()
        {
            current_ = target_;
        }

        ValueType getCurrentValue() const noexcept { return current_; }
        ValueType getTargetValue() const noexcept { return target_; }
        ValueType getNextValue() const noexcept { return target_; }

        operator ValueType() const noexcept { return current_; }

    private:
        ValueType current_ = ValueType{};
        ValueType target_ = ValueType{};
    };

    // -------------------------------------------------------------
    // MemoryBlock / streams
    // -------------------------------------------------------------

    class MemoryBlock
    {
    public:
        MemoryBlock() = default;

        MemoryBlock(const void* data, size_t size)
        {
            if (data && size > 0)
            {
                const auto* p = static_cast<const uint8_t*>(data);
                bytes_.assign(p, p + size);
            }
        }

        void append(const void* data, size_t size)
        {
            if (!data || size == 0)
                return;

            const auto* p = static_cast<const uint8_t*>(data);
            bytes_.insert(bytes_.end(), p, p + size);
        }

        void clear() { bytes_.clear(); }
        const void* getData() const noexcept { return bytes_.empty() ? nullptr : bytes_.data(); }
        size_t getSize() const noexcept { return bytes_.size(); }

        std::vector<uint8_t>& data() { return bytes_; }
        const std::vector<uint8_t>& data() const { return bytes_; }

    private:
        std::vector<uint8_t> bytes_;
    };

    class MemoryInputStream
    {
    public:
        MemoryInputStream(const void* data, size_t size, bool /*keepData*/ = false)
            : data_(reinterpret_cast<const uint8_t*>(data),
                    reinterpret_cast<const uint8_t*>(data) + size)
        {
        }

        bool isOpen() const noexcept { return !data_.empty(); }
        const uint8_t* getData() const noexcept { return data_.data(); }
        size_t getDataSize() const noexcept { return data_.size(); }

        std::string readAllAsString() const
        {
            return std::string(data_.begin(), data_.end());
        }

    private:
        std::vector<uint8_t> data_;
    };

    class MemoryOutputStream
    {
    public:
        void write(const void* data, size_t size)
        {
            if (!data || size == 0)
                return;

            const auto* p = static_cast<const uint8_t*>(data);
            output_.insert(output_.end(), p, p + size);
        }

        void writeText(const String& text)
        {
            write(text.c_str(), text.size());
        }

        std::vector<uint8_t>& data() { return output_; }
        const std::vector<uint8_t>& data() const { return output_; }

    private:
        std::vector<uint8_t> output_;
    };

    // -------------------------------------------------------------
    // File
    // -------------------------------------------------------------

    class File
    {
    public:
        File() = default;
        explicit File(const std::string& path) : path_(path) {}
        explicit File(const char* path) : path_(path ? path : "") {}
        explicit File(const String& path) : path_(path.toStdString()) {}

        bool exists() const
        {
            return std::filesystem::exists(path_);
        }

        bool isDirectory() const
        {
            return std::filesystem::is_directory(path_);
        }

        bool createDirectories() const
        {
            std::error_code ec;
            std::filesystem::create_directories(path_, ec);
            return !ec;
        }

        bool deleteFile() const
        {
            std::error_code ec;
            std::filesystem::remove(path_, ec);
            return !ec;
        }

        bool replaceWithText(const String& text) const
        {
            std::ofstream os(path_, std::ios::binary | std::ios::trunc);
            if (!os)
                return false;
            os << text.toStdString();
            return true;
        }

        std::string loadFileAsString() const
        {
            std::ifstream in(path_, std::ios::binary);
            if (!in)
                return "";
            std::ostringstream oss;
            oss << in.rdbuf();
            return oss.str();
        }

        std::vector<uint8_t> loadFileAsData() const
        {
            std::ifstream in(path_, std::ios::binary);
            if (!in)
                return {};
            return std::vector<uint8_t>(
                std::istreambuf_iterator<char>(in),
                std::istreambuf_iterator<char>());
        }

        String getFullPathName() const { return String(path_.string()); }
        String getFileName() const { return String(path_.filename().string()); }
        File getParentDirectory() const { return File(path_.parent_path().string()); }

        bool operator==(const File& other) const { return path_ == other.path_; }
        bool operator!=(const File& other) const { return path_ != other.path_; }

        std::filesystem::path path() const { return path_; }

    private:
        std::filesystem::path path_;
    };

    // -------------------------------------------------------------
    // Timing / thread
    // -------------------------------------------------------------

    class Time
    {
    public:
        Time() : seconds_(0.0) {}
        explicit Time(double seconds) : seconds_(seconds) {}

        double inSeconds() const noexcept { return seconds_; }
        double inMilliseconds() const noexcept { return seconds_ * 1000.0; }

        static Time getCurrentTime() { return Time(0.0); }

    private:
        double seconds_;
    };

    class Thread
    {
    public:
        virtual ~Thread() = default;
        virtual void run() {}
        bool startThread() { return true; }
        bool stopThread() { return false; }
        bool isRunning() const noexcept { return false; }
    };

    class ThreadPool
    {
    public:
        ThreadPool() = default;
        void addJob(const std::function<void()>& job) { jobs_.push_back(job); }
        void addTask(const std::function<void()>& job) { jobs_.push_back(job); }
        void waitForAllJobs() { for (auto& j : jobs_) if (j) j(); }
        size_t size() const { return jobs_.size(); }

    private:
        std::vector<std::function<void()>> jobs_;
    };

    // -------------------------------------------------------------
    // Random
    // -------------------------------------------------------------

    class Random
    {
    public:
        Random() : gen_(std::random_device{}()) {}
        explicit Random(uint32_t seed) : gen_(seed) {}

        int nextInt() { return static_cast<int>(gen_() % 1000000); }
        float nextFloat()
        {
            return static_cast<float>(
                gen_() / static_cast<double>(std::mt19937::max()));
        }

    private:
        std::mt19937 gen_;
    };

    // -------------------------------------------------------------
    // RSAKey
    // -------------------------------------------------------------

    struct RSAKey
    {
        std::string modulus;
        std::string exponent;
        std::string privateExponent;

        bool isValid() const { return !modulus.empty() && !exponent.empty(); }
    };

    // -------------------------------------------------------------
    // No-op support
    // -------------------------------------------------------------

    class ScopedNoDenormals
    {
    public:
        ScopedNoDenormals() = default;
    };

    class DynamicObject
    {
    public:
        void setProperty(const String&, const var&) {}
        var getProperty(const String&) const { return var(); }
    };

    class PropertySet
    {
    public:
        void setValue(const String&, const var&) {}
        var getValue(const String&) const { return var(); }
    };

    // -------------------------------------------------------------
    // Some generic audio format stubs
    // -------------------------------------------------------------

    class AudioFormat
    {
    public:
        virtual ~AudioFormat() = default;
        virtual String getFormatName() const { return String("generic"); }
        virtual bool isCompressed() const { return false; }
        virtual bool hasHeaderData() const { return false; }
        virtual bool canDoStereo() const { return true; }
    };

    class AudioFormatReader : public AudioFormat
    {
    public:
        int numChannels = 1;
        int lengthInSamples = 0;
        virtual ~AudioFormatReader() = default;
    };

    class AudioFormatWriter : public AudioFormat
    {
    public:
        virtual ~AudioFormatWriter() = default;
    };

    class AudioFormatManager
    {
    public:
        void registerBasicFormats() {}
        AudioFormat* getFormat(const String&) { return nullptr; }
    };

    // -------------------------------------------------------------
    // Helper aliases
    // -------------------------------------------------------------

    using Identifier = String;

    template <typename T>
    inline String toString(const T& value)
    {
        return String(std::to_string(value));
    }

    template <>
    inline String toString(const String& value)
    {
        return value;
    }

    template <>
    inline String toString(const bool& value)
    {
        return String(value);
    }

    template <>
    inline String toString(const float& value)
    {
        return String(value, 7);
    }

    template <>
    inline String toString(const double& value)
    {
        return String(value, 12);
    }

    inline void ignoreUnused(int) {}
    inline void ignoreUnused(const void*) {}
    inline void ignoreUnused(const String&) {}

    inline String getCurrentWorkingDirectory()
    {
        return String(std::filesystem::current_path().string());
    }

    template <typename Key, typename Value>
    using HashMap = std::unordered_map<Key, Value>;

    template <typename T>
    using SparseSet = std::set<T>;

} // namespace juce
