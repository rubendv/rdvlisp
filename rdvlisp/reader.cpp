//
//  reader.cpp
//  rdvlisp
//
//  Created by Ruben De Visscher on 29/06/14.
//  Copyright (c) 2014 Ruben De Visscher. All rights reserved.
//

#include "reader.h"
#include <regex>
#include <map>

class token {
public:
    enum class type {
        array_start, array_end, integer, floating_point, string, identifier, quote, unquote, array_short, error
    } t;
    size_t start;
    size_t end;
    std::string value;
    token(type t, const std::string& value, size_t start, size_t end) : t(t), value(value), start(start), end(end) {}
};

static const std::string identifier_first_pattern = R"([_a-zA-Z\^~!@$%&*-+=|><?/])";
static const std::string identifier_rest_pattern = "(" + identifier_first_pattern + R"(|(\d)))";
static const std::string identifier_pattern = identifier_first_pattern + identifier_rest_pattern + "+";
static const std::string integer_pattern = R"((((-|+)?\d+)|0(x[0-9a-fA-F])|(b[0-1]+))(_)" + identifier_pattern + ")?";
static const std::string float_pattern = R"([-+]?\d+(\.\d+)?([eE][-+]?\d+)?)(_)" + identifier_pattern + ")?";
std::vector<std::pair<token::type, std::regex>> regexes({
    {token::type::array_start, std::regex("\\(")},
    {token::type::array_end, std::regex("\\)")},
    {token::type::floating_point, std::regex(float_pattern)},
    {token::type::integer, std::regex(integer_pattern)},
    {token::type::identifier, std::regex(identifier_pattern)},
    {token::type::quote, std::regex("'")},
    {token::type::unquote, std::regex(",")},
    {token::type::array_short, std::regex("#")}
});

void skip_ws(std::istream& is) {
    while(is.good() and std::isspace(is.peek())) {
        is.get();
    }
}

token get_token(const std::string& s, size_t offset) {
    std::match_results<std::string::const_iterator> results;
    while(offset < s.size() and std::isspace(s[offset])) {
        ++offset;
    }
    for(auto pair : regexes) {
        bool matched = std::regex_search(s.begin()+offset, s.end(), results, pair.second, std::regex_constants::match_continuous);
        if(matched) {
            return token(pair.first, results[0].str(), offset, results[0].length());
        }
    }
    return token(token::type::error, "", 0, 0);
}

using namespace rdvlisp::ast;
using namespace rdvlisp;

class expression_print_visitor : public boost::static_visitor<std::ostream&> {
    std::ostream& os;
public:
    expression_print_visitor(std::ostream& os) : os(os) {}
    template <typename T>
    std::ostream& operator()(T t) {
        return os << t;
    }
};

std::ostream& rdvlisp::ast::operator<<(std::ostream& os, expression expression) {
    expression_print_visitor v(os);
    return expression.variant.apply_visitor(v);
}

std::ostream& rdvlisp::ast::operator<<(std::ostream& os, array array) {
    os << "(";
    if(array.elements.size() > 0) {
        os << *array.elements[0];
        for(auto it = ++array.elements.begin(); it != array.elements.end(); ++it) {
            os << " " << **it;
        }
    }
    return os << ")";
}

std::ostream& rdvlisp::ast::operator<<(std::ostream& os, identifier identifier) {
    if(identifier.name.size() > 0) {
        return os << identifier.name[0];
    }
    for(auto it = ++identifier.name.begin(); it != identifier.name.end(); ++it) {
        os << "." << *it;
    }
    return os;
}

/*
result<array> read_array(std::istream& is) {
    std::vector<expression_ref> elements;
    if(is.peek() != '(') {
        return result<array>("arrays must start with '('", is.tellg(), is.tellg()+std::istream::streamoff(1));
    }
    auto start = is.tellg();
    // Consume opening parenthesis
    is.get();
    skip_ws(is);
    while(is.good() and is.peek() != ')') {
        auto r = read(is);
        if(r.good()) {
            elements.push_back(r.get());
        } else {
            return result<array>(r.error(), r.start, r.end);
        }
        skip_ws(is);
    }
    if(is.get() != ')') {
        return result<array>("array not ended with ')'", start, is.tellg());
    } else {
        return result<array>(array(elements), start, is.tellg());
    }
}

result<identifier> read_identifier(std::istream& is) {
    std::stringstream name_ss;
    
    auto start = is.tellg();
    while(is.good() and (std::isalpha(is.peek()) or std::string("!@#$%^&*-+=_|/?<>").find(is.peek()) != std::string::npos)) {
        name_ss << (char)is.get();
    }
    
    auto name = name_ss.str();
    
    if(name.size() > 0) {
        return result<identifier>(identifier(name), start, is.tellg());
    } else {
        return result<identifier>(std::string("unexpected character or end of file"), start, is.tellg());
    }
}

template <typename T>
result<expression_ref> make_result(const result<T>& r) {
    if(r.good()) {
        return result<expression_ref>(std::make_shared<expression>(r.get()), r.start, r.end);
    } else {
        return result<expression_ref>(r.error(), r.start, r.end);
    }
}
*/
result<expression_ref> rdvlisp::read(const std::string& s) {
    
    /*skip_ws(is);
    
    while(is.good()) {
        int c = is.peek();
        auto start = is.tellg();
        if(c == '(') {
            return make_result(read_array(is));
        } else if(std::isdigit(c) or c == '-' or c == '+') {
            auto r_int = read_integer(is);
            if(r_int.good()) {
                return make_result(r_int);
            } else {
                is.seekg(start);
                auto r_float = read_floating_point(is);
                return make_result(r_float);
            }
        } else if(std::isalpha(c) or std::string("~!@$%^&*-+=_|/?<>").find(c) != std::string::npos) {
            auto r = read_identifier(is);
            return make_result(r);
        } else if(c == '\'' or c == ',') {
            auto start = is.tellg();
            is.get();
            auto r = read(is);
            auto expr_result = make_result(r);
            if(expr_result.good()) {
                expression_ref expr_ref = expr_result.get();
                std::string name;
                if(c == '\'') {
                    name = "quote";
                } else if(c == ',') {
                    name = "unquote";
                }
                expression_ref quote_ident = std::make_shared<expression>(identifier(name));
                expression_ref quoted = std::make_shared<expression>(array({quote_ident, expr_ref}));
                return result<expression_ref>(quoted, start, r.end);
            }
        } else if(c == '#') {
            auto start = is.tellg();
            is.get();
            auto r = read_array(is);
            if(r.good()) {
                array arr = r.get();
                expression_ref quote_ident = std::make_shared<expression>(identifier("array"));
                std::vector<expression_ref> elements({quote_ident});
                for(auto element : arr.elements) {
                    elements.push_back(element);
                }
                expression_ref quoted = std::make_shared<expression>(array(elements));
                return result<expression_ref>(quoted, start, r.end);
            } else {
                return result<expression_ref>("'#' must be followed by an array literal", start, is.tellg());
            }
        } else {
            std::stringstream ss;
            ss << "unexpected character '" << (char)c << "'";
            return result<expression_ref>(ss.str(), is.tellg(), is.tellg()+std::istream::streamoff(1));
        }
    }*/
    return result<expression_ref>("reached end of file", 0, 0);
}