//
//  types.h
//  rdvlisp
//
//  Created by Ruben De Visscher on 29/06/14.
//  Copyright (c) 2014 Ruben De Visscher. All rights reserved.
//

#ifndef __rdvlisp__types__
#define __rdvlisp__types__

#include <iostream>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <vector>
#include <sstream>

namespace rdvlisp {
    namespace types {
        class type;
        typedef std::shared_ptr<type> type_ref;
        template <typename T>
        type_ref ref(T t) {
            return std::make_shared<type>(t);
        }
        
        class pod {
        public:
            uint64_t bits;
            pod(uint64_t bits) : bits(bits) {
                if(bits == 0) {
                    throw std::out_of_range("number of bits must be non-zero");
                }
            }
        };
        class integer : public pod {
        public:
            bool is_signed;
            integer(uint64_t bits, bool is_signed=true) : pod(bits), is_signed(is_signed) {}
        };
        class floating_point : public pod {
        public:
            floating_point(uint64_t bits) : pod(bits) {}
        };
        class boolean : public pod {
        public:
            boolean() : pod(1) {}
        };
        class array {
        public:
            boost::optional<uint64_t> length;
            type_ref inner_type;
            array(type_ref inner_type, uint64_t length) : inner_type(inner_type), length(length) {
                if(length == 0) {
                    throw std::out_of_range("array length must be non-zero");
                }
            }
            array(type_ref inner_type) : inner_type(inner_type), length() {}
        };
        class function {
        public:
            type_ref return_type;
            std::vector<type_ref> argument_types;
            bool is_vararg;
            function(type_ref return_type, std::vector<type_ref> argument_types, bool is_vararg=false)
            : return_type(return_type), argument_types(argument_types), is_vararg(is_vararg) {}
        };
        class type_variable {
        public:
            std::string name;
            type_variable(const std::string& name) : name(name) {
                if(name.length() == 0) {
                    throw std::invalid_argument("name must not be an empty string");
                }
            }
        };
        class undetermined {
        public:
        };
        class string {
        public:
        };
        class keyword {
        public:
        };
        
        class type {
        public:
            typedef boost::make_recursive_variant<integer, floating_point, array, function, type_variable, undetermined, string, keyword>::type variant_type;
            variant_type variant;
            template <typename T>
            type(T t) : variant(t) {}
        };
        
        class print_visitor : public boost::static_visitor<std::string> {
        public:
            std::string operator()(undetermined t) const {
                return "undetermined";
            }
            std::string operator()(string t) const {
                return "string";
            }
            std::string operator()(keyword t) const {
                return "keyword";
            }
            std::string operator()(type_variable t) const {
                std::string result;
                return t.name;
            }
            std::string operator()(array t) const {
                std::stringstream ss;
                ss << "(array " << boost::apply_visitor(print_visitor(), t.inner_type->variant);
                if(t.length) {
                    ss << " " << *t.length;
                }
                ss << ")";
                return ss.str();
            }
            std::string operator()(integer t) const {
                std::stringstream ss;
                ss << "(integer " << t.bits << " " << t.is_signed << ")";
                return ss.str();
            }
            std::string operator()(floating_point t) const {
                std::stringstream ss;
                ss << "(floating-point " << t.bits << ")";
                return ss.str();
            }
            std::string operator()(function t) const {
                std::stringstream ss;
                ss << "(function " << boost::apply_visitor(print_visitor(), t.return_type->variant);
                if(t.argument_types.size() > 0) {
                    for(auto at : t.argument_types) {
                        ss << " " << boost::apply_visitor(print_visitor(), at->variant);
                    }
                } else {
                    ss << " void";
                }
                return ss.str();
            }
        };
    }
}

#endif /* defined(__rdvlisp__types__) */
