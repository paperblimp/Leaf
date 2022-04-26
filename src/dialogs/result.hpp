#pragma once

// builtin
#include <optional>



template <typename>
class DialogResult;

class EmptyDialogResult
{
    
    private:

        bool _runned;
        bool _ended;

    public:

        EmptyDialogResult(bool runned, bool ended): _runned(runned), _ended(ended) {};

        static EmptyDialogResult make_closed()
        {
            return EmptyDialogResult{false, false};
        }

        static EmptyDialogResult make_runned()
        {
            return EmptyDialogResult{true, false};
        }

        static EmptyDialogResult make_ended()
        {
            return EmptyDialogResult{true, true};
        }

        template <typename T>
        operator DialogResult<T>();

        bool runned()
        {
            return this->_runned;
        }

        bool ended()
        {
            return this->_ended;
        }
};

template <typename T>
class DialogResult: public EmptyDialogResult
{
    private:

        std::optional<T> _value;

    public:

        DialogResult(bool runned, bool ended, std::optional<T> value): EmptyDialogResult{runned, ended}, _value{value} {}

        bool has_value()
        {
            return this->_value.has_value();
        }

        T value()
        {
            leaf_assert(this->_value.has_value());
            return *this->_value;
        }
};

template <>
class DialogResult<void>: public EmptyDialogResult {};


template <typename T>
EmptyDialogResult::operator DialogResult<T>()
{
    return {this->_runned, this->_ended, std::nullopt};
}

template <>
inline EmptyDialogResult::operator DialogResult<void>()
{
    return {*this};
}
