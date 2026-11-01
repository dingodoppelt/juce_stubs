#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <initializer_list>
#include <limits>
#include <list>
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

#ifndef JUCE_DECLARE_NON_COPYABLE
#define JUCE_DECLARE_NON_COPYABLE(Class) \
    Class(const Class&) = delete; \
    Class& operator=(const Class&) = delete;
#endif

#ifndef JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR
#define JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Class) \
    JUCE_DECLARE_NON_COPYABLE(Class)
#endif

namespace juce
{
    using int64 = std::int64_t;
    using uint64 = std::uint64_t;
    using uint = unsigned int;
    using uchar = unsigned char;
    using juce_wchar = wchar_t;

    namespace MathConstants
    {
        template <typename T>
        constexpr T pi = static_cast<T>(3.1415926535897932384626433832795);

        template <typename T>
        constexpr T twoPi = static_cast<T>(6.283185307179586476925286766559);

        template <typename T>
        constexpr T halfPi = static_cast<T>(1.5707963267948966192313216916398);

        template <typename T>
        constexpr T epsilon = static_cast<T>(1e-7);
    }

    namespace ValueSmoothingTypes
    {
        enum Type
        {
            Linear,
            Multiplicative
        };
    }

    template <typename T>
    class Array
    {
    public:
        Array() = default;
        Array(std::initializer_list<T> init) : data_(init) {}

        void add(const T& value) { data_.push_back(value); }
        void add(T&& value) { data_.push_back(std::move(value)); }
        void clear() { data_.clear(); }
        bool isEmpty() const noexcept { return data_.empty(); }
        int size() const noexcept { return static_cast<int>(data_.size()); }

        T& operator[](int index) { return data_.at(index); }
        const T& operator[](int index) const { return data_.at(index); }

        void ensureStorageAllocated(int newSize)
        {
            if (newSize > static_cast<int>(data_.size()))
                data_.resize(static_cast<size_t>(newSize));
        }

        T* begin() noexcept { return data_.empty() ? nullptr : data_.data(); }
        T* end() noexcept { return data_.empty() ? nullptr : data_.data() + data_.size(); }
        const T* begin() const noexcept { return data_.empty() ? nullptr : data_.data(); }
        const T* end() const noexcept { return data_.empty() ? nullptr : data_.data() + data_.size(); }

    private:
        std::vector<T> data_;
    };

    template <typename T>
    class AudioBuffer
    {
    public:
        AudioBuffer() = default;
        AudioBuffer(int numChannels, int numSamples)
        {
            setSize(numChannels, numSamples, true);
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

        void copyFrom(int destChannel, int destStartSample, const T* src, int numSamples)
        {
            auto& ch = channels_.at(destChannel);
            std::copy(src, src + numSamples, ch.begin() + destStartSample);
        }

        void addFrom(int destChannel, int destStartSample, const T* src, int numSamples)
        {
            auto& ch = channels_.at(destChannel);
            for (int i = 0; i < numSamples; ++i)
                ch[destStartSample + i] += src[i];
        }

        T& getReference(int channel, int index) { return channels_.at(channel).at(index); }
        const T& getReference(int channel, int index) const { return channels_.at(channel).at(index); }

    private:
        std::vector<std::vector<T>> channels_;
        int numSamples_ = 0;
    };

    template <typename T>
    class Range
    {
    public:
        Range() : start_(T{}), end_(T{}) {}
        Range(T start, T end) : start_(start), end_(end) {}

        T getStart() const noexcept { return start_; }
        T getEnd() const noexcept { return end_; }
        T getLength() const noexcept { return end_ - start_; }

    private:
        T start_, end_;
    };

    template <typename T, int SmoothingType = ValueSmoothingTypes::Linear>
    class SmoothedValue
    {
    public:
        SmoothedValue() = default;
        SmoothedValue(T initialValue) : current_(initialValue), target_(initialValue) {}

        void setCurrentAndTargetValue(T value) { current_ = value; target_ = value; }
        void setTarget(T value) { target_ = value; }
        void process() { current_ = target_; }

        T getCurrentValue() const noexcept { return current_; }
        T getTargetValue() const noexcept { return target_; }

    private:
        T current_ = T{};
        T target_ = T{};
    };

    class String
    {
    public:
        String() = default;
        String(const char* s) : value_(s ? s : "") {}
        String(const std::string& s) : value_(s) {}
        String(char c) : value_(1, c) {}
        String(int v) : value_(std::to_string(v)) {}
        String(int64 v) : value_(std::to_string(v)) {}
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
        std::size_t size() const noexcept { return value_.size(); }
        bool isEmpty() const noexcept { return value_.empty(); }

        String& operator+=(const String& rhs)
        {
            value_ += rhs.value_;
            return *this;
        }

        friend bool operator==(const String& a, const String& b) noexcept
        {
            return a.value_ == b.value_;
        }

        friend bool operator!=(const String& a, const String& b) noexcept
        {
            return a.value_ != b.value_;
        }

    private:
        std::string value_;
    };

    class Random
    {
    public:
        Random() : engine_(std::random_device{}()) {}
        explicit Random(uint32_t seed) : engine_(seed) {}

        static Random getSystemRandom() { return Random(); }

        int nextInt() { return static_cast<int>(engine_() % 1000000); }
        int64 nextInt64() { return static_cast<int64>(engine_()); }
        float nextFloat() { return static_cast<float>(engine_() / static_cast<double>(std::mt19937::max())); }

    private:
        std::mt19937 engine_;
    };

    class AbstractFifo
    {
    public:
        AbstractFifo() = default;
        void clear() {}
        int getNumReady() const noexcept { return 0; }
    };

    class File
    {
    public:
        File() = default;
        explicit File(const std::string& p) : path_(p) {}
        explicit File(const char* p) : path_(p ? p : "") {}

        bool exists() const { return std::filesystem::exists(path_); }
        bool isDirectory() const { return std::filesystem::is_directory(path_); }
        bool isFile() const { return std::filesystem::is_regular_file(path_); }
        std::string getFullPathName() const { return path_.string(); }

    private:
        std::filesystem::path path_;
    };

    class var
    {
    public:
        using storage_t = std::variant<std::monostate, int, int64, double, float, bool, std::string, String>;

        var() = default;
        var(const std::string& s) : value_(s) {}
        var(const String& s) : value_(s.toStdString()) {}
        var(int v) : value_(v) {}
        var(int64 v) : value_(v) {}
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
            return 0;
        }

        double toDouble() const
        {
            if (std::holds_alternative<int>(value_)) return static_cast<double>(std::get<int>(value_));
            if (std::holds_alternative<int64>(value_)) return static_cast<double>(std::get<int64>(value_));
            if (std::holds_alternative<double>(value_)) return std::get<double>(value_);
            if (std::holds_alternative<float>(value_)) return static_cast<double>(std::get<float>(value_));
            if (std::holds_alternative<bool>(value_)) return std::get<bool>(value_) ? 1.0 : 0.0;
            return 0.0;
        }

        String toString() const
        {
            if (std::holds_alternative<std::string>(value_)) return String(std::get<std::string>(value_));
            if (std::holds_alternative<String>(value_)) return std::get<String>(value_);
            if (std::holds_alternative<int>(value_)) return String(std::get<int>(value_));
            if (std::holds_alternative<double>(value_)) return String(std::get<double>(value_), 10);
            if (std::holds_alternative<float>(value_)) return String(std::get<float>(value_), 10);
            if (std::holds_alternative<bool>(value_)) return String(std::get<bool>(value_));
            return String();
        }

    private:
        storage_t value_;
    };

    template <typename T>
    class ConstructedArray
    {
    public:
        ConstructedArray() = default;

        void resize(int newSize)
        {
            data_.resize(static_cast<size_t>(newSize));
        }

        T* operator[](int index) { return &data_.at(index); }
        const T* operator[](int index) const { return &data_.at(index); }

        int size() const noexcept { return static_cast<int>(data_.size()); }
        bool isEmpty() const noexcept { return data_.empty(); }

    private:
        std::vector<T> data_;
    };

    class ReferenceCountedObject
    {
    public:
        ReferenceCountedObject() = default;
        virtual ~ReferenceCountedObject() = default;

        void incReference() noexcept { ++refCount_; }
        void decReference() noexcept
        {
            if (--refCount_ == 0)
                delete this;
        }

        int getReferenceCount() const noexcept { return refCount_; }

    private:
        std::atomic<int> refCount_ { 1 };
    };

    template <typename T>
    class SampleStream
    {
    public:
        SampleStream() = default;
        explicit SampleStream(std::function<T()> func) : func_(std::move(func)) {}

        T getSample() const
        {
            return func_ ? func_() : T{};
        }

        void setFunction(std::function<T()> func)
        {
            func_ = std::move(func);
        }

    private:
        std::function<T()> func_;
    };
}

namespace lemons
{
    namespace dsp
    {
        template <typename T>
        using Array = juce::Array<T>;

        template <typename T>
        using AudioBuffer = juce::AudioBuffer<T>;

        template <typename T>
        using SampleStream = juce::SampleStream<T>;

        template <typename T>
        using ConstructedArray = juce::ConstructedArray<T>;

        template <typename T>
        using ValueSmoother = juce::SmoothedValue<T, juce::ValueSmoothingTypes::Multiplicative>;

        template <typename T>
        using Range = juce::Range<T>;
    }
}
