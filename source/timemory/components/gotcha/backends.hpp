// MIT License
//
// Copyright (c) 2020, The Regents of the University of California,
// through Lawrence Berkeley National Laboratory (subject to receipt of any
// required approvals from the U.S. Dept. of Energy).  All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

/**
 * \file timemory/components/gotcha/backends.hpp
 * \brief Implementation of the gotcha functions/utilities
 */

#pragma once

#include "timemory/backends/gotcha.hpp"
#include "timemory/components/base.hpp"
#include "timemory/components/types.hpp"
#include "timemory/mpl/apply.hpp"
#include "timemory/mpl/filters.hpp"
#include "timemory/settings/declaration.hpp"
#include "timemory/units.hpp"
#include "timemory/utility/mangler.hpp"
#include "timemory/variadic/types.hpp"

#include <cassert>
#include <cstdint>
#include <string>
#include <tuple>

//======================================================================================//
//
namespace tim
{
namespace component
{
//
//======================================================================================//
//
class gotcha_suppression
{
private:
    template <size_t Nt, typename Components, typename Differentiator>
    friend struct gotcha;

    template <typename Tp, typename Ret>
    struct gotcha_invoker;

    template <typename Tp>
    friend struct operation::init_storage;

    template <size_t, typename Tp>
    friend struct user_bundle;

    friend struct opaque;

    static bool& get()
    {
        static thread_local bool _instance = false;
        return _instance;
    }

public:
    struct auto_toggle
    {
        explicit auto_toggle(bool& _value, bool _if_equal = false)
        : m_value(_value)
        , m_if_equal(_if_equal)
        {
            if(m_value == m_if_equal)
            {
                m_value      = !m_value;
                m_did_toggle = true;
            }
        }

        ~auto_toggle()
        {
            if(m_value != m_if_equal && m_did_toggle)
            {
                m_value = !m_value;
            }
        }

        auto_toggle(const auto_toggle&) = delete;
        auto_toggle(auto_toggle&&)      = delete;
        auto_toggle& operator=(const auto_toggle&) = delete;
        auto_toggle& operator=(auto_toggle&&) = delete;

    private:
        bool& m_value;
        bool  m_if_equal;
        bool  m_did_toggle = false;
    };
};
//
//======================================================================================//
///
/// \struct component::gotcha_invoker
///
///
template <typename Tp, typename Ret>
struct gotcha_invoker
{
    using Type       = Tp;
    using value_type = typename Type::value_type;
    using base_type  = typename Type::base_type;

    template <typename FuncT, typename... Args>
    static decltype(auto) invoke(Tp& _obj, bool& _ready, FuncT&& _func, Args&&... _args)
    {
        return invoke_sfinae(_obj, _ready, std::forward<FuncT>(_func),
                             std::forward<Args>(_args)...);
    }

private:
    //----------------------------------------------------------------------------------//
    //  Call:
    //
    //      Ret Type::operator{}(Args...)
    //
    //  instead of gotcha_wrappee
    //
    template <typename FuncT, typename... Args>
    static auto invoke_sfinae_impl(Tp& _obj, int, bool&, FuncT&&, Args&&... _args)
        -> decltype(_obj(std::forward<Args>(_args)...))
    {
        return _obj(std::forward<Args>(_args)...);
    }

    //----------------------------------------------------------------------------------//
    //  Call the original gotcha_wrappee
    //
    template <typename FuncT, typename... Args>
    static auto invoke_sfinae_impl(Tp&, long, bool&, FuncT&& _func, Args&&... _args)
        -> decltype(std::forward<FuncT>(_func)(std::forward<Args>(_args)...))
    {
        return std::forward<FuncT>(_func)(std::forward<Args>(_args)...);
    }

    //----------------------------------------------------------------------------------//
    //  Wrapper that calls one of two above
    //
    template <typename FuncT, typename... Args>
    static auto invoke_sfinae(Tp& _obj, bool& _ready, FuncT&& _func, Args&&... _args)
        -> decltype(invoke_sfinae_impl(_obj, 0, _ready, std::forward<FuncT>(_func),
                                       std::forward<Args>(_args)...))
    {
        return invoke_sfinae_impl(_obj, 0, _ready, std::forward<FuncT>(_func),
                                  std::forward<Args>(_args)...);
    }

public:
    //==================================================================================//
    //
    template <typename FuncT, typename... Args>
    static decltype(auto) invoke(Tp& _obj, FuncT&& _func, Args&&... _args)
    {
        return invoke_sfinae(_obj, std::forward<FuncT>(_func),
                             std::forward<Args>(_args)...);
    }

private:
    //----------------------------------------------------------------------------------//
    //  Call the operator of the instance
    //
    template <typename FuncT, typename... Args>
    static auto invoke_sfinae_impl(Tp& _obj, int, FuncT&&, Args&&... _args)
        -> decltype(_obj(std::forward<Args>(_args)...))
    {
        return _obj(std::forward<Args>(_args)...);
    }

    //----------------------------------------------------------------------------------//
    //  Call the original gotcha_wrappee
    //
    template <typename FuncT, typename... Args>
    static auto invoke_sfinae_impl(Tp&, long, FuncT&& _func, Args&&... _args)
        -> decltype(std::forward<FuncT>(_func)(std::forward<Args>(_args)...))
    {
        return std::forward<FuncT>(_func)(std::forward<Args>(_args)...);
    }

    //----------------------------------------------------------------------------------//
    //  Wrapper that calls one of two above
    //
    template <typename FuncT, typename... Args>
    static auto invoke_sfinae(Tp& _obj, FuncT&& _func, Args&&... _args)
        -> decltype(invoke_sfinae_impl(_obj, 0, _func, std::forward<Args>(_args)...))
    {
        return invoke_sfinae_impl(_obj, 0, std::forward<FuncT>(_func),
                                  std::forward<Args>(_args)...);
    }
    //
    //----------------------------------------------------------------------------------//
};
//
}  // namespace component
}  // namespace tim
//
//======================================================================================//
