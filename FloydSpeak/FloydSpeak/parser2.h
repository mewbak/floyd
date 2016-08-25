//
//  parser2.h
//  FloydSpeak
//
//  Created by Marcus Zetterquist on 25/08/16.
//  Copyright © 2016 Marcus Zetterquist. All rights reserved.
//

#ifndef parser2_h
#define parser2_h


#include "quark.h"

#include <string>
#include <memory>


/*
	C99-language constants.
*/
const std::string k_c99_number_chars = "0123456789.";
const std::string k_c99_whitespace_chars = " \n\t\r";
	const std::string k_identifier_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";

//	Precedence same as C99.
enum class eoperator_precedence {
	k_super_strong = 0,

	//	(xyz)
	k_parentesis = 0,

	//	a()
	k_function_call = 2,

	k_multiply_divider_remainder = 5,

	k_add_sub = 6,

	//	<   <=	For relational operators < and ≤ respectively
	//	>   >=
	k_larger_smaller = 8,


	k_equal__not_equal = 9,

	k_logical_and = 13,
	k_logical_or = 14,

	k_comparison_operator = 15,

	k_super_weak
};

enum class eoperation {
	k_0_number_constant = 100,
	k_0_identifer,
	k_0_string_literal,

	k_2_add,
	k_2_subtract,
	k_2_multiply,
	k_2_divide,
	k_2_remainder,

	k_2_smaller_or_equal,
	k_2_smaller,

	k_2_larger_or_equal,
	k_2_larger,

	k_2_logical_equal,
	k_2_logical_nonequal,
	k_2_logical_and,
	k_2_logical_or,

	k_3_conditional_operator,

	k_n_call,

	k_1_logical_not
};



template<typename EXPRESSION> struct maker {
	public: virtual ~maker(){};
	public: virtual const EXPRESSION maker__on_string(const eoperation op, const std::string& s) const = 0;
	public: virtual const EXPRESSION maker__make(const eoperation op, const EXPRESSION& expr) const = 0;
	public: virtual const EXPRESSION maker__make(const eoperation op, const EXPRESSION& lhs, const EXPRESSION& rhs) const = 0;
	public: virtual const EXPRESSION maker__make(const eoperation op, const EXPRESSION& e1, const EXPRESSION& e2, const EXPRESSION& e3) const = 0;
	public: virtual const EXPRESSION maker__make(const eoperation op, const std::vector<const EXPRESSION>& args) const = 0;
};


//??? Add support for member variable/function/lookup paths.
template<typename EXPRESSION>
std::pair<EXPRESSION, seq_t> evaluate_expression(const maker<EXPRESSION>& helper, const seq_t& p, const eoperator_precedence precedence);



seq_t skip_whitespace(const seq_t& p);

std::pair<std::string, seq_t> parse_string_literal(const seq_t& p);



template<typename EXPRESSION>
std::pair<EXPRESSION, seq_t> evaluate_single(const maker<EXPRESSION>& helper, const seq_t& p) {
	QUARK_ASSERT(p.check_invariant());

	if(p.first() == "\""){
		const auto s = read_while_not(p.rest(), "\"");
		const EXPRESSION result = helper.maker__on_string(eoperation::k_0_string_literal, s.first);
		return { result, s.second.rest() };
	}

	{
		const auto number_s = read_while(p, k_c99_number_chars);
		if(!number_s.first.empty()){
			const EXPRESSION result = helper.maker__on_string(eoperation::k_0_number_constant, number_s.first);
			return { result, number_s.second };
		}
	}

	{
		const auto identifier_s = read_while(p, k_identifier_chars);
		if(!identifier_s.first.empty()){
			const EXPRESSION result = helper.maker__on_string(eoperation::k_0_identifer, identifier_s.first);
			return { result, identifier_s.second };
		}
	}


	//??? Identifiers, string constants, true/false.


	QUARK_ASSERT(false);
}

template<typename EXPRESSION>
std::pair<EXPRESSION, seq_t> evaluate_atom(const maker<EXPRESSION>& helper, const seq_t& p){
	QUARK_ASSERT(p.check_invariant());

    const auto p2 = skip_whitespace(p);
	if(p2.empty()){
		throw std::runtime_error("Unexpected end of string");
	}

	const char ch1 = p2.first_char();

	//	"-xxx"
	if(ch1 == '-'){
		const auto a = evaluate_expression(helper, p2.rest(), eoperator_precedence::k_super_strong);
		const auto value2 = helper.maker__make(eoperation::k_1_logical_not, a.first);
		return { value2, skip_whitespace(a.second) };
	}
	//	"(yyy)xxx"
	else if(ch1 == '('){
		const auto a = evaluate_expression(helper, p2.rest(), eoperator_precedence::k_super_weak);
		if (a.second.first(1) != ")"){
			throw std::runtime_error("Expected ')'");
		}
		return { a.first, skip_whitespace(a.second.rest()) };
	}

	//	":yyy xxx"
	else if(ch1 == ':'){
		const auto a = evaluate_expression(helper, p2.rest(), eoperator_precedence::k_super_weak);
		return { a.first, skip_whitespace(a.second.rest()) };
	}

	//	"1234xxx" or "my_function(3)xxx"
	else {
		const auto a = evaluate_single(helper, p2);
		return { a.first, skip_whitespace(a.second) };
	}
}

template<typename EXPRESSION>
std::pair<EXPRESSION, seq_t> evaluate_operation(const maker<EXPRESSION>& helper, const seq_t& p1, const EXPRESSION& lhs, const eoperator_precedence precedence){
	QUARK_ASSERT(p1.check_invariant());

	const auto p = p1;
	if(!p.empty()){
		const char op1 = p.first_char();
		const std::string op2 = p.first(2);

		//	Ending parantesis
		if(op1 == ')' && precedence > eoperator_precedence::k_parentesis){
			return { lhs, p };
		}


		//	EXPRESSION + EXPRESSION +
		else if(op1 == '+'  && precedence > eoperator_precedence::k_add_sub){
			const auto rhs = evaluate_expression(helper, p.rest(), eoperator_precedence::k_add_sub);
			const auto value2 = helper.maker__make(eoperation::k_2_add, lhs, rhs.first);
			return evaluate_operation(helper, rhs.second, value2, precedence);
		}

		//	EXPRESSION - EXPRESSION -
		else if(op1 == '-' && precedence > eoperator_precedence::k_add_sub){
			const auto rhs = evaluate_expression(helper, p.rest(), eoperator_precedence::k_add_sub);
			const auto value2 = helper.maker__make(eoperation::k_2_subtract, lhs, rhs.first);
			return evaluate_operation(helper, rhs.second, value2, precedence);
		}

		//	EXPRESSION * EXPRESSION *
		else if(op1 == '*' && precedence > eoperator_precedence::k_multiply_divider_remainder) {
			const auto rhs = evaluate_expression(helper, p.rest(), eoperator_precedence::k_multiply_divider_remainder);
			const auto value2 = helper.maker__make(eoperation::k_2_multiply, lhs, rhs.first);
			return evaluate_operation(helper, rhs.second, value2, precedence);
		}
		//	EXPRESSION / EXPRESSION /
		else if(op1 == '/' && precedence > eoperator_precedence::k_multiply_divider_remainder) {
			const auto rhs = evaluate_expression(helper, p.rest(), eoperator_precedence::k_multiply_divider_remainder);
			const auto value2 = helper.maker__make(eoperation::k_2_divide, lhs, rhs.first);
			return evaluate_operation(helper, rhs.second, value2, precedence);
		}

		//	EXPRESSION % EXPRESSION %
		else if(op1 == '%' && precedence > eoperator_precedence::k_multiply_divider_remainder) {
			const auto rhs = evaluate_expression(helper, p.rest(), eoperator_precedence::k_multiply_divider_remainder);
			const auto value2 = helper.maker__make(eoperation::k_2_remainder, lhs, rhs.first);
			return evaluate_operation(helper, rhs.second, value2, precedence);
		}


		//	EXPRESSION ? EXPRESSION : EXPRESSION
		else if(op1 == '?' && precedence > eoperator_precedence::k_comparison_operator) {
			const auto true_expr_p = evaluate_expression(helper, p.rest(), eoperator_precedence::k_comparison_operator);

			const auto colon = true_expr_p.second.first(1);
			if(colon != ":"){
				throw std::runtime_error("Expected ':'");
			}

			const auto false_expr_p = evaluate_expression(helper, true_expr_p.second.rest(), precedence);
			const auto value2 = helper.maker__make(eoperation::k_3_conditional_operator, lhs, true_expr_p.first, false_expr_p.first);

			//	End this precedence level.
			return { value2, false_expr_p.second.rest() };
		}


		//	EXPRESSION == EXPRESSION
		else if(op2 == "==" && precedence > eoperator_precedence::k_equal__not_equal){
			const auto rhs = evaluate_expression(helper, p.rest(2), eoperator_precedence::k_equal__not_equal);
			const auto value2 = helper.maker__make(eoperation::k_2_logical_equal, lhs, rhs.first);

			//	End this precedence level.
			return { value2, rhs.second.rest() };
		}
		//	EXPRESSION != EXPRESSION
		else if(op2 == "!=" && precedence > eoperator_precedence::k_equal__not_equal){
			const auto rhs = evaluate_expression(helper, p.rest(2), eoperator_precedence::k_equal__not_equal);
			const auto value2 = helper.maker__make(eoperation::k_2_logical_nonequal, lhs, rhs.first);

			//	End this precedence level.
			return { value2, rhs.second.rest() };
		}

		//	!!! Check for "<=" before we check for "<".
		//	EXPRESSION <= EXPRESSION
		else if(op2 == "<=" && precedence > eoperator_precedence::k_larger_smaller){
			const auto rhs = evaluate_expression(helper, p.rest(2), eoperator_precedence::k_larger_smaller);
			const auto value2 = helper.maker__make(eoperation::k_2_smaller_or_equal, lhs, rhs.first);

			//	End this precedence level.
			return { value2, rhs.second.rest() };
		}

		//	EXPRESSION < EXPRESSION
		else if(op1 == '<' && precedence > eoperator_precedence::k_larger_smaller){
			const auto rhs = evaluate_expression(helper, p.rest(2), eoperator_precedence::k_larger_smaller);
			const auto value2 = helper.maker__make(eoperation::k_2_smaller, lhs, rhs.first);

			//	End this precedence level.
			return { value2, rhs.second.rest() };
		}


		//	!!! Check for ">=" before we check for ">".
		//	EXPRESSION >= EXPRESSION
		else if(op2 == ">=" && precedence > eoperator_precedence::k_larger_smaller){
			const auto rhs = evaluate_expression(helper, p.rest(2), eoperator_precedence::k_larger_smaller);
			const auto value2 = helper.maker__make(eoperation::k_2_larger_or_equal, lhs, rhs.first);

			//	End this precedence level.
			return { value2, rhs.second.rest() };
		}

		//	EXPRESSION > EXPRESSION
		else if(op1 == '>' && precedence > eoperator_precedence::k_larger_smaller){
			const auto rhs = evaluate_expression(helper, p.rest(2), eoperator_precedence::k_larger_smaller);
			const auto value2 = helper.maker__make(eoperation::k_2_larger, lhs, rhs.first);

			//	End this precedence level.
			return { value2, rhs.second.rest() };
		}


		//	EXPRESSION && EXPRESSION
		else if(op2 == "&&" && precedence > eoperator_precedence::k_logical_and){
			const auto rhs = evaluate_expression(helper, p.rest(2), eoperator_precedence::k_logical_and);
			const auto value2 = helper.maker__make(eoperation::k_2_logical_and, lhs, rhs.first);
			return evaluate_operation(helper, rhs.second, value2, precedence);
		}

		//	EXPRESSION || EXPRESSION
		else if(op2 == "||" && precedence > eoperator_precedence::k_logical_or){
			const auto rhs = evaluate_expression(helper, p.rest(2), eoperator_precedence::k_logical_or);
			const auto value2 = helper.maker__make(eoperation::k_2_logical_or, lhs, rhs.first);
			return evaluate_operation(helper, rhs.second, value2, precedence);
		}

		else{
			return { lhs, p };
		}
	}
	else{
		return { lhs, p };
	}
}

template<typename EXPRESSION>
std::pair<EXPRESSION, seq_t> evaluate_expression(const maker<EXPRESSION>& helper, const seq_t& p, const eoperator_precedence precedence){
	QUARK_ASSERT(p.check_invariant());

	auto a = evaluate_atom(helper, p);
	return evaluate_operation<EXPRESSION>(helper, a.second, a.first, precedence);
}


template<typename EXPRESSION>
std::pair<EXPRESSION, seq_t> evaluate_expression2(const maker<EXPRESSION>& helper, const seq_t& p){
	QUARK_ASSERT(p.check_invariant());

	auto a = evaluate_atom(helper, p);
	return evaluate_operation<EXPRESSION>(helper, a.second, a.first, eoperator_precedence::k_super_weak);
}


#endif /* parser2_h */
