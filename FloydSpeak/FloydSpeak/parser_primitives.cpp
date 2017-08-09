//
//  parser_primitives.cpp
//  FloydSpeak
//
//  Created by Marcus Zetterquist on 26/07/16.
//  Copyright © 2016 Marcus Zetterquist. All rights reserved.
//


#include "parser_primitives.h"


#include "text_parser.h"

#include "steady_vector.h"
#include "statements.h"
#include "parser_value.h"
#include "json_support.h"
#include "json_parser.h"
#include "json_writer.h"

#include <string>
#include <memory>
#include <map>
#include <iostream>
#include <cmath>

namespace floyd_parser {

using std::vector;
using std::string;
using std::pair;


//////////////////////////////////////////////////		Text parsing primitives



std::string skip_whitespace(const string& s){
	return read_while(seq_t(s), whitespace_chars).second.get_s();
}

QUARK_UNIT_TEST("", "skip_whitespace()", "", ""){
	QUARK_TEST_VERIFY(skip_whitespace("") == "");
	QUARK_TEST_VERIFY(skip_whitespace(" ") == "");
	QUARK_TEST_VERIFY(skip_whitespace("\t") == "");
	QUARK_TEST_VERIFY(skip_whitespace("int blob()") == "int blob()");
	QUARK_TEST_VERIFY(skip_whitespace("\t\t\t int blob()") == "int blob()");
}


seq_t skip_whitespace(const seq_t& s){
	return read_while(s, whitespace_chars).second;
}


bool is_whitespace(char ch){
	return whitespace_chars.find(string(1, ch)) != string::npos;
}

QUARK_UNIT_TEST("", "is_whitespace()", "", ""){
	QUARK_TEST_VERIFY(is_whitespace('x') == false);
	QUARK_TEST_VERIFY(is_whitespace(' ') == true);
	QUARK_TEST_VERIFY(is_whitespace('\t') == true);
	QUARK_TEST_VERIFY(is_whitespace('\n') == true);
}


bool is_start_char(char c){
	return c == '(' || c == '[' || c == '{';
}

QUARK_UNIT_TEST("", "is_start_char()", "", ""){
	QUARK_TEST_VERIFY(is_start_char('('));
	QUARK_TEST_VERIFY(is_start_char('['));
	QUARK_TEST_VERIFY(is_start_char('{'));
	QUARK_TEST_VERIFY(!is_start_char('<'));

	QUARK_TEST_VERIFY(!is_start_char(')'));
	QUARK_TEST_VERIFY(!is_start_char(']'));
	QUARK_TEST_VERIFY(!is_start_char('}'));
	QUARK_TEST_VERIFY(!is_start_char('>'));

	QUARK_TEST_VERIFY(!is_start_char(' '));
}


bool is_end_char(char c){
	return c == ')' || c == ']' || c == '}';
}

QUARK_UNIT_TEST("", "is_end_char()", "", ""){
	QUARK_TEST_VERIFY(!is_end_char('('));
	QUARK_TEST_VERIFY(!is_end_char('['));
	QUARK_TEST_VERIFY(!is_end_char('{'));
	QUARK_TEST_VERIFY(!is_end_char('<'));

	QUARK_TEST_VERIFY(is_end_char(')'));
	QUARK_TEST_VERIFY(is_end_char(']'));
	QUARK_TEST_VERIFY(is_end_char('}'));
	QUARK_TEST_VERIFY(!is_end_char('>'));

	QUARK_TEST_VERIFY(!is_end_char(' '));
}


char start_char_to_end_char(char start_char){
	QUARK_ASSERT(is_start_char(start_char));

	if(start_char == '('){
		return ')';
	}
	else if(start_char == '['){
		return ']';
	}
	else if(start_char == '{'){
		return '}';
	}
	else {
		QUARK_ASSERT(false);
	}
}

QUARK_UNIT_TEST("", "start_char_to_end_char()", "", ""){
	QUARK_TEST_VERIFY(start_char_to_end_char('(') == ')');
	QUARK_TEST_VERIFY(start_char_to_end_char('[') == ']');
	QUARK_TEST_VERIFY(start_char_to_end_char('{') == '}');
}

/*
	First char is the start char, like '(' or '{'.

	Checks *all* balancing-chars
	??? Should be recursive and not just count intermediate chars, also pair match them.
*/
std::pair<std::string, seq_t> get_balanced(const seq_t& s){
	QUARK_ASSERT(s.size() > 0);

	const char start_char = s.first1_char();
	QUARK_ASSERT(is_start_char(start_char));

	const char end_char = start_char_to_end_char(start_char);

	int depth = 0;
	auto pos = s;
	string result;
	while(pos.empty() == false && !(depth == 1 && pos.first1_char() == end_char)){
		const char ch = pos.first1_char();
		if(is_start_char(ch)) {
			depth++;
		}
		else if(is_end_char(ch)){
			if(depth == 0){
				throw std::runtime_error("unbalanced ([{< >}])");
			}
			depth--;
		}
		result = result + pos.first1();
		pos = pos.rest1();
	}

	return { result + pos.first1(), pos.rest1() };
}

QUARK_UNIT_TEST("", "get_balanced()", "", ""){
//	QUARK_TEST_VERIFY(get_balanced("") == seq("", ""));
	QUARK_TEST_VERIFY(get_balanced(seq_t("()")) == (std::pair<std::string, seq_t>("()", seq_t(""))));
	QUARK_TEST_VERIFY(get_balanced(seq_t("(abc)")) == (std::pair<std::string, seq_t>("(abc)", seq_t(""))));
	QUARK_TEST_VERIFY(get_balanced(seq_t("(abc)def")) == (std::pair<std::string, seq_t>("(abc)", seq_t("def"))));
	QUARK_TEST_VERIFY(get_balanced(seq_t("((abc))def")) == (std::pair<std::string, seq_t>("((abc))", seq_t("def"))));
	QUARK_TEST_VERIFY(get_balanced(seq_t("((abc)[])def")) == (std::pair<std::string, seq_t>("((abc)[])", seq_t("def"))));
	QUARK_TEST_VERIFY(get_balanced(seq_t("(return 4 < 5;)xxx")) == (std::pair<std::string, seq_t>("(return 4 < 5;)", seq_t("xxx"))));

	QUARK_TEST_VERIFY(get_balanced(seq_t("{}")) == (std::pair<std::string, seq_t>("{}", seq_t(""))));
	QUARK_TEST_VERIFY(get_balanced(seq_t("{aaa}bbb")) == (std::pair<std::string, seq_t>("{aaa}", seq_t("bbb"))));
	QUARK_TEST_VERIFY(get_balanced(seq_t("{return 4 < 5;}xxx")) == (std::pair<std::string, seq_t>("{return 4 < 5;}", seq_t("xxx"))));

//	QUARK_TEST_VERIFY(get_balanced("{\n\t\t\t\treturn 4 < 5;\n\t\t\t}\n\t\t") == seq("((abc)[])", "def"));
}



//////////////////////////////////////		SYMBOLS



std::pair<std::string, seq_t> read_required_single_symbol(const seq_t& s){
	const auto a = skip_whitespace(s);
	const auto b = read_while(a, identifier_chars);

	if(b.first.empty()){
		throw std::runtime_error("missing identifier");
	}
	return b;
}

QUARK_UNIT_TESTQ("read_required_single_symbol()", ""){
	QUARK_TEST_VERIFY(read_required_single_symbol(seq_t("\thello\txxx")) == (std::pair<std::string, seq_t>("hello", seq_t("\txxx"))));
}


//////////////////////////////////////		TYPE IDENTIFIERS





	//////////////////////////////////////////////////		type_identifier_t



	type_identifier_t type_identifier_t::make(const std::string& s){
		QUARK_ASSERT(is_valid_type_identifier(s));

		const type_identifier_t result(s);

		QUARK_ASSERT(result.check_invariant());
		return result;
	}


	type_identifier_t::type_identifier_t(const type_identifier_t& other) :
		_type_magic(other._type_magic)
	{
		QUARK_ASSERT(check_invariant());
		QUARK_ASSERT(other.check_invariant());
	}

	bool type_identifier_t::operator==(const type_identifier_t& other) const{
		QUARK_ASSERT(check_invariant());
		QUARK_ASSERT(other.check_invariant());

		return _type_magic == other._type_magic /*&& compare_shared_values(_resolved, other._resolved)*/;
	}

	bool type_identifier_t::operator!=(const type_identifier_t& other) const{
		return !(*this == other);
	}


	type_identifier_t::type_identifier_t(const char s[]) :
		_type_magic(s)
	{
		QUARK_ASSERT(s != nullptr);
		QUARK_ASSERT(is_valid_type_identifier(std::string(s)));

		QUARK_ASSERT(check_invariant());
	}

	type_identifier_t::type_identifier_t(const std::string& s) :
		_type_magic(s)
	{
		QUARK_ASSERT(is_valid_type_identifier(s));

		QUARK_ASSERT(check_invariant());
	}

	void type_identifier_t::swap(type_identifier_t& other){
		QUARK_ASSERT(check_invariant());
		QUARK_ASSERT(other.check_invariant());

		_type_magic.swap(other._type_magic);
	}

	std::string type_identifier_t::to_string() const {
		QUARK_ASSERT(check_invariant());

		return _type_magic;
	}

	bool type_identifier_t::check_invariant() const {
		QUARK_ASSERT(_type_magic != "");
		QUARK_ASSERT(is_valid_type_identifier(_type_magic));
		return true;
	}


	void trace(const type_identifier_t& v){
		QUARK_TRACE("type_identifier_t <" + v.to_string() + ">");
	}





std::pair<std::string, seq_t> read_type(const seq_t& s){
	const auto a = skip_whitespace(s);
	const auto b = read_while(a, type_chars);
	return b;
}

pair<type_identifier_t, seq_t> read_required_type_identifier(const seq_t& s){
	const auto type_pos = read_type(s);
	if(type_pos.first.empty()){
		throw std::runtime_error("illegal character in type identifier");
	}
	const auto type = type_identifier_t::make(type_pos.first);
	return { type, skip_whitespace(type_pos.second) };
}

	bool is_valid_type_identifier(const std::string& s){
		const auto a = read_while(seq_t(s), floyd_parser::type_chars);
		return a.first == s;
	}


//??? add all. Use map<>
bool is_math2_op(const std::string& op){
	return
		op == "+" || op == "-" || op == "*" || op == "/" || op == "%"
		|| op == "<=" || op == "<" || op == ">=" || op == ">"
		|| op == "==" || op == "!=" || op == "&&" || op == "||" || op == "neg";
}



//////////////////////////////////////		FLOYD JSON BASICS





json_t make_member_def(const std::string& type, const std::string& name, const json_t& expression){
	QUARK_ASSERT(type.empty() || (type.size() > 2 && type.front() == '<' && type.back() == '>'));
	QUARK_ASSERT(expression.check_invariant());

	if(expression.is_null()){
		return json_t::make_object({
			{ "type", type },
			{ "name", name }
		});
	}
	else{
		return json_t::make_object({
			{ "type", type },
			{ "name", name },
			{ "expr", expression }
		});
	}
}


}	//	floyd_parser
