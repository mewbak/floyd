//
//  parser2.h
//  FloydSpeak
//
//  Created by Marcus Zetterquist on 24/08/16.
//  Copyright © 2016 Marcus Zetterquist. All rights reserved.
//

//	Based on Magnus snippet.

#include "quark.h"

#include <string>
#include <memory>

#include "text_parser.h"


#include "parser2.h"

using namespace std;





QUARK_UNIT_TESTQ("enum class()", ""){
	enum class my_enum {
		k_one = 1,
		k_four = 4
	};

	QUARK_UT_VERIFY(my_enum::k_one == my_enum::k_one);
	QUARK_UT_VERIFY(my_enum::k_one != my_enum::k_four);
	QUARK_UT_VERIFY(static_cast<int>(my_enum::k_one) == 1);
}




seq_t skip_whitespace(const seq_t& p) {
	QUARK_ASSERT(p.check_invariant());

	return read_while(p, k_c99_whitespace_chars).second;
}



pair<std::string, seq_t> parse_string_literal(const seq_t& p){
	QUARK_ASSERT(!p.empty());
	QUARK_ASSERT(p.first_char() == '\"');

	const auto pos = p.rest();
	const auto s = read_while_not(pos, "\"");
	return { s.first, s.second.rest() };
}

QUARK_UNIT_TESTQ("parse_string_literal()", ""){
	quark::ut_compare(parse_string_literal(seq_t("\"\" xxx")), pair<std::string, seq_t>("", seq_t(" xxx")));
}

QUARK_UNIT_TESTQ("parse_string_literal()", ""){
	quark::ut_compare(parse_string_literal(seq_t("\"hello\" xxx")), pair<std::string, seq_t>("hello", seq_t(" xxx")));
}

QUARK_UNIT_TESTQ("parse_string_literal()", ""){
	quark::ut_compare(parse_string_literal(seq_t("\".5\" xxx")), pair<std::string, seq_t>(".5", seq_t(" xxx")));
}






pair<int, seq_t> parse_long(const seq_t& s) {
	QUARK_ASSERT(s.check_invariant());

	char* e = nullptr;
	long value = strtol(s.c_str(), &e, 0);
	size_t used = e - s.c_str();
	if (used == 0){
		throw std::runtime_error("Invalid number");
	}
	return { (int)value, s.rest(used) };
}

QUARK_UNIT_1("parse_long()", "", (parse_long(seq_t("0")) == pair<int, seq_t>{ 0, seq_t("") }));
QUARK_UNIT_1("parse_long()", "", (parse_long(seq_t("0xyz")) == pair<int, seq_t>{ 0, seq_t("xyz") }));
QUARK_UNIT_1("parse_long()", "", (parse_long(seq_t("13xyz")) == pair<int, seq_t>{ 13, seq_t("xyz") }));





template<typename EXPRESSION>
struct my_helper : public maker<EXPRESSION> {


	public: virtual const EXPRESSION maker__on_string(const eoperation op, const std::string& s) const{
		if(op == eoperation::k_0_number_constant){
			return stoi(s);
		}
		else if(op == eoperation::k_0_resolve){
			return 0;
		}
		else if(op == eoperation::k_0_string_literal){
			return 0;
		}
		else{
			QUARK_ASSERT(false);
		}
	}
	public: virtual const EXPRESSION maker__make(const eoperation op, const EXPRESSION& expr) const{
		if(op == eoperation::k_1_logical_not){
			return -expr;
		}
		else if(op == eoperation::k_1_load){
			return 0;
		}
		else{
			QUARK_ASSERT(false);
		}
	}
	public: virtual const EXPRESSION maker__make(const eoperation op, const EXPRESSION& lhs, const EXPRESSION& rhs) const{
		if(op == eoperation::k_2_add){
			return lhs + rhs;
		}
		else if(op == eoperation::k_2_subtract){
			return lhs - rhs;
		}
		else if(op == eoperation::k_2_multiply){
			return lhs * rhs;
		}
		else if(op == eoperation::k_2_divide){
			return lhs / rhs;
		}
		else if(op == eoperation::k_2_remainder){
			return lhs % rhs;
		}

		else if(op == eoperation::k_2_smaller_or_equal){
			return lhs <= rhs;
		}
		else if(op == eoperation::k_2_smaller){
			return lhs < rhs;
		}
		else if(op == eoperation::k_2_larger_or_equal){
			return lhs >= rhs;
		}
		else if(op == eoperation::k_2_larger){
			return lhs > rhs;
		}


		else if(op == eoperation::k_2_logical_equal){
			return lhs == rhs;
		}
		else if(op == eoperation::k_2_logical_nonequal){
			return lhs != rhs;
		}
		else if(op == eoperation::k_2_logical_and){
			return lhs != 0 && rhs != 0;
		}
		else if(op == eoperation::k_2_logical_or){
			return lhs != 0 || rhs != 0;
		}
		else{
			QUARK_ASSERT(false);
		}
	}
	public: virtual const EXPRESSION maker__make(const eoperation op, const EXPRESSION& e1, const EXPRESSION& e2, const EXPRESSION& e3) const{
		if(op == eoperation::k_3_conditional_operator){
			return e1 != 0 ? e2 : e3;
		}
		else{
			QUARK_ASSERT(false);
		}
	}

	public: virtual const EXPRESSION maker__call(const EXPRESSION& f, const std::vector<EXPRESSION>& args) const{
		return 0;
	}

	public: virtual const EXPRESSION maker__member_access(const EXPRESSION& address, const std::string& member_name) const{
		return 0;
	}

};


bool test_evaluate_single(const std::string& expression, int expected_value, const std::string& expected_seq){
	my_helper<int64_t> helper;
	const auto result = evaluate_single<int64_t>(helper, seq_t(expression));
	if(result.first != expected_value){
		return false;
	}
	else if(result.second.get_all() != expected_seq){
		return false;
	}
	return true;
}

/*
QUARK_UNIT_1("evaluate_single()", "f(3)", test_evaluate_single("f(3)", 0, ""));
QUARK_UNIT_1("evaluate_single()", "f(3)", test_evaluate_single("f(3 + 4, 4 * g(1000 + 2345), 5)", 0, ""));
QUARK_UNIT_1("evaluate_single()", "f(3)", test_evaluate_single("f(3 + 4, 4 * g(\"hello\"), 5)", 0, ""));
*/
//??? test member access etc.QUARK_UNIT_1("evaluate_single()", "f(3)", test_evaluate_single("f(3 + 4, 4 * g(\"hello\"), 5)", 0, ""));


pair<int, seq_t> test_evaluate_int_expr(const seq_t& p){
	QUARK_ASSERT(p.check_invariant());

	my_helper<int64_t> helper;
	return evaluate_expression<int64_t>(helper, p);
}

bool test(const std::string& expression, int expected_int, string expected_seq){
	const auto result = test_evaluate_int_expr(seq_t(expression));
	if(result.first != expected_int){
		return false;
	}
	else if(result.second.get_all() != expected_seq){
		return false;
	}
	return true;
}

QUARK_UNIT_TESTQ("test_evaluate_int_expr()", ""){
	try{
		test_evaluate_int_expr(seq_t(""));
		QUARK_UT_VERIFY(false);
	}
	catch(...){
	}
}

QUARK_UNIT_1("test_evaluate_int_expr()", "", test("0", 0, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "", test("1234567890", 1234567890, ""));

QUARK_UNIT_1("test_evaluate_int_expr()", "", test("10 + 4", 14, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "", test("1 + 2 + 3 + 4", 10, ""));

QUARK_UNIT_1("test_evaluate_int_expr()", "", test("1 + 8 + 7 + 2 * 3 + 4 * 5 + 6", 48, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "", test("10 * 4", 40, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "", test("10 * 4 * 3", 120, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "", test("40 / 4", 10, ""));

QUARK_UNIT_TESTQ("test_evaluate_int_expr()", ""){ 
	QUARK_ASSERT((40 / 5 / 2) == 4);
	QUARK_UT_VERIFY(test("40 / 5 / 2", 4, ""));
}

QUARK_UNIT_1("test_evaluate_int_expr()", "", test("41 % 5", 1, ""));
QUARK_UNIT_TESTQ("test_evaluate_int_expr()", ""){
	QUARK_ASSERT((413 % 50 % 10) == 3);
	QUARK_UT_VERIFY(test("413 % 50 % 10", 3, ""));
}

QUARK_UNIT_1("test_evaluate_int_expr()", "", test("(3)", 3, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "", test("(3 * 8)", 24, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "", test("1 + 3 * 2 + 100", 107, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "", test("-(3 * 2 + (8 * 2)) - (((1))) * 2", -(3 * 2 + (8 * 2)) - (((1))) * 2, ""));

#if false
QUARK_UNIT_1("test_evaluate_int_expr()", "?:", test("1 ? 2 : 3", 2, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "?:", test("0 ? 2 : 3", 3, ""));
#endif

QUARK_UNIT_1("test_evaluate_int_expr()", "<=", test("4 <= 4", 1, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "<=", test("3 <= 4", 1, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "<=", test("5 <= 4", 0, ""));

QUARK_UNIT_1("test_evaluate_int_expr()", "<", test("2 < 3", 1, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "<", test("3 < 3", 0, ""));

QUARK_UNIT_1("test_evaluate_int_expr()", ">=", test("4 >= 4", 1, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", ">=", test("4 >= 3", 1, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", ">=", test("4 >= 5", 0, ""));

QUARK_UNIT_1("test_evaluate_int_expr()", ">", test("3 > 2", 1, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", ">", test("3 > 3", 0, ""));

QUARK_UNIT_1("test_evaluate_int_expr()", "==", test("4 == 4", 1, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "==", test("4 == 5", 0, ""));

QUARK_UNIT_1("test_evaluate_int_expr()", "!=", test("1 != 2", 1, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "!=", test("3 != 3", 0, ""));

QUARK_UNIT_1("test_evaluate_int_expr()", "&&", test("0 && 0", 0, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "&&", test("0 && 1", 0, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "&&", test("1 && 0", 0, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "&&", test("1 && 1", 1, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "&&", test("0 && 0 && 0", 0, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "&&", test("0 && 0 && 1", 0, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "&&", test("0 && 1 && 0", 0, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "&&", test("0 && 1 && 1", 0, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "&&", test("1 && 0 && 0", 0, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "&&", test("1 && 0 && 1", 0, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "&&", test("1 && 1 && 0", 0, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "&&", test("1 && 1 && 1", 1, ""));
QUARK_UNIT_TESTQ("test_evaluate_int_expr()", "&&"){
	QUARK_UT_VERIFY((1 * 1 && 0 + 1) == true);
	QUARK_UT_VERIFY(test("1 * 1 && 0 + 1", 1, ""));
}
QUARK_UNIT_1("test_evaluate_int_expr()", "&&", test("1 * 1 && 0 * 1", 0, ""));

QUARK_UNIT_1("test_evaluate_int_expr()", "||", test("0 || 0", 0, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "||", test("0 || 1", 1, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "||", test("1 || 0", 1, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "||", test("1 || 1", 1, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "||", test("0 || 0 || 0", 0, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "||", test("0 || 0 || 1", 1, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "||", test("0 || 1 || 0", 1, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "||", test("0 || 1 || 1", 1, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "||", test("1 || 0 || 0", 1, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "||", test("1 || 0 || 1", 1, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "||", test("1 || 1 || 0", 1, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "||", test("1 || 1 || 1", 1, ""));

QUARK_UNIT_1("test_evaluate_int_expr()", "", test("10 + my_variable", 10, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "", test("10 + \"my string\"", 10, ""));


QUARK_UNIT_1("test_evaluate_int_expr()", "f()", test("f()", 0, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "f(3)", test("f(3)", 0, ""));
QUARK_UNIT_1("test_evaluate_int_expr()", "f(3)", test("f(3, 4, 5)", 0, ""));





/////////////////////////////////		TO JSON

#include "json_support.h"

template<typename EXPRESSION>
struct json_helper : public maker<EXPRESSION> {


	public: virtual const EXPRESSION maker__on_string(const eoperation op, const std::string& s) const{
		if(op == eoperation::k_0_number_constant){
			return json_value_t::make_array_skip_nulls({ json_value_t("k"), json_value_t("<int>"), json_value_t((double)stoi(s)) });
		}
		else if(op == eoperation::k_0_resolve){
			return json_value_t::make_array_skip_nulls({ json_value_t("@"), json_value_t(), json_value_t(s) });
		}
		else if(op == eoperation::k_0_string_literal){
			return json_value_t::make_array_skip_nulls({ json_value_t("k"), json_value_t("<string>"), json_value_t(s) });
		}
		else{
			QUARK_ASSERT(false);
		}
	}
	public: virtual const EXPRESSION maker__make(const eoperation op, const EXPRESSION& expr) const{
		if(op == eoperation::k_1_logical_not){
			return json_value_t::make_array_skip_nulls({ json_value_t("neg"), json_value_t(), expr });
		}
		else if(op == eoperation::k_1_load){
			return json_value_t::make_array_skip_nulls({ json_value_t("load"), json_value_t(), expr });
		}
		else{
			QUARK_ASSERT(false);
		}
	}

	private: static const std::map<eoperation, string> _2_operator_to_string;

	public: virtual const EXPRESSION maker__make(const eoperation op, const EXPRESSION& lhs, const EXPRESSION& rhs) const{
		const auto op_str = _2_operator_to_string.at(op);
		return json_value_t::make_array2({ json_value_t(op_str), lhs, rhs });
	}
	public: virtual const EXPRESSION maker__make(const eoperation op, const EXPRESSION& e1, const EXPRESSION& e2, const EXPRESSION& e3) const{
		if(op == eoperation::k_3_conditional_operator){
			return json_value_t::make_array2({ json_value_t("?:"), e1, e2, e3 });
		}
		else{
			QUARK_ASSERT(false);
		}
	}

	public: virtual const EXPRESSION maker__call(const EXPRESSION& f, const std::vector<EXPRESSION>& args) const{
		return json_value_t::make_array_skip_nulls({ json_value_t("call"), json_value_t(f), json_value_t(), args });
	}

	public: virtual const EXPRESSION maker__member_access(const EXPRESSION& address, const std::string& member_name) const{
		return json_value_t::make_array_skip_nulls({ json_value_t("->"), json_value_t(), address, json_value_t(member_name) });
	}

};

template<typename EXPRESSION>
const std::map<eoperation, string> json_helper<EXPRESSION>::_2_operator_to_string{
//	{ eoperation::k_2_member_access, "->" },

	{ eoperation::k_2_looup, "[-]" },

	{ eoperation::k_2_add, "+" },
	{ eoperation::k_2_subtract, "-" },
	{ eoperation::k_2_multiply, "*" },
	{ eoperation::k_2_divide, "/" },
	{ eoperation::k_2_remainder, "%" },

	{ eoperation::k_2_smaller_or_equal, "<=" },
	{ eoperation::k_2_smaller, "<" },
	{ eoperation::k_2_larger_or_equal, ">=" },
	{ eoperation::k_2_larger, ">" },

	{ eoperation::k_2_logical_equal, "==" },
	{ eoperation::k_2_logical_nonequal, "!=" },
	{ eoperation::k_2_logical_and, "&&" },
	{ eoperation::k_2_logical_or, "||" },
};



bool test__evaluate_single(const std::string& expression, const std::string& expected_value, const std::string& expected_seq){
	QUARK_TRACE_SS("input:" << expression);
	QUARK_TRACE_SS("expect:" << expected_value);

	json_helper<json_value_t> helper;
	const auto result = evaluate_single<json_value_t>(helper, seq_t(expression));
	const string json_s = json_to_compact_string(result.first);
	QUARK_TRACE_SS("result:" << json_s);
	if(json_s != expected_value){
		return false;
	}
	else if(result.second.get_all() != expected_seq){
		return false;
	}
	return true;
}

QUARK_UNIT_1("evaluate_single()", "identifier", test__evaluate_single(
	"123 xxx",
	R"(["k", "<int>", 123])",
	" xxx"
));

/*??? add support for floats
QUARK_UNIT_1("evaluate_single()", "identifier", test__evaluate_single(
	"123.4 xxx",
	R"(["k", "<float>", 123.4])",
	" xxx"
));
*/

QUARK_UNIT_1("evaluate_single()", "identifier", test__evaluate_single(
	"hello xxx",
	R"(["@", "hello"])",
	" xxx"
));

QUARK_UNIT_1("evaluate_single()", "identifier", test__evaluate_single(
	"\"world!\" xxx",
	R"(["k", "<string>", "world!"])",
	" xxx"
));


bool test__evaluate_expression(const std::string& expression, const std::string& expected_value, const std::string& expected_seq){
	QUARK_TRACE_SS("input:" << expression);
	QUARK_TRACE_SS("expect:" << expected_value);

	json_helper<json_value_t> helper;
	const auto result = evaluate_expression<json_value_t>(helper, seq_t(expression));
	const string json_s = json_to_compact_string(result.first);
	QUARK_TRACE_SS("result:" << json_s);
	if(json_s != expected_value){
		return false;
	}
	else if(result.second.get_all() != expected_seq){
		return false;
	}
	return true;
}

QUARK_UNIT_1("evaluate_expression()", "||", test__evaluate_expression(
	"1 || 0 || 1",
	R"(["||", ["||", ["k", "<int>", 1], ["k", "<int>", 0]], ["k", "<int>", 1]])",
	""
));




QUARK_UNIT_1("evaluate_expression()", "identifier", test__evaluate_expression(
	"hello xxx",
	R"(["@", "hello"])",
	" xxx"
));

QUARK_UNIT_1("evaluate_expression()", "struct member access", test__evaluate_expression(
	"hello.kitty xxx",
	R"(["->", ["@", "hello"], "kitty"])",
	" xxx"
));

QUARK_UNIT_1("evaluate_expression()", "struct member access", test__evaluate_expression(
	"hello.kitty.cat xxx",
	R"(["->", ["->", ["@", "hello"], "kitty"], "cat"])",
	" xxx"
));




QUARK_UNIT_1("evaluate_expression()", "function call", test__evaluate_expression(
	"f () xxx",
	R"(["call", ["@", "f"], []])", " xxx"
));

QUARK_UNIT_1("evaluate_single()", "function call, one simple arg", test__evaluate_expression(
	"f(3)",
	R"(["call", ["@", "f"], [["k", "<int>", 3]]])",
	""
));

QUARK_UNIT_1("evaluate_expression()", "call with expression-arg", test__evaluate_expression(
	"f (x + 10) xxx",
	R"(["call", ["@", "f"], [["+", ["@", "x"], ["k", "<int>", 10]]]])",
	" xxx"
));



QUARK_UNIT_1("evaluate_expression()", "function call", test__evaluate_expression(
	"poke.mon.f() xxx",
	R"(["call", ["->", ["->", ["@", "poke"], "mon"], "f"], []])", " xxx"
));



QUARK_UNIT_1("evaluate_expression()", "complex chain", test__evaluate_expression(
	"hello[\"troll\"].kitty[10].cat xxx",
	R"(["->", ["[-]", ["->", ["[-]", ["@", "hello"], ["k", "<string>", "troll"]], "kitty"], ["k", "<int>", 10]], "cat"])",
	" xxx"
));


QUARK_UNIT_1("evaluate_expression()", "chain", test__evaluate_expression(
	"poke.mon.v[10].a.b.c[\"three\"] xxx",
	R"(["[-]", ["->", ["->", ["->", ["[-]", ["->", ["->", ["@", "poke"], "mon"], "v"], ["k", "<int>", 10]], "a"], "b"], "c"], ["k", "<string>", "three"]])",
	" xxx"
));


QUARK_UNIT_1("evaluate_expression()", "function call with expression-args", test__evaluate_expression(
	"f(3 + 4, 4 * g(1000 + 2345), \"hello\", 5)",
	R"(["call", ["@", "f"], [["+", ["k", "<int>", 3], ["k", "<int>", 4]], ["*", ["k", "<int>", 4], ["call", ["@", "g"], [["+", ["k", "<int>", 1000], ["k", "<int>", 2345]]]]], ["k", "<string>", "hello"], ["k", "<int>", 5]]])",
	""
));




QUARK_UNIT_1("evaluate_expression()", "lookup with int", test__evaluate_expression(
	"hello[10] xxx",
	R"(["[-]", ["@", "hello"], ["k", "<int>", 10]])", " xxx"
));

QUARK_UNIT_1("evaluate_expression()", "lookup with string", test__evaluate_expression(
	"hello[\"troll\"] xxx",
	R"(["[-]", ["@", "hello"], ["k", "<string>", "troll"]])", " xxx"
));



//??? test member access etc.QUARK_UNIT_1("evaluate_single()", "f(3)", test_evaluate_single("f(3 + 4, 4 * g(\"hello\"), 5)", "JSON", ""));

/*
bool test__evaluate_expression(const std::string& expression, const std::string& expected_value, const std::string& expected_seq){
	my_helper<json_value_t> helper;
	const auto result = evaluate_expression2<json_value_t>(helper, seq_t(expression));
	const string json_s = json_to_compact_string(result.first);
	if(json_s != expected_value){
		return false;
	}
	else if(result.second.get_all() != expected_seq){
		return false;
	}
	return true;
}
*/



