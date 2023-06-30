#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").

#include <microlib/support-machinery/basic-types.hpp>           // C_string_ptr, Mutable_cstr_ptr
#include <microlib/support-machinery/exception-handling.hpp>    // SM_FAIL, hopefully, fail_, fail, rethrow_any_nested_x_of, with_messages_of, messages_of
#include <microlib/support-machinery/Interval_.hpp>             // Interval_, is_in, zero_to, one_through
#include <microlib/support-machinery/misc.hpp>
#include <microlib/support-machinery/string-building.hpp>       // ~, sb, operator<<, inline namespace string_building
#include <microlib/support-machinery/type-builders.hpp>         // const_, ref_, in_
