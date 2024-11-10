#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <assert.h>
#include <cstdint>

#include <peglib.h>
#include <xtensor/xarray.hpp>
#include <xtensor/xio.hpp>
#include <xtensor/xindex_view.hpp>
#include <xtensor/xrandom.hpp>

#include "FormulaParser.h"

using namespace fparse;
using namespace peg;
using namespace std;

// 语法
const char* FormulaParser::grammar = R"(
		INPUT       <- EXPRESSION {no_ast_opt}
		EXPRESSION  <- ATOM (OPERATOR ATOM)* {
				 precedence
				   L ^
				   L & |
				   L + -
				   L * / %
				   L << >>
			   }
		ATOM        <- NUMBER / FUNCCALL / VAR / '(' EXPRESSION ')'
		FUNCCALL	<- FUNCNAME '(' ( EXPRESSION ( ',' EXPRESSION )* )? ')'	{ no_ast_opt }
		OPERATOR    <- < '+' | '-' | '*' | '/' | '%' | '^' | '&' | '|' | '>>' | '<<' >
		NUMBER      <- < '-'? [0-9]+ >
		FUNCNAME    <- < [a-zA-Z_] [0-9a-zA-Z_]* > & '('
		VAR			<- < [a-zA-Z_] [0-9a-zA-Z_]* > ! '('
		%whitespace <- [ \t\n\r]* ( ( ('//' [^\n\r]* [\n\r]*) / ('/*' (!'*/' .)* '*/') ) [ \t\n\r]* )*
    )";

const EvaluationResult FormulaParser::sine_table({
		127, 130, 133, 136, 139, 143, 146, 149, 152, 155, 158, 161, 164,
	   167, 170, 173, 176, 179, 182, 184, 187, 190, 193, 195, 198, 200,
	   203, 205, 208, 210, 213, 215, 217, 219, 221, 224, 226, 228, 229,
	   231, 233, 235, 236, 238, 239, 241, 242, 244, 245, 246, 247, 248,
	   249, 250, 251, 251, 252, 253, 253, 254, 254, 254, 254, 254, 255,
	   254, 254, 254, 254, 254, 253, 253, 252, 251, 251, 250, 249, 248,
	   247, 246, 245, 244, 242, 241, 239, 238, 236, 235, 233, 231, 229,
	   228, 226, 224, 221, 219, 217, 215, 213, 210, 208, 205, 203, 200,
	   198, 195, 193, 190, 187, 184, 182, 179, 176, 173, 170, 167, 164,
	   161, 158, 155, 152, 149, 146, 143, 139, 136, 133, 130, 127, 124,
	   121, 118, 115, 111, 108, 105, 102,  99,  96,  93,  90,  87,  84,
		81,  78,  75,  72,  70,  67,  64,  61,  59,  56,  54,  51,  49,
		46,  44,  41,  39,  37,  35,  33,  30,  28,  26,  25,  23,  21,
		19,  18,  16,  15,  13,  12,  10,   9,   8,   7,   6,   5,   4,
		 3,   3,   2,   1,   1,   0,   0,   0,   0,   0,   0,   0,   0,
		 0,   0,   0,   1,   1,   2,   3,   3,   4,   5,   6,   7,   8,
		 9,  10,  12,  13,  15,  16,  18,  19,  21,  23,  25,  26,  28,
		30,  33,  35,  37,  39,  41,  44,  46,  49,  51,  54,  56,  59,
		61,  64,  67,  70,  72,  75,  78,  81,  84,  87,  90,  93,  96,
		99, 102, 105, 108, 111, 115, 118, 121, 124 });

const EvaluationResult FormulaParser::triangle_table({
		127, 129, 131, 133, 135, 137, 139, 141, 143, 145, 147, 149, 151,
	   153, 155, 157, 159, 161, 163, 165, 167, 169, 171, 173, 175, 177,
	   179, 181, 183, 185, 187, 189, 191, 193, 195, 197, 199, 201, 203,
	   205, 207, 209, 211, 213, 215, 217, 219, 221, 223, 225, 227, 229,
	   231, 233, 235, 237, 239, 241, 243, 245, 247, 249, 251, 253, 255,
	   253, 251, 249, 247, 245, 243, 241, 239, 237, 235, 233, 231, 229,
	   227, 225, 223, 221, 219, 217, 215, 213, 211, 209, 207, 205, 203,
	   201, 199, 197, 195, 193, 191, 189, 187, 185, 183, 181, 179, 177,
	   175, 173, 171, 169, 167, 165, 163, 161, 159, 157, 155, 153, 151,
	   149, 147, 145, 143, 141, 139, 137, 135, 133, 131, 129, 127, 125,
	   123, 121, 119, 117, 115, 113, 111, 109, 107, 105, 103, 101,  99,
		97,  95,  93,  91,  89,  87,  85,  83,  81,  79,  77,  75,  73,
		71,  69,  67,  65,  63,  61,  59,  57,  55,  53,  51,  49,  47,
		45,  43,  41,  39,  37,  35,  33,  31,  29,  27,  25,  23,  21,
		19,  17,  15,  13,  11,   9,   7,   5,   3,   1,   0,   1,   3,
		 5,   7,   9,  11,  13,  15,  17,  19,  21,  23,  25,  27,  29,
		31,  33,  35,  37,  39,  41,  43,  45,  47,  49,  51,  53,  55,
		57,  59,  61,  63,  65,  67,  69,  71,  73,  75,  77,  79,  81,
		83,  85,  87,  89,  91,  93,  95,  97,  99, 101, 103, 105, 107,
	   109, 111, 113, 115, 117, 119, 121, 123, 125 });

// 合法变量名及常数化简求值用的暂时变量值
unordered_map<string, EvaluationResult> FormulaParser::temp_vars = {
	{"T", EvaluationResult(0)},
	{"t", EvaluationResult(0)},
	{"w", EvaluationResult(0)},
	{"x", EvaluationResult(0)},
	{"y", EvaluationResult(0)},
	{"z", EvaluationResult(0)},
	//{"env1", EvaluationResult(0)},
	//{"env2", EvaluationResult(0)},
	//{"env3", EvaluationResult(0)},
	//{"env4", EvaluationResult(0)}
};

// 合法的函数名及实现
unordered_map<string, FunctionWithBound> FormulaParser::function_dictionary = {
	{
		"sin",
		{ [](const vector<shared_ptr<Expression>>& args, const unordered_map<string, EvaluationResult>& vars, size_t block_size) -> EvaluationResult {
			return xt::index_view(FormulaParser::sine_table, (args[0]->evaluate(vars, block_size) % 256 + 256) % 256);
		}, 1 , 1}
	},
	{
		"cos",
		{ [](const vector<shared_ptr<Expression>>& args, const unordered_map<string, EvaluationResult>& vars, size_t block_size) -> EvaluationResult {
			return xt::index_view(FormulaParser::sine_table, ((args[0]->evaluate(vars, block_size) + 64) % 256 + 256) % 256);
		}, 1 , 1}
	},
	{
		"tri",
		{ [](const vector<shared_ptr<Expression>>& args, const unordered_map<string, EvaluationResult>& vars, size_t block_size) -> EvaluationResult {
			return xt::index_view(FormulaParser::triangle_table, (args[0]->evaluate(vars, block_size) % 256 + 256) % 256);
		}, 1 , 1}
	},
	{
		"rand",
		{ [](const vector<shared_ptr<Expression>>& args, const unordered_map<string, EvaluationResult>& vars, size_t block_size) -> EvaluationResult {
			return xt::random::randint({block_size}, 0, 255);
		}, 0 , 0}
	},
	{
		"abs",
		{ [](const vector<shared_ptr<Expression>>& args, const unordered_map<string, EvaluationResult>& vars, size_t block_size) -> EvaluationResult {
			return xt::abs(args[0]->evaluate(vars, block_size));
		}, 1 , 1}
	},
	{
		"srand",
		{ [](const vector<shared_ptr<Expression>>& args, const unordered_map<string, EvaluationResult>& vars, size_t block_size) -> EvaluationResult {
			EvaluationResult r = args[0]->evaluate(vars, block_size);
			r = ((r + 3463) * 2971);
			r = r ^ (r << 13);
			r = r ^ (r >> 17);
			return r ^ (r << 5);
		}, 1 , 1}
	}
};

// 变量类
Variable::Variable(const string& name) : name(name) {};

string Variable::toString() const { return name; }

EvaluationResult Variable::evaluate(const unordered_map<string, EvaluationResult>& vars, size_t block_size) const {	// evaluation
	// vars 应存放 broadcast 后的向量
	return vars.at(name);
}


// 常量类
Constant::Constant(int32_t value) : value(value) {};

string Constant::toString() const { return to_string(value); }

EvaluationResult Constant::evaluate(const unordered_map<string, EvaluationResult>&, size_t block_size) const {			// evaluation
	return xt::broadcast(value, { block_size });
}


// 二元表达式类
CompoundExpression::CompoundExpression(Operation op, shared_ptr<Expression> lhs, shared_ptr<Expression> rhs)
	: operation(op), l(lhs), r(rhs) {}

string CompoundExpression::toString() const {
	string opStr;
	switch (operation) {
	case Operation::ADD: opStr = "+"; break;
	case Operation::SUBTRACT: opStr = "-"; break;
	case Operation::MULTIPLY: opStr = "*"; break;
	case Operation::DIVIDE: opStr = "/"; break;
	case Operation::MOD: opStr = "%"; break;
	case Operation::AND: opStr = "&"; break;
	case Operation::OR: opStr = "|"; break;
	case Operation::XOR: opStr = "^"; break;
	case Operation::SHIFT_LEFT: opStr = "<<"; break;
	case Operation::SHIFT_RIGHT: opStr = ">>"; break;
	default: opStr = "?"; break;
	}
	return "(" + l->toString() + " " + opStr + " " + r->toString() + ")";
}

EvaluationResult CompoundExpression::evaluate(const unordered_map<string, EvaluationResult>& vars, size_t block_size) const {
	EvaluationResult leftValue = l->evaluate(vars, block_size);		// l operand
	EvaluationResult rightValue = r->evaluate(vars, block_size);	// r operand

	switch (operation) {
	case Operation::ADD: return leftValue + rightValue;
	case Operation::SUBTRACT: return leftValue - rightValue;
	case Operation::MULTIPLY: return leftValue * rightValue;
	case Operation::DIVIDE:
		return xt::where(xt::equal(rightValue, 0), 0, leftValue / rightValue);
	case Operation::MOD:
		return xt::where(xt::equal(rightValue, 0), 0, leftValue % rightValue);
	case Operation::AND: return leftValue & rightValue;
	case Operation::OR: return leftValue | rightValue;
	case Operation::XOR: return leftValue ^ rightValue;
	case Operation::SHIFT_LEFT: return leftValue << (rightValue % 16);
	case Operation::SHIFT_RIGHT: return leftValue >> (rightValue % 16);
	default: throw invalid_argument("Invalid operation"); // invalid operation
	}
}


// 函数表达式类
FunctionExpression::FunctionExpression(string function_name, vector<shared_ptr<Expression>> function_args)
	:name(function_name), function(FormulaParser::function_dictionary.at(function_name)), args(function_args) {
}

string FunctionExpression::toString() const {
	string result_str = name + "(";
	for (shared_ptr<Expression> expr : args)
		result_str += expr->toString() + ",";
	return result_str + ")";
}

EvaluationResult FunctionExpression::evaluate(const unordered_map<string, EvaluationResult>& vars, size_t block_size) const {
	return function.function(args, vars, block_size);
}


// +
shared_ptr<Expression> operator+(shared_ptr<Expression> lhs, shared_ptr<Expression> rhs) {
	// Constant simplify
	if (lhs->isConstant() && rhs->isConstant()) {
		auto l = dynamic_pointer_cast<Constant>(lhs);
		auto r = dynamic_pointer_cast<Constant>(rhs);
		return make_shared<Constant>(l->value + r->value);
	} // preprocess 0
	else if (lhs->isConstant() && dynamic_pointer_cast<Constant>(lhs)->value == 0)
		return rhs;
	else if (rhs->isConstant() && dynamic_pointer_cast<Constant>(rhs)->value == 0)
		return lhs;
	// build new expression object
	return make_shared<CompoundExpression>(Operation::ADD, lhs, rhs);
}
// -
shared_ptr<Expression> operator-(shared_ptr<Expression> lhs, shared_ptr<Expression> rhs) {
	if (lhs->isConstant() && rhs->isConstant()) {
		auto l = dynamic_pointer_cast<Constant>(lhs);
		auto r = dynamic_pointer_cast<Constant>(rhs);
		return make_shared<Constant>(l->value - r->value);
	}
	else if (rhs->isConstant() && dynamic_pointer_cast<Constant>(rhs)->value == 0)
		return lhs;

	return make_shared<CompoundExpression>(Operation::SUBTRACT, lhs, rhs);
}
// *
shared_ptr<Expression> operator*(shared_ptr<Expression> lhs, shared_ptr<Expression> rhs) {
	if (lhs->isConstant() && rhs->isConstant()) {
		auto l = dynamic_pointer_cast<Constant>(lhs);
		auto r = dynamic_pointer_cast<Constant>(rhs);
		return make_shared<Constant>(l->value * r->value);
	}
	else if (lhs->isConstant() && dynamic_pointer_cast<Constant>(lhs)->value == 0)
		return make_shared<Constant>(0);
	else if (rhs->isConstant() && dynamic_pointer_cast<Constant>(rhs)->value == 0)
		return make_shared<Constant>(0);

	return make_shared<CompoundExpression>(Operation::MULTIPLY, lhs, rhs);
}
// /
shared_ptr<Expression> operator/(shared_ptr<Expression> lhs, shared_ptr<Expression> rhs) {
	if (lhs->isConstant() && rhs->isConstant()) {
		auto l = dynamic_pointer_cast<Constant>(lhs);
		auto r = dynamic_pointer_cast<Constant>(rhs);
		if (r->value == 0)
			return make_shared<Constant>(0);
		return make_shared<Constant>(l->value / r->value);
	}
	else if (lhs->isConstant() && dynamic_pointer_cast<Constant>(lhs)->value == 0)
		return make_shared<Constant>(0);
	else if (rhs->isConstant() && dynamic_pointer_cast<Constant>(rhs)->value == 0)
		return make_shared<Constant>(0);

	return make_shared<CompoundExpression>(Operation::DIVIDE, lhs, rhs);
}
// %
shared_ptr<Expression> operator%(shared_ptr<Expression> lhs, shared_ptr<Expression> rhs) {
	if (lhs->isConstant() && rhs->isConstant()) {
		auto l = dynamic_pointer_cast<Constant>(lhs);
		auto r = dynamic_pointer_cast<Constant>(rhs);
		if (r->value == 0)
			return make_shared<Constant>(0);
		return make_shared<Constant>(l->value % r->value);
	}
	else if (lhs->isConstant() && dynamic_pointer_cast<Constant>(lhs)->value == 0)
		return make_shared<Constant>(0);
	else if (rhs->isConstant() && dynamic_pointer_cast<Constant>(rhs)->value == 0)
		return make_shared<Constant>(0);

	return make_shared<CompoundExpression>(Operation::MOD, lhs, rhs);
}
// &
shared_ptr<Expression> operator&(shared_ptr<Expression> lhs, shared_ptr<Expression> rhs) {
	if (lhs->isConstant() && rhs->isConstant()) {
		auto l = dynamic_pointer_cast<Constant>(lhs);
		auto r = dynamic_pointer_cast<Constant>(rhs);
		return make_shared<Constant>(l->value & r->value);
	}
	else if (lhs->isConstant() && dynamic_pointer_cast<Constant>(lhs)->value == 0)
		return make_shared<Constant>(0);
	else if (rhs->isConstant() && dynamic_pointer_cast<Constant>(rhs)->value == 0)
		return make_shared<Constant>(0);

	return make_shared<CompoundExpression>(Operation::AND, lhs, rhs);
}
// |
shared_ptr<Expression> operator|(shared_ptr<Expression> lhs, shared_ptr<Expression> rhs) {
	if (lhs->isConstant() && rhs->isConstant()) {
		auto l = dynamic_pointer_cast<Constant>(lhs);
		auto r = dynamic_pointer_cast<Constant>(rhs);
		return make_shared<Constant>(l->value | r->value);
	}
	else if (lhs->isConstant() && dynamic_pointer_cast<Constant>(lhs)->value == 0)
		return rhs;
	else if (rhs->isConstant() && dynamic_pointer_cast<Constant>(rhs)->value == 0)
		return lhs;

	return make_shared<CompoundExpression>(Operation::OR, lhs, rhs);
}
// ^
shared_ptr<Expression> operator^(shared_ptr<Expression> lhs, shared_ptr<Expression> rhs) {
	if (lhs->isConstant() && rhs->isConstant()) {
		auto l = dynamic_pointer_cast<Constant>(lhs);
		auto r = dynamic_pointer_cast<Constant>(rhs);
		return make_shared<Constant>(l->value ^ r->value);
	}
	else if (lhs->isConstant() && dynamic_pointer_cast<Constant>(lhs)->value == 0)
		return rhs;
	else if (rhs->isConstant() && dynamic_pointer_cast<Constant>(rhs)->value == 0)
		return lhs;

	return make_shared<CompoundExpression>(Operation::XOR, lhs, rhs);
}
// <<
shared_ptr<Expression> operator<<(shared_ptr<Expression> lhs, shared_ptr<Expression> rhs) {
	if (lhs->isConstant() && rhs->isConstant()) {
		auto l = dynamic_pointer_cast<Constant>(lhs);
		auto r = dynamic_pointer_cast<Constant>(rhs);
		return make_shared<Constant>(l->value << (r->value % 16));
	}
	else if (lhs->isConstant() && dynamic_pointer_cast<Constant>(lhs)->value == 0)
		return make_shared<Constant>(0);
	else if (rhs->isConstant() && dynamic_pointer_cast<Constant>(rhs)->value % 16 == 0)
		return lhs;

	return make_shared<CompoundExpression>(Operation::SHIFT_LEFT, lhs, rhs);
}
// >>
shared_ptr<Expression> operator>>(shared_ptr<Expression> lhs, shared_ptr<Expression> rhs) {
	if (lhs->isConstant() && rhs->isConstant()) {
		auto l = dynamic_pointer_cast<Constant>(lhs);
		auto r = dynamic_pointer_cast<Constant>(rhs);
		return make_shared<Constant>(l->value >> (r->value % 16));
	}
	else if (lhs->isConstant() && dynamic_pointer_cast<Constant>(lhs)->value == 0)
		return make_shared<Constant>(0);
	else if (rhs->isConstant() && dynamic_pointer_cast<Constant>(rhs)->value % 16 == 0)
		return lhs;

	return make_shared<CompoundExpression>(Operation::SHIFT_RIGHT, lhs, rhs);
}

shared_ptr<Expression> castToExpression(const any& value) {
	if (value.type() == typeid(shared_ptr<Constant>))
		return any_cast<shared_ptr<Constant>>(value);

	if (value.type() == typeid(shared_ptr<Variable>))
		return any_cast<shared_ptr<Variable>>(value);

	if (value.type() == typeid(shared_ptr<CompoundExpression>))
		return any_cast<shared_ptr<CompoundExpression>>(value);

	if (value.type() == typeid(shared_ptr<FunctionExpression>))
		return any_cast<shared_ptr<FunctionExpression>>(value);

	if (value.type() == typeid(shared_ptr<Expression>))
		return any_cast<shared_ptr<Expression>>(value);

	cout << value.type().name() << ";" << endl;
	throw bad_any_cast();
}


// 解析器类
FormulaParser::FormulaParser() : parser(grammar) {
	assert(static_cast<bool>(parser) == true);		// debug

	// INPUT pattern
	parser["INPUT"] = [](const SemanticValues& vs) {
		return castToExpression(vs[0]);
		};

	// EXPRESSION pattern
	parser["EXPRESSION"] = [](const SemanticValues& vs) {
		auto result = castToExpression(vs[0]);
		if (vs.size() > 1) {
			auto ope = any_cast<char>(vs[1]);
			auto expr = castToExpression(vs[2]);
			switch (ope) {
			case '+': result = result + expr; break;
			case '-': result = result - expr; break;
			case '*': result = result * expr; break;
			case '/': result = result / expr; break;
			case '%': result = result % expr; break;
			case '&': result = result & expr; break;
			case '|': result = result | expr; break;
			case '^': result = result ^ expr; break;
			case '<': result = result << expr; break;
			case '>': result = result >> expr; break;
			}
		}
		return result;
		};

	// FUNCCALL pattern
	parser["FUNCCALL"] = [](const SemanticValues& vs) -> shared_ptr<Expression> {
		auto name = any_cast<string>(vs[0]);

		vector<shared_ptr<Expression>> args;

		if (vs.size() > 1) {
			bool constant_flag = true;

			for (size_t i = 1; i < vs.size(); i++) {
				shared_ptr<Expression> expr = castToExpression(vs[i]);
				// 添加参数
				args.push_back(expr);
				// 检查函数的所有参数是否均为常数
				if (!expr->isConstant()) constant_flag = false;
			}

			// 如果所有参数均为常数
			if (constant_flag) {
				std::unordered_map<std::string, EvaluationResult> empty_map;	// 空的 unordered_map, 不占用实际空间
				FunctionType function = FormulaParser::function_dictionary.at(name).function;
				return make_shared<Constant>(function(args, empty_map, 1)[0]);
			}
		}

		return make_shared<FunctionExpression>(name, args);
		};

	parser["FUNCCALL"].predicate = [](const SemanticValues& vs, const any&, string& msg) {
		auto name = any_cast<string>(vs[0]);

		// 检查函数的参数量是否合适
		FunctionWithBound function = FormulaParser::function_dictionary.at(name);

		if ((function.lower_bound >= 0 && vs.size() - 1 < function.lower_bound) || (function.upper_bound >= 0 && vs.size() - 1 > function.upper_bound)) {
			msg = "The " + name + " function has an inappropriate number of parameters: " + to_string(vs.size() - 1) + ".";
			return false;
		}
		return true;
		};

	// OPERATOR token
	parser["OPERATOR"] = [](const SemanticValues& vs) {
		return vs.token_to_string()[0];
		};

	// NUMBER token
	parser["NUMBER"] = [](const SemanticValues& vs) {
		return make_shared<Constant>(vs.token_to_number<int32_t>());
		};

	// VAR token
	parser["VAR"] = [](const SemanticValues& vs) {
		return make_shared<Variable>(vs.token_to_string());
		};

	parser["VAR"].predicate = [](const SemanticValues& vs, const any&, string& msg) {
		// 检查是否存在该名字的函数
		auto name = any_cast<string>(vs.token_to_string());

		if (FormulaParser::temp_vars.count(name) == 0) {
			msg = "Unknown variable " + name + ".";
			return false;
		};
		return true;
		};

	// FUNCNAME token
	parser["FUNCNAME"] = [](const SemanticValues& vs) {
		return vs.token_to_string();
		};

	parser["FUNCNAME"].predicate = [](const SemanticValues& vs, const any&, string& msg) {
		// 检查是否存在该名字的函数
		auto name = any_cast<string>(vs.token_to_string());

		if (FormulaParser::function_dictionary.count(name) == 0) {
			msg = "Unknown function " + name + ".";
			return false;
		};
		return true;
		};

	parser.enable_packrat_parsing();
};

ParseResult FormulaParser::parse(string& input) noexcept {

	ParseResult result = { false, nullptr, 0, 0, "", "" };

	parser.set_logger([&result](size_t line, size_t col, const string& msg, const string& rule) {
		result = { false, nullptr, line,  col, msg, rule };
		});

	shared_ptr<Expression> expr;

	try {
		bool parse_success = parser.parse(input, expr);		// logger 在此处被调用
		if (parse_success)
			result = { true, expr, 0,  0, "", "" };			// 解析错误不会作为异常被抛出
	}
	catch (const std::exception& e) {						// 标准异常
		result = { false, nullptr, 0,  0, e.what(), "" };
	}
	catch (...) {											// 未知的潜在异常
		result = { false, nullptr, 0,  0, "Unknown Exception", "" };
	}

	return result;
};