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
#include <sstream>
#include <iomanip>

class Token {
public:
    enum class Type {
        tuple_start, tuple_end, integer, floating_point, string, identifier, keyword, quote, unquote, tuple_short, error, whitespace
    } type;
    
    size_t start;
    size_t end;
    std::string value;
    Token() : type(Type::error) {}
    Token(Type type, const std::string& value, size_t start, size_t end) : type(type), value(value), start(start), end(end) {}
};

static std::map<Token::Type, std::string> token_type_to_string({
    {Token::Type::tuple_start, "tuple_start"},
    {Token::Type::tuple_end, "tuple_end"},
    {Token::Type::integer, "integer"},
    {Token::Type::floating_point, "floating_point"},
    {Token::Type::string, "string"},
    {Token::Type::identifier, "identifier"},
    {Token::Type::keyword, "keyword"},
    {Token::Type::quote, "quote"},
    {Token::Type::unquote, "unquote"},
    {Token::Type::tuple_short, "tuple_short"},
    {Token::Type::whitespace, "whitespace"},
    {Token::Type::error, "error"}
});

void skip_ws(std::istream& is) {
    while(is.good() and std::isspace(is.peek())) {
        is.get();
    }
}

size_t consume_unsigned_decimal_integer(const std::string& s, size_t current) {
    while(current < s.size() and isdigit(s[current])) { ++current; }
    return current;
}

size_t consume_optionally_signed_decimal_integer(const std::string& s, size_t start) {
    auto current = start;
    if(current < s.size() and (s[current] == '+' or s[current] == '-')) { ++current; }
    auto digits_start = current;
    auto digits_end = consume_unsigned_decimal_integer(s, current);
    if(digits_end == digits_start) {
        return start;
    } else {
        return digits_end;
    }
}

Token get_token_numeric(const std::string& s, size_t start) {
    auto current = start;
    
    if(s.size()-current >= 2) {
        if(s[current] == '0') {
            if(s[current+1] == 'x') {
                return Token(Token::Type::error, "hexadecimal integers are not yet implemented", start, current+2);
            } else if(s[current+1] == 'b') {
                return Token(Token::Type::error, "binary integers are not yet implemented", start, current+2);
            }
        }
    }
    
    current = consume_optionally_signed_decimal_integer(s, current);
    if(current == start) {
        return Token(Token::Type::error, "unexpected end of file while parsing signed numeric type", start, current);
    } else {
        if(current < s.size()) {
            bool is_float = false;
            if(s[current] == '.') {
                ++current;
                auto fractional_start = current;
                current = consume_unsigned_decimal_integer(s, current);
                if(fractional_start == current) {
                    return Token(Token::Type::error, "could not parse floating point fractional part", start, current);
                }
                is_float = true;
            }
            if(current < s.size() and (s[current] == 'e' or s[current] == 'E')) {
                ++current;
                auto exponent_start = current;
                current = consume_optionally_signed_decimal_integer(s, current);
                if(exponent_start == current) {
                    return Token(Token::Type::error, "could not parse floating point exponent", start, current);
                }
                is_float = true;
            }
            if(is_float) {
                return Token(Token::Type::floating_point, s.substr(start, current-start), start, current);
            } else {
                return Token(Token::Type::integer, s.substr(start, current-start), start, current);
            }
        } else {
            return Token(Token::Type::integer, s.substr(start, current-start), start, current);
        }
    }
}

Token get_token_string(const std::string& s, size_t start) {
    auto current = start;
    if(current < s.size() and s[current] == '"') {
        ++current;
        while(current < s.size() and s[current] != '"') {
            if(s[current] == '\\') {
                ++current;
                if(current < s.size()) {
                    switch(s[current]) {
                        case '"':
                        case 'n':
                        case 't':
                        case 'r':
                        case 'v':
                        case 'b':
                        case 'a':
                        case 'f':
                        case '\\':
                            break;
                        default:
                            return Token(Token::Type::error, "unsupported escape sequence", start, current+1);
                    }
                } else {
                    return Token(Token::Type::error, "unexpected end of file while parsing escape sequence", start, current);
                }
            }
            ++current;
        }
        if(current < s.size() and s[current] == '"') {
            ++current;
            return Token(Token::Type::string, s.substr(start, current-start), start, current);
        } else {
            return Token(Token::Type::error, "unexpected end of file while scanning for end of string", start, current);
        }
    } else {
        return Token(Token::Type::error, "could not parse string", start, current+1);
    }
}

static const std::string identifier_punctuation_chars = "~!@#$%^&*_-+=|?/><";

Token get_token_identifier(const std::string& s, size_t start) {
    auto current = start;
    if(current < s.size() and (isalpha(s[current]) or identifier_punctuation_chars.find(s[current]) != std::string::npos)) {
        ++current;
        while(current < s.size() and (isalnum(s[current]) or identifier_punctuation_chars.find(s[current]) != std::string::npos)) { ++current; }
        return Token(Token::Type::identifier, s.substr(start, current-start), start, current);
    } else {
        return Token(Token::Type::error, "could not parse identifier", start, current+1);
    }
}

Token get_token(const std::string& s, size_t start) {
    auto current = start;
    while(current < s.size() and std::isspace(s[current])) {
        ++current;
    }
    
    if(current > start) {
        return Token(Token::Type::whitespace, s.substr(start, current-start), start, current);
    }
    
    if(current >= s.size()) {
        return Token(Token::Type::error, "end of file", s.size(), s.size());
    }
    
    Token token;
    switch(s[current]) {
        case '(':
            return Token(Token::Type::tuple_start, "(", current, current+1);
            break;
        case ')':
            return Token(Token::Type::tuple_end, ")", current, current+1);
            break;
        case '#':
            return Token(Token::Type::tuple_short, "#", current, current+1);
            break;
        case ',':
            return Token(Token::Type::unquote, ",", current, current+1);
            break;
        case '\'':
            return Token(Token::Type::quote, "'", current, current+1);
            break;
        case '-':
            token = get_token_numeric(s, current);
            if(token.type != Token::Type::error) {
                return token;
            } else {
                return get_token_identifier(s, current);
            }
            break;
        case '+':
            token = get_token_numeric(s, current);
            if(token.type != Token::Type::error) {
                return token;
            } else {
                return get_token_identifier(s, current);
            }
            break;
        case '"':
            return get_token_string(s, current);
            break;
        case ':':
            token = get_token_identifier(s, current+1);
            if(token.type != Token::Type::error) {
                return Token(Token::Type::keyword, s.substr(current, token.end-current), current, token.end);
            } else {
                return Token(Token::Type::error, "could not parse keyword", current, current+1);
            }
        default:
            if(isdigit(s[current]) or isxdigit(s[current])) {
                return get_token_numeric(s, current);
            } else if(isalpha(s[current]) or identifier_punctuation_chars.find(s[current]) != std::string::npos) {
                return get_token_identifier(s, current);
            } else {
                std::stringstream ss;
                ss << "unexpected character '" << s[current] << "'";
                return Token(Token::Type::error, ss.str(), current, current+1);
            }
    }
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

std::ostream& rdvlisp::ast::operator<<(std::ostream& os, Expression expression) {
    expression_print_visitor v(os);
    return expression.variant.apply_visitor(v);
}

std::ostream& rdvlisp::ast::operator<<(std::ostream& os, Tuple tuple) {
    os << "(";
    if(tuple.elements.size() > 0) {
        os << *tuple.elements[0];
        for(auto it = ++tuple.elements.begin(); it != tuple.elements.end(); ++it) {
            os << " " << **it;
        }
    }
    return os << ")";
}

std::ostream& rdvlisp::ast::operator<<(std::ostream& os, Identifier identifier) {
    if(identifier.name.size() > 0) {
        os << identifier.name[0];
    }
    for(auto it = ++identifier.name.begin(); it != identifier.name.end(); ++it) {
        os << "." << *it;
    }
    return os;
}

std::ostream& rdvlisp::ast::operator<<(std::ostream& os, Keyword keyword) {
    return os << ":" << keyword.name.substr(1, keyword.name.size()-1);
}

std::ostream& rdvlisp::ast::operator<<(std::ostream& os, FloatingPoint floating_point) {
    return os << floating_point.value;
}

std::ostream& rdvlisp::ast::operator<<(std::ostream& os, Integer integer) {
    return os << integer.value;
}

std::ostream& rdvlisp::ast::operator<<(std::ostream& os, String string) {
    os << "\"";
    for(char c : string.contents) {
        switch(c) {
            case '\n':
                os << "\\n";
                break;
            case '\t':
                os << "\\t";
                break;
            case '\r':
                os << "\\r";
                break;
            case '\v':
                os << "\\v";
                break;
            case '\b':
                os << "\\b";
                break;
            case '\a':
                os << "\\a";
                break;
            default:
                os << c;
        }
    }
    return os << "\"";
}

Result<Tuple> read_tuple(const std::string& s, size_t start) {
    auto current = start;
    std::vector<ExpressionRef> elements;
    Token token = get_token(s, start);
    if(token.type != Token::Type::tuple_start) {
        return Result<Tuple>("tuple not started with '('", start, token.end);
    }
    current = token.end;
    bool is_first = true;
    bool sepby_whitespace = false;
    while(true) {
        token = get_token(s, current);
        if(token.type == Token::Type::error) {
            return Result<Tuple>(token.value, start, token.end);
        }
        if(token.type == Token::Type::whitespace) {
            sepby_whitespace = true;
            current = token.end;
            continue;
        }
        if(token.type == Token::Type::tuple_end) {
            return Result<Tuple>(Tuple(elements), start, token.end);
        }
        if(!is_first and !sepby_whitespace) {
            return Result<Tuple>("tuple elements must be separated by whitespace", start, token.end);
        }
        auto r = read(s, current);
        if(r.good()) {
            elements.push_back(r.get());
            current = r.end;
        } else {
            return Result<Tuple>(r.error(), start, r.end);
        }
        is_first = false;
        sepby_whitespace = false;
    }
}

template <typename T>
Result<ExpressionRef> make_result(const Result<T>& r) {
    if(r.good()) {
        return Result<ExpressionRef>(std::make_shared<Expression>(r.get()), r.start, r.end);
    } else {
        return Result<ExpressionRef>(r.error(), r.start, r.end);
    }
}

Result<ExpressionRef> rdvlisp::read(const std::string& s, size_t start) {
    size_t current = start;
    
    Token token;
    while(current < s.size()) {
        token = get_token(s, current);
        if(token.type == Token::Type::whitespace) {
            start = token.end;
            current = token.end;
            continue;
        }
        std::stringstream ss;
        size_t i;
        switch(token.type) {
            case Token::Type::tuple_start:
                return make_result(read_tuple(s, current));
                break;
            case Token::Type::string:
                ss.str("");
                ss.clear();
                i = 1;
                while(i < token.value.size()-1) {
                    if(token.value[i] == '\\') {
                        ++i;
                        if(token.value[i] == 'n') {
                            ss << '\n';
                        } else if(token.value[i] == 't') {
                            ss << '\t';
                        } else if(token.value[i] == 'r') {
                            ss << '\r';
                        } else if(token.value[i] == 'f') {
                            ss << '\f';
                        } else if(token.value[i] == 'v') {
                            ss << '\v';
                        } else if(token.value[i] == 'b') {
                            ss << '\b';
                        } else if(token.value[i] == 'a') {
                            ss << '\a';
                        } else if(token.value[i] == '"' or token.value[i] == '\\') {
                            ss << token.value[i];
                        }
                    } else {
                        ss << token.value[i];
                    }
                    ++i;
                }
                return make_result(Result<String>(String(ss.str()), start, token.end));
                break;
            case Token::Type::identifier:
                return make_result(Result<Identifier>(Identifier(token.value), start, token.end));
                break;
            case Token::Type::integer:
                return make_result(Result<Integer>(Integer(token.value), start, token.end));
                break;
            case Token::Type::floating_point:
                return make_result(Result<FloatingPoint>(FloatingPoint(token.value), start, token.end));
                break;
            case Token::Type::keyword:
                return make_result(Result<Keyword>(Keyword(token.value), start, token.end));
                break;
            default:
                return Result<ExpressionRef>("unexpected token of type " + token_type_to_string[token.type] + " encountered", start, token.end);
        }
    }
    
    return Result<ExpressionRef>("reached end of file", start, current);
}