%require "3.0.2"
%defines
%define parser_class_name {RezParser}
%skeleton "lalr1.cc"

%locations;

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%token<std::string> IDENTIFIER;
%token<int> CHARLIT;
%token<std::string> STRINGLIT;
%token<int> INTLIT;

%token<std::string> BADTOKEN;


%token LEFTBRACE "{";
%token RIGHTBRACE "}";
%token LEFTBRACKET "[";
%token RIGHTBRACKET "]";
%token LEFTPAREN "(";
%token RIGHTPAREN ")";
%token SEMICOLON ";";
%token COMMA ",";
%token PLUS "+";
%token MINUS "-";
%token DIVIDE "/";
%token STAR "*";
%token ASSIGN "=";
%token COLON ":";
%token SHIFTLEFT "<<";
%token SHIFTRIGHT ">>";
%token EQUAL "==";
%token NOTEQUAL "!=";
%token AND "&";
%token OR "|";
%token XOR "^";
%token COMPL "~";
%token DOLLAR "$";

%token TYPE "type";
%token RESOURCE "resource";
%token DATA "data";
%token READ "read";
%token INCLUDE "include";
%token CHANGE "change";
%token DELETE "delete";


%token ARRAY "array";
%token SWITCH "switch";
%token CASE "case";
%token AS "as";
%token FILL "fill";
%token ALIGN "align";
%token HEX "hex";
%token KEY "key";
%token WIDE "wide";
%token LITERAL "literal";
%token UNSIGNED "unsigned";
%token BINARY "binary";

%token BOOLEAN "boolean";
%token BIT "bit";
%token NIBBLE "nibble";
%token BYTE "byte";
%token CHAR "char";
%token WORD "word";
%token INTEGER "integer";
%token LONG "long";
%token LONGINT "longint";
%token PSTRING "pstring";
%token WSTRING "wstring";
%token STRING "string";
%token POINT "point";
%token RECT "rect";
%token BITSTRING "bitstring";

%token FUN_COUNTOF "$$countof";
%token FUN_ARRAYINDEX "$$arrayindex";
%token FUN_READ "$$read";
%token FUN_BITFIELD "$$bitfield";
%token FUN_WORD "$$word";
%token FUN_BYTE "$$byte";
%token FUN_LONG "$$long";

/*
%left "|";
%left "^";
%left "&";
%left "==" "!=";
%left ">>" "<<";
%left "+" "-";
%left "*" "/";
*/

%param { RezLexer& lexer }
%param { RezWorld& world }

%code requires {
	#include "ResourceDefinitions.h"
	#include "Expression.h"
	#include "ResSpec.h"

	#define YY_NULLPTR nullptr
	class RezLexer;
	class RezWorld;
}

%code provides {
	using yy::RezParser;
	//using RezSymbol = yy::RezParser::symbol_type;

	class RezSymbol : public yy::RezParser::symbol_type
	{
	public:
		RezSymbol() = default;
		RezSymbol(yy::RezParser::symbol_type&& x) : yy::RezParser::symbol_type(std::move(x)) {}
	};
}

%code {
	#include "RezLexer.h"
	#include "RezWorld.h"
	#include "ResourceCompiler.h"
	#include "Diagnostic.h"

	static yy::RezParser::symbol_type yylex(RezLexer& lexer, RezWorld&)
	{
		return lexer.nextToken();
	}

	void yy::RezParser::error(const location_type& loc, std::string const& err)
	{
		world.problem(Diagnostic(Diagnostic::error, err, loc));
	}

	static std::string fromHex(std::string hex)
	{
		std::string bin;
		int nibble;
		bool haveNibble = false;
		for(std::string::iterator p = hex.begin(); p != hex.end(); ++p)
		{
			if(std::isspace(*p))
				continue;
			assert(isdigit(*p) || (tolower(*p) >= 'a' && tolower(*p) <= 'f'));
			int digit;
			if(isdigit(*p))
				digit = *p - '0';
			else
				digit = tolower(*p) - 'a' + 0xA;

			if(haveNibble)
			{
				bin += (char) ((nibble << 4) | digit);
				haveNibble = false;
			}
			else
			{
				nibble = digit;
				haveNibble = true;
			}
		}
		return bin;
	}

}

%%
%start rez;

rez	: %empty
	| rez type_definition ";"
	| rez resource ";"
	| rez data ";"
	;

type_definition	: "type" type_spec
					{
						TypeDefinitionPtr td = std::make_shared<TypeDefinition>();
						world.addTypeDefinition($type_spec, td);
						world.fieldLists.push(td);
					}
				"{" field_definitions "}"
				{ world.fieldLists.pop(); if(world.verboseFlag) std::cout << "TYPE " << $2 << std::endl; }
				| "type" type_spec "as" type_spec
				{
					if(world.verboseFlag) std::cout << "TYPE " << $2 << std::endl;
					auto spec = $4;
					world.addTypeDefinition($2, world.getTypeDefinition(spec.getType(), spec.getID(), @4));
				}
				;

%type <ResType> res_type;
res_type : CHARLIT { $$ = ResType($1); } ;

%type <TypeSpec> type_spec;
type_spec : res_type { $$ = TypeSpec($res_type); }
		  | res_type "(" INTLIT ")" { $$ = TypeSpec($res_type, $INTLIT); }
		  ;

field_definitions	: %empty
					| field_definitions IDENTIFIER ":"			{ world.fieldLists.top()->addLabel($2, @2); }
					| field_definitions ";"
					| field_definitions field_definition ";"	{ world.fieldLists.top()->addField($2, @2); }
					;

%type <FieldPtr> field_definition;
field_definition: simple_field_definition	{ $$ = $1; }
				| array_definition			{ $$ = $1; }
				| switch_definition			{ $$ = $1; }
				| fill_statement			{ $$ = $1; }
				| align_statement			{ $$ = $1; }
				;

%type <SimpleFieldPtr> simple_field_definition;
simple_field_definition: field_attributes simpletype array_count_opt value_spec_opt
	{
		$$ = std::make_shared<SimpleField>();
		$$->attrs = $field_attributes;
		$$->type = $simpletype;
		$$->arrayCount = $array_count_opt;
		$$->value = $value_spec_opt;
	}
	| simple_field_definition IDENTIFIER
	{ $$ = $1; $$->addNamedValue($IDENTIFIER); }
	| simple_field_definition IDENTIFIER "=" value
	{ $$ = $1; $$->addNamedValue($IDENTIFIER, $value); }
	| simple_field_definition "," IDENTIFIER
	{ $$ = $1; $$->addNamedValue($IDENTIFIER); }
	| simple_field_definition "," IDENTIFIER "=" value
	{ $$ = $1; $$->addNamedValue($IDENTIFIER, $value); }
	;

%type <ExprPtr> array_count array_count_opt value_spec_opt value resource_item;
%type <ExprPtr> expression expression1 expression2 ;
%type <ExprPtr> expression3 expression4 expression5 ;
%type <ExprPtr> expression6 expression7 expression8;

value_spec_opt	: %empty { $$ = nullptr; } | "=" value { $$ = $2; } ;

%type <SimpleField::Type> simpletype;
simpletype	: "boolean"		{ $$ = SimpleField::Type::boolean; }
			| "byte"		{ $$ = SimpleField::Type::byte; }
			| "integer"		{ $$ = SimpleField::Type::integer; }
			| "longint"		{ $$ = SimpleField::Type::longint; }
			| "rect"		{ $$ = SimpleField::Type::rect; }
			| "point"		{ $$ = SimpleField::Type::point; }
			| "char"		{ $$ = SimpleField::Type::char_; }
			| "pstring" 	{ $$ = SimpleField::Type::pstring; }
			| "wstring"		{ $$ = SimpleField::Type::wstring; }
			| "string"		{ $$ = SimpleField::Type::string; }
			| "bitstring"	{ $$ = SimpleField::Type::bitstring; }
			;

%type <FieldPtr> fill_statement align_statement;
fill_statement	: "fill" fill_unit array_count_opt
				{ $$ = std::make_shared<FillAlignField>($fill_unit, false, $array_count_opt); }
				;
align_statement	: "align" fill_unit
				{ $$ = std::make_shared<FillAlignField>($fill_unit, true); }
				;

%type <FillAlignField::Type> fill_unit;
fill_unit	: "bit"		{ $$ = FillAlignField::Type::bit; }
			| "nibble"	{ $$ = FillAlignField::Type::nibble; }
			| "byte"	{ $$ = FillAlignField::Type::byte; }
			| "word"	{ $$ = FillAlignField::Type::word; }
			| "long"	{ $$ = FillAlignField::Type::long_; }
			;

%type <FieldPtr> array_definition;
array_definition:
	array_attributes "array" array_name_opt array_count_opt
	{
		ArrayFieldPtr af = std::make_shared<ArrayField>($array_name_opt, $array_count_opt);
		world.fieldLists.push(af);
	}
	"{" field_definitions "}"
	{
		$$ = world.fieldLists.top();
		world.fieldLists.pop();
	}
	;

array_count : "[" expression "]" { $$ = $2; }
array_count_opt : %empty { $$ = nullptr; } | array_count { $$ = $1; };

%type <std::string> array_name_opt;
array_name_opt : %empty { $$ = ""; } | IDENTIFIER { $$ = $1; } ;

array_attributes: %empty | "wide" ;

%type <SimpleField::Attrs> field_attributes field_attribute;
field_attributes: %empty { $$ = SimpleField::Attrs::none; }
				| field_attributes field_attribute { $$ = $1 | $2; }
				;

field_attribute : "hex"			{ $$ = SimpleField::Attrs::hex; }
				| "key"			{ $$ = SimpleField::Attrs::key; }
				| "unsigned"	{ $$ = SimpleField::Attrs::unsigned_; }
				| "literal"		{ $$ = SimpleField::Attrs::literal; }
                | "binary"      { $$ = SimpleField::Attrs::binary; }
				;

%type <FieldPtr> switch_definition;
switch_definition:
	"switch"
		{ world.switches.push(std::make_shared<SwitchField>()); }
	"{"
		switch_cases
	"}"
		{
			$$ = world.switches.top();
			world.switches.pop();
		}
	;

switch_cases : %empty | switch_cases switch_case ;

switch_case : "case" IDENTIFIER ":"
	{
		world.fieldLists.push(std::make_shared<FieldList>());
	}
	field_definitions
	{
		world.switches.top()->addCase($IDENTIFIER, world.fieldLists.top());
		world.fieldLists.pop();
	}
	;


value	: expression	{ $$ = $1; }
		| "{" resource_body "}"	{ $$ = $2; }
		| string_expression { $$ = $1; }
		;

expression	: expression1	{ $$ = $1; }
			| expression "^" expression1	{ $$ = std::make_shared<BinaryExpr>(BinaryOp::XOR, $1, $3, @1); }
			;

expression1	: expression2	{ $$ = $1; }
			| expression1 "&" expression2	{ $$ = std::make_shared<BinaryExpr>(BinaryOp::AND, $1, $3, @1); }
			;

expression2 : expression3	{ $$ = $1; }
			| expression2 "|" expression3	{ $$ = std::make_shared<BinaryExpr>(BinaryOp::OR, $1, $3, @1); }
			;

expression3 : expression4	{ $$ = $1; }
			| expression3 "==" expression4	{ $$ = std::make_shared<BinaryExpr>(BinaryOp::EQUAL, $1, $3, @1); }
			| expression3 "!=" expression4	{ $$ = std::make_shared<BinaryExpr>(BinaryOp::NOTEQUAL, $1, $3, @1); }
			;

expression4 : expression5	{ $$ = $1; }
			| expression4 ">>" expression5	{ $$ = std::make_shared<BinaryExpr>(BinaryOp::SHIFTRIGHT, $1, $3, @1); }
			| expression4 "<<" expression5	{ $$ = std::make_shared<BinaryExpr>(BinaryOp::SHIFTLEFT, $1, $3, @1); }
			;

expression5 : expression6	{ $$ = $1; }
			| expression5 "+" expression6	{ $$ = std::make_shared<BinaryExpr>(BinaryOp::PLUS, $1, $3, @1); }
			| expression5 "-" expression6	{ $$ = std::make_shared<BinaryExpr>(BinaryOp::MINUS, $1, $3, @1); }
			;

expression6 : expression7	{ $$ = $1; }
			| expression6 "*" expression7	{ $$ = std::make_shared<BinaryExpr>(BinaryOp::MULTIPLY, $1, $3, @1); }
			| expression6 "/" expression7	{ $$ = std::make_shared<BinaryExpr>(BinaryOp::DIVIDE, $1, $3, @1); }
			;
expression7	: expression8		{ $$ = $1; }
			| "-" expression7	{ $$ = std::make_shared<UnaryExpr>(UnaryOp::MINUS, $2, @1); }
			| "+" expression7	{ $$ = $2; }
			| "~" expression7	{ $$ = std::make_shared<UnaryExpr>(UnaryOp::COMPLEMENT, $2, @1); }
			;

expression8	: INTLIT	{ $$ = std::make_shared<IntExpr>($1, @1); }
			| CHARLIT	{ $$ = std::make_shared<IntExpr>($1, @1); }

			| identifier_expression	{ $$ = $1; }
			| "(" expression ")"	{ $$ = $2; }

			| "$$countof" "(" identifier_expression ")"
				{ $$ = std::make_shared<CountOfExpr>($identifier_expression, @1); }
			| "$$arrayindex" "(" identifier_expression ")"
				{ $$ = std::make_shared<ArrayIndexExpr>($identifier_expression, @1); }
			| "$$bitfield" "(" expression "," expression "," expression ")"
				{ $$ = std::make_shared<PeekExpr>($3, $5, $7, @1); }
			| "$$word" "(" expression ")"
				{ $$ = std::make_shared<PeekExpr>($3, 16, @1); }
			| "$$byte" "(" expression ")"
				{ $$ = std::make_shared<PeekExpr>($3, 8, @1); }
			| "$$long" "(" expression ")"
				{ $$ = std::make_shared<PeekExpr>($3, 32, @1); }
			;

%type <IdentifierExprPtr> identifier_expression;
identifier_expression	: IDENTIFIER	{ $$ = std::make_shared<IdentifierExpr>($1, @1); }
						| IDENTIFIER
							{ world.functionCalls.push(std::make_shared<IdentifierExpr>($1, @1)); }
							"[" function_argument_list1 "]"
							{ $$ = world.functionCalls.top(); world.functionCalls.pop(); }
						;

function_argument_list : %empty | function_argument_list1 ;
function_argument_list1 : expression
							{ world.functionCalls.top()->addArgument($expression); }
						| function_argument_list "," expression
							{ world.functionCalls.top()->addArgument($expression); }
						;

%type <ExprPtr> string_expression string_expression1;
string_expression	: string_expression1	{ $$ = $1; }
					| string_expression string_expression1
						{ $$ = std::make_shared<BinaryExpr>(BinaryOp::CONCAT, $1, $2, @1); }
					;

%type <std::string> stringlit;
stringlit	: STRINGLIT { $$ = $1; }
			| DOLLAR STRINGLIT { $$ = fromHex($2); }
			;

string_expression1	:	stringlit	{ $$ = std::make_shared<StringExpr>($1, @1); }
					|	"$$read" "(" string_expression ")"
						{ $$ = std::make_shared<ReadExpr>($string_expression, @1); }
					;

resource	: "resource" res_spec "{" resource_body "}"
			{
				world.addResource($res_spec, $resource_body, @1);
			}
			;

%type <ResSpec> res_spec;
%type <ResSpec> resource_attributes;

res_spec : res_type "(" expression resource_attributes ")"
			{
				$$ = $resource_attributes;
				$$.type() = $res_type;
				$$.id() = $expression->evaluateInt(nullptr);
			}
			;

resource_attributes	: %empty { $$ = ResSpec(); }
					| resource_attributes "," IDENTIFIER
						{
							$$ = $1;
							if($IDENTIFIER == "changed")
								$$.attr() |= 2;
							else if($IDENTIFIER == "preload")
								$$.attr() |= 4;
							else if($IDENTIFIER == "protected")
								$$.attr() |= 8;
							else if($IDENTIFIER == "locked")
								$$.attr() |= 16;
							else if($IDENTIFIER == "purgeable")
								$$.attr() |= 32;
							else if($IDENTIFIER == "sysheap")
								$$.attr() |= 64;
							else if($IDENTIFIER == "unchanged")
								$$.attr() &= ~2;
							else if($IDENTIFIER == "nonpreload")
								$$.attr() &= ~4;
							else if($IDENTIFIER == "unprotected")
								$$.attr() &= ~8;
							else if($IDENTIFIER == "unlocked")
								$$.attr() &= ~16;
							else if($IDENTIFIER == "nonpurgeable")
								$$.attr() &= ~32;
							else if($IDENTIFIER == "appheap")
								$$.attr() &= ~64;
							else
								world.problem(Diagnostic(Diagnostic::error, "illegal attribute " + $IDENTIFIER, @1));
						}
					| resource_attributes "," string_expression
						{
							$$ = $1;
							$$.name() = $3->evaluateString(nullptr);
						}
					;



%type <CompoundExprPtr> resource_body resource_body1;
resource_body	: %empty { $$ = std::make_shared<CompoundExpr>(yy::location()); }
				| resource_body1 { $$ = $1; $$->location = @1; }
				;
resource_body1	: resource_item	{ $$ = std::make_shared<CompoundExpr>(@1); $$->addItem($1); }
				| resource_body1 "," resource_item	{ $$ = $1; $$->addItem($3); }
				| resource_body1 ";" resource_item	{ $$ = $1; $$->addItem($3); }
				| resource_body1 ";" { $$ = $1; }
				| resource_body1 "," { $$ = $1; }
				;

resource_item	: value { $$ = $1; }
				| IDENTIFIER "{" resource_body "}" { $$ = std::make_shared<CaseExpr>($IDENTIFIER, $resource_body, @1); }
				;


data : "data" res_spec "{" string_expression "}"
{
	world.addData($res_spec, $string_expression->evaluateString(nullptr), @1);
}
;

%%
