//  MIT License
//
//  Copyright (c) 2020, The Regents of the University of California,
//  through Lawrence Berkeley National Laboratory (subject to receipt of any
//  required approvals from the U.S. Dept. of Energy).  All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "timemory/components/base/declaration.hpp"
#include "timemory/components/base/templates.hpp"
#include "timemory/components/base/types.hpp"
#include "timemory/components/metadata.hpp"
#include "timemory/mpl/math.hpp"
#include "timemory/mpl/types.hpp"
#include "timemory/operations/types/record.hpp"
#include "timemory/storage/types.hpp"
#include "timemory/tpls/cereal/cereal.hpp"
#include "timemory/units.hpp"

//======================================================================================//
//
namespace tim
{
namespace component
{
//
//======================================================================================//
//
//                              NON-VOID BASE
//
//======================================================================================//
//
template <typename Tp, typename Value>
void
base<Tp, Value>::reset()
{
    is_running   = false;
    is_on_stack  = false;
    is_transient = false;
    is_flat      = false;
    depth_change = false;
    laps         = 0;
    value        = value_type{};
    accum        = accum_type{};
    last         = last_type{};
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
void
base<Tp, Value>::measure()
{
    is_transient                = false;
    Type*                   obj = static_cast<Type*>(this);
    operation::record<Type> m{ *obj };
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
void
base<Tp, Value>::get(void*& ptr, size_t _typeid_hash) const
{
    if(!ptr && _typeid_hash == typeid_hash<Tp>())
        ptr = reinterpret_cast<void*>(const_cast<base_type*>(this));
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
void
base<Tp, Value>::get_opaque_data(void*& ptr, size_t _typeid_hash) const
{
    if(!ptr && _typeid_hash == typeid_hash<Tp>())
    {
        auto _data      = static_cast<const Tp*>(this)->get();
        using data_type = decay_t<decltype(_data)>;
        auto _pdata     = new data_type(_data);
        ptr             = reinterpret_cast<void*>(_pdata);
    }
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
void
base<Tp, Value>::set_started()
{
    is_running = true;
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
void
base<Tp, Value>::set_stopped()
{
    if(is_running)
    {
        ++laps;
        is_transient = true;
        is_running   = false;
    }
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
tim::base::storage*
base<Tp, Value>::get_storage()
{
    return tim::base::storage::template base_instance<Tp, Value>();
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
short
base<Tp, Value>::get_width()
{
    static short _instance = Type::width;
    if(settings::width() >= 0)
        _instance = settings::width();

    if(timing_category_v && settings::timing_width() >= 0)
        _instance = settings::timing_width();
    else if(memory_category_v && settings::memory_width() >= 0)
        _instance = settings::memory_width();

    return _instance;
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
short
base<Tp, Value>::get_precision()
{
    static short _instance = Type::precision;
    if(settings::precision() >= 0)
        _instance = settings::precision();

    if(timing_category_v && settings::timing_precision() >= 0)
        _instance = settings::timing_precision();
    else if(memory_category_v && settings::memory_precision() >= 0)
        _instance = settings::memory_precision();

    return _instance;
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
std::ios_base::fmtflags
base<Tp, Value>::get_format_flags()
{
    static std::ios_base::fmtflags _instance = Type::format_flags;

    auto _set_scientific = []() {
        _instance &= (std::ios_base::fixed & std::ios_base::scientific);
        _instance |= (std::ios_base::scientific);
    };

    if(!percent_units_v &&
       (settings::scientific() || (timing_category_v && settings::timing_scientific()) ||
        (memory_category_v && settings::memory_scientific())))
        _set_scientific();

    return _instance;
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
std::string
base<Tp, Value>::label()
{
    return metadata<Tp>::label();
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
std::string
base<Tp, Value>::description()
{
    return metadata<Tp>::description();
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
std::string
base<Tp, Value>::get_label()
{
    static std::string _instance = Type::label();
    return _instance;
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
std::string
base<Tp, Value>::get_description()
{
    static std::string _instance = Type::description();
    return _instance;
}
//
//--------------------------------------------------------------------------------------//
//
//              operator + - * /    (base_type)
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
Tp&
base<Tp, Value>::plus_oper(const base_type& rhs)
{
    return operator+=(static_cast<const Type&>(rhs));
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
Tp&
base<Tp, Value>::minus_oper(const base_type& rhs)
{
    return operator-=(static_cast<const Type&>(rhs));
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
Tp&
base<Tp, Value>::multiply_oper(const base_type& rhs)
{
    return operator*=(static_cast<const Type&>(rhs));
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
Tp&
base<Tp, Value>::divide_oper(const base_type& rhs)
{
    return operator/=(static_cast<const Type&>(rhs));
}
//
//--------------------------------------------------------------------------------------//
//
//              operator + - * /    (Type)
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
Tp&
base<Tp, Value>::plus_oper(const Tp& rhs)
{
    value = value_compute_type::plus(value, rhs.value);
    accum = accum_compute_type::plus(accum, rhs.accum);
    return static_cast<Type&>(*this);
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
Tp&
base<Tp, Value>::minus_oper(const Tp& rhs)
{
    value = value_compute_type::minus(value, rhs.value);
    accum = accum_compute_type::minus(accum, rhs.accum);
    return static_cast<Type&>(*this);
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
Tp&
base<Tp, Value>::multiply_oper(const Tp& rhs)
{
    value = value_compute_type::multiply(value, rhs.value);
    accum = accum_compute_type::multiply(accum, rhs.accum);
    return static_cast<Type&>(*this);
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
Tp&
base<Tp, Value>::divide_oper(const Tp& rhs)
{
    value = value_compute_type::divide(value, rhs.value);
    accum = accum_compute_type::divide(accum, rhs.accum);
    return static_cast<Type&>(*this);
}
//
//--------------------------------------------------------------------------------------//
//
//              operator + - * /    (Value)
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
Tp&
base<Tp, Value>::plus_oper(const Value& rhs)
{
    value = value_compute_type::plus(value, rhs);
    accum = accum_compute_type::plus(accum, rhs);
    return static_cast<Type&>(*this);
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
Tp&
base<Tp, Value>::minus_oper(const Value& rhs)
{
    value = value_compute_type::minus(value, rhs);
    accum = accum_compute_type::minus(accum, rhs);
    return static_cast<Type&>(*this);
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
Tp&
base<Tp, Value>::multiply_oper(const Value& rhs)
{
    value = value_compute_type::multiply(value, rhs);
    accum = accum_compute_type::multiply(accum, rhs);
    return static_cast<Type&>(*this);
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
Tp&
base<Tp, Value>::divide_oper(const Value& rhs)
{
    value = value_compute_type::divide(value, rhs);
    accum = accum_compute_type::divide(accum, rhs);
    return static_cast<Type&>(*this);
}
//
//--------------------------------------------------------------------------------------//
//
//              operator + - * / <<
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
Tp
operator+(const base<Tp, Value>& lhs, const base<Tp, Value>& rhs)
{
    return Tp(static_cast<const Tp&>(lhs)) += static_cast<const Tp&>(rhs);
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
Tp
operator-(const base<Tp, Value>& lhs, const base<Tp, Value>& rhs)
{
    return Tp(static_cast<const Tp&>(lhs)) -= static_cast<const Tp&>(rhs);
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
Tp operator*(const base<Tp, Value>& lhs, const base<Tp, Value>& rhs)
{
    return Tp(static_cast<const Tp&>(lhs)) *= static_cast<const Tp&>(rhs);
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp, typename Value>
Tp
operator/(const base<Tp, Value>& lhs, const base<Tp, Value>& rhs)
{
    return Tp(static_cast<const Tp&>(lhs)) /= static_cast<const Tp&>(rhs);
}
//
//======================================================================================//
//
//                              VOID BASE
//
//======================================================================================//
//
template <typename Tp>
void
base<Tp, void>::reset()
{
    is_running   = false;
    is_on_stack  = false;
    is_transient = false;
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp>
void
base<Tp, void>::measure()
{
    // is_running   = false;
    is_transient = false;
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp>
void
base<Tp, void>::set_started()
{
    is_running = true;
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp>
void
base<Tp, void>::set_stopped()
{
    is_running   = false;
    is_transient = true;
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp>
void
base<Tp, void>::get(void*& ptr, size_t _typeid_hash) const
{
    if(!ptr && _typeid_hash == typeid_hash<Tp>())
        ptr = reinterpret_cast<void*>(const_cast<base_type*>(this));
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp>
void
base<Tp, void>::get_opaque_data(void*&, size_t) const
{}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp>
std::string
base<Tp, void>::label()
{
    return metadata<Tp>::label();
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp>
std::string
base<Tp, void>::description()
{
    return metadata<Tp>::description();
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp>
std::string
base<Tp, void>::get_label()
{
    static std::string _instance = Type::label();
    return _instance;
}
//
//--------------------------------------------------------------------------------------//
//
template <typename Tp>
std::string
base<Tp, void>::get_description()
{
    static std::string _instance = Type::description();
    return _instance;
}
//
//--------------------------------------------------------------------------------------//
//
}  // namespace component
}  // namespace tim
//
//======================================================================================//
