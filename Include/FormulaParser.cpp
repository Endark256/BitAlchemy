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
		128, 131, 134, 137, 141, 144, 147, 150, 153, 156, 159, 162, 165,
		168, 171, 174, 177, 180, 183, 186, 188, 191, 194, 196, 199, 202,
		204, 207, 209, 212, 214, 216, 219, 221, 223, 225, 227, 229, 231,
		233, 234, 236, 238, 239, 241, 242, 244, 245, 246, 247, 249, 250,
		250, 251, 252, 253, 254, 254, 255, 255, 255, 256, 256, 256, 256,
		256, 256, 256, 255, 255, 255, 254, 254, 253, 252, 251, 250, 250,
		249, 247, 246, 245, 244, 242, 241, 239, 238, 236, 234, 233, 231,
		229, 227, 225, 223, 221, 219, 216, 214, 212, 209, 207, 204, 202,
		199, 196, 194, 191, 188, 186, 183, 180, 177, 174, 171, 168, 165,
		162, 159, 156, 153, 150, 147, 144, 141, 137, 134, 131, 128, 125,
		122, 119, 115, 112, 109, 106, 103, 100,  97,  94,  91,  88,  85,
		82,  79,  76,  73,  70,  68,  65,  62,  60,  57,  54,  52,  49,
		47,  44,  42,  40,  37,  35,  33,  31,  29,  27,  25,  23,  22,
		20,  18,  17,  15,  14,  12,  11,  10,   9,   7,   6,   6,   5,
		4,   3,   2,   2,   1,   1,   1,   0,   0,   0,   0,   0,   0,
		0,   1,   1,   1,   2,   2,   3,   4,   5,   6,   6,   7,   9,
		10,  11,  12,  14,  15,  17,  18,  20,  22,  23,  25,  27,  29,
		31,  33,  35,  37,  40,  42,  44,  47,  49,  52,  54,  57,  60,
		62,  65,  68,  70,  73,  76,  79,  82,  85,  88,  91,  94,  97,
		100, 103, 106, 109, 112, 115, 119, 122, 125 });

const EvaluationResult FormulaParser::triangle_table({
		128, 130, 132, 134, 136, 138, 140, 142, 144, 146, 148, 150, 152,
	   154, 156, 158, 160, 162, 164, 166, 168, 170, 172, 174, 176, 178,
	   180, 182, 184, 186, 188, 190, 192, 194, 196, 198, 200, 202, 204,
	   206, 208, 210, 212, 214, 216, 218, 220, 222, 224, 226, 228, 230,
	   232, 234, 236, 238, 240, 242, 244, 246, 248, 250, 252, 254, 256,
	   254, 252, 250, 248, 246, 244, 242, 240, 238, 236, 234, 232, 230,
	   228, 226, 224, 222, 220, 218, 216, 214, 212, 210, 208, 206, 204,
	   202, 200, 198, 196, 194, 192, 190, 188, 186, 184, 182, 180, 178,
	   176, 174, 172, 170, 168, 166, 164, 162, 160, 158, 156, 154, 152,
	   150, 148, 146, 144, 142, 140, 138, 136, 134, 132, 130, 128, 126,
	   124, 122, 120, 118, 116, 114, 112, 110, 108, 106, 104, 102, 100,
		98,  96,  94,  92,  90,  88,  86,  84,  82,  80,  78,  76,  74,
		72,  70,  68,  66,  64,  62,  60,  58,  56,  54,  52,  50,  48,
		46,  44,  42,  40,  38,  36,  34,  32,  30,  28,  26,  24,  22,
		20,  18,  16,  14,  12,  10,   8,   6,   4,   2,   0,   2,   4,
		 6,   8,  10,  12,  14,  16,  18,  20,  22,  24,  26,  28,  30,
		32,  34,  36,  38,  40,  42,  44,  46,  48,  50,  52,  54,  56,
		58,  60,  62,  64,  66,  68,  70,  72,  74,  76,  78,  80,  82,
		84,  86,  88,  90,  92,  94,  96,  98, 100, 102, 104, 106, 108,
	   110, 112, 114, 116, 118, 120, 122, 124, 126 });

// 合法变量名及常数化简求值用的暂时变量值
unordered_map<string, EvaluationResult> FormulaParser::temp_vars = {
	{"T", EvaluationResult(0)},
	{"t", EvaluationResult(0)},
	{"w", EvaluationResult(0)},
	{"x", EvaluationResult(0)},
	{"y", EvaluationResult(0)},
	{"z", EvaluationResult(0)},
	{"env1", EvaluationResult(0)},
	{"env2", EvaluationResult(0)},
	{"env3", EvaluationResult(0)},
	{"env4", EvaluationResult(0)}
};

// 合法的函数名及实现
unordered_map<string, FunctionWithBound> FormulaParser::function_dictionary = {
	{
		"sin",
		{ [](const vector<shared_ptr<Expression>>& args, const unordered_map<string, EvaluationResult>& vars, size_t block_size) -> EvaluationResult {
			return xt::index_view(FormulaParser::sine_table, args[0]->evaluate(vars, block_size) % 256);
		}, 1 , 1}
	},
	{
		"cos",
		{ [](const vector<shared_ptr<Expression>>& args, const unordered_map<string, EvaluationResult>& vars, size_t block_size) -> EvaluationResult {
			return xt::index_view(FormulaParser::sine_table, (args[0]->evaluate(vars, block_size) + 64) % 256);
		}, 1 , 1}
	},
	{
		"tri",
		{ [](const vector<shared_ptr<Expression>>& args, const unordered_map<string, EvaluationResult>& vars, size_t block_size) -> EvaluationResult {
			return xt::index_view(FormulaParser::triangle_table, (args[0]->evaluate(vars, block_size)) % 256);
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