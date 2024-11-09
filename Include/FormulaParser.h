#ifndef PARSER_H
#define PARSER_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <peglib.h>
#include <xtensor/xarray.hpp>

namespace fparse {
	// ¶¨Òå²Ù×÷·ûµÄÃ¶¾Ù
	enum class Operation {
		NONE, // ´ú±íÃ»ÓÐ²Ù×÷
		ADD,
		SUBTRACT,
		MULTIPLY,
		DIVIDE,
		MOD,
		AND,
		OR,
		XOR,
		SHIFT_LEFT,
		SHIFT_RIGHT
	};

	using EvaluationResult = xt::xarray<int32_t>;

	// ±í´ïÊ½»ùÀà
	class Expression {
	public:
		virtual ~Expression() = default;
		virtual std::string toString() const = 0;											// debug
		virtual bool isConstant() const = 0;											// constant simplify
		virtual EvaluationResult evaluate(const std::unordered_map<std::string, EvaluationResult>& vars, size_t block_size) const = 0;	// evaluation
	};

	// ÄäÃûº¯ÊýµÄÀàÐÍ
	using FunctionType = std::function<EvaluationResult(const std::vector<std::shared_ptr<Expression>>&, const std::unordered_map<std::string, EvaluationResult>&, size_t)>;

	// ÄäÃûº¯ÊýµÄº¯Êý²ÎÊý¶¨ÒåÀàÐÍ
	struct FunctionWithBound {
		FunctionType function;	// º¯Êý±¾Ìå
		int16_t lower_bound;	// ²ÎÊýÁ¿ÉÏ½ç
		int16_t upper_bound;	// ²ÎÊýÁ¿ÏÂ½ç
	};


	// ±äÁ¿Àà
	class Variable : public Expression {
	public:
		std::string name;																	// Var name

		Variable(const std::string& name);
		~Variable() override {};
		bool isConstant() const override { return false; }								// constant simplify
		std::string toString() const override;												// debug
		EvaluationResult evaluate(const std::unordered_map<std::string, EvaluationResult>& vars, size_t block_size) const override;	// evaluation
	};

	// ³£Á¿Àà
	class Constant : public Expression {
	public:
		int32_t value;

		Constant(int32_t value);
		~Constant() override {}
		bool isConstant() const override { return true; }								// constant simplify
		std::string toString() const override;												// debug
		EvaluationResult evaluate(const std::unordered_map<std::string, EvaluationResult>&, size_t block_size) const override;			// evaluation
	};

	// ¶þÔª±í´ïÊ½Àà
	class CompoundExpression : public Expression {
	public:
		std::shared_ptr<Expression> l, r;													// operands
		Operation operation;

		CompoundExpression(Operation op, std::shared_ptr<Expression> lhs, std::shared_ptr<Expression> rhs);
		~CompoundExpression() override {}
		bool isConstant() const override { return false; }								// constant simplify
		std::string toString() const override;												// debug
		EvaluationResult evaluate(const std::unordered_map<std::string, EvaluationResult>& vars, size_t block_size) const override;	// evaluation
	};

	// º¯Êý±í´ïÊ½Àà
	class FunctionExpression : public Expression {
	public:
		std::string name;																	// function name
		FunctionWithBound function;														// function		
		std::vector<std::shared_ptr<Expression>> args;											// function arguments

		FunctionExpression(std::string function_name, std::vector<std::shared_ptr<Expression>> function_args);
		~FunctionExpression() override {}
		bool isConstant() const override { return false; }								// constant simplify
		std::string toString() const override;												// debug
		EvaluationResult evaluate(const std::unordered_map<std::string, EvaluationResult>& vars, size_t block_size) const override;	// evaluation
	};

	// ½âÎö½á¹û
	struct ParseResult {
		bool success;
		std::shared_ptr<Expression> expr;
		size_t line;
		size_t col;
		std::string msg;
		std::string rule;
	};

	// ½âÎöÆ÷Àà
	class FormulaParser {
	public:
		static const char* grammar;

		static const EvaluationResult sine_table, triangle_table;

		static std::unordered_map<std::string, EvaluationResult> temp_vars;
		static std::unordered_map<std::string, FunctionWithBound> function_dictionary;

		FormulaParser();
		ParseResult parse(std::string& input) noexcept;

	private:
		peg::parser parser;
	};
};
#endif