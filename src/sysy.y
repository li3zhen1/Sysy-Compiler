%code requires {
  #include <memory>
  #include <string>
  #include "ast.h"
}


%{

#include <iostream>
#include <memory>
#include <string>
#include "ast.h"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 *unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
    std::string    	*str_val;
    int 		int_val;
    BaseAST      	*ast_val;
    std::vector<BaseAST*>*ast_vec_val;
    char     		char_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN CONST IF ELSE WHILE CONTINUE BREAK VOID ASSIGN
%token <str_val> IDENT
%token <str_val> LT GT LTE GTE   EQ NEQ AND OR

%token <int_val> INT_CONST

%token <char_val> ADD SUB MUL DIV MOD NEG


// 非终结符的类型定义
%type <ast_val> FuncDef FuncHeader Block Stmt Exp UnaryExp PrimaryExp AddExp MulExp RelExp EqExp LAndExp LOrExp

%type <ast_val> Decl ConstDecl ConstDef
%type <ast_val> ConstInitVal ConstExp BlockItem LVal
%type <ast_val> VarDecl VarDef InitVal
%type <ast_val> FuncFParam GlobalItem
%type <ast_vec_val> GlobalItems BlockItems ConstDefs VarDefs
%type <ast_vec_val> FuncFParams FuncRParams
%type <ast_vec_val> DimConstExps DimExps ConstInitVals InitVals

%type <int_val> Number
%type <char_val> AddOp UnaryOp MulOp
%type <str_val> EqOp RelOp LAndOp LOrOp

%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit
    : GlobalItems {
        auto comp_unit = make_unique<CompUnitAST>();
        comp_unit->setGlobalItems($1);
        ast = move(comp_unit);
    };

GlobalItems
	: GlobalItem {
		auto items = new vector<BaseAST*>();
		items->push_back($1);
		$$ = items;
	}
	| GlobalItem GlobalItems {
		$2->push_back($1);
		$$ = $2;
	}

GlobalItem
 	: Decl {
		auto gi = new GlobalItemAST();
		gi->kind = GlobalItemKind::Decl;
		gi->decl = cast_ast<DeclAST>($1);
		$$ = gi;
	}
	| FuncDef {
          	auto gi = new GlobalItemAST();
          	gi->kind = GlobalItemKind::FuncDef;
          	gi->funcDef = cast_ast<FuncDefAST>($1);
          	$$ = gi;
        };



Decl
	: ConstDecl {
		auto decl = new DeclAST();
		decl->kind = DeclKind::Const;
		decl->constDecl = cast_ast<ConstDeclAST>($1);
		$$ = decl;
	}
	| VarDecl {
		auto decl = new DeclAST();
		decl->kind = DeclKind::Var;
        	decl->varDecl = cast_ast<VarDeclAST>($1);


        	$$ = decl;
	}

ConstDecl
	: CONST INT ConstDefs ';' {
		auto constDecl = new ConstDeclAST();
		constDecl->setConstDefs($3);
		$$ = constDecl;
	}

ConstDefs
	: ConstDef {
		auto defs = new vector<BaseAST*>();
		defs->push_back($1);
		$$ = defs;
	}
	| ConstDef ',' ConstDefs {
		$3->push_back($1);
		$$ = $3;
	}

ConstDef
	: IDENT ASSIGN ConstInitVal {
		auto constDef = new ConstDefAST();
		constDef->ident = *unique_ptr<string>($1);
		constDef->constInitVal = cast_ast<ConstInitValAST>($3);
		$$ = constDef;
	}
	| IDENT DimConstExps ASSIGN ConstInitVal {
        	auto constDef = new ConstDefAST();
        	constDef->ident = *unique_ptr<string>($1);
        	constDef->constInitVal = cast_ast<ConstInitValAST>($4);
        	constDef->setDims($2);
        	$$ = constDef;
        }

DimConstExps
	: '[' ConstExp ']' {
			$$ = new vector<BaseAST*>();
        		$$->push_back($2);
	}
	| '[' ConstExp ']' DimConstExps {
		$4->push_back($2);
		$$ = $4;
	}

DimExps
	: '[' Exp ']' {
		$$ = new vector<BaseAST*>();
        	$$->push_back($2);
	}
	| '[' Exp ']' DimExps {
		$4->push_back($2);
		$$ = $4;
	}



ConstInitVal
	: ConstExp {
		auto civ = new ConstInitValAST();
		civ->kind = ConstInitValKind::Exp;
		civ->constExp = cast_ast<ConstExpAST>($1);
		$$ = civ;
	}
	| '{' '}' {
		auto civ = new ConstInitValAST();
		civ->kind = ConstInitValKind::List;
		civ->setDims(NULL);
		$$ = civ;
        }
	| '{' ConstInitVals '}' {
		auto civ = new ConstInitValAST();
		civ->kind = ConstInitValKind::List;
		civ->setDims($2);
		$$ = civ;
	}

ConstInitVals
	: ConstInitVal {
			$$ = new vector<BaseAST*>();
                	$$->push_back($1);
	}
	| ConstInitVal ',' ConstInitVals {
		$3->push_back($1);
		$$ = $3;
	}

InitVals
	: InitVal {
			$$ = new vector<BaseAST*>();
                	$$->push_back($1);
	}
	| InitVal ',' InitVals {
		$3->push_back($1);
		$$ = $3;
	}


VarDecl
	: INT VarDefs {
		auto varDecl = new VarDeclAST();
		varDecl->setVarDefs($2);
		$$ = varDecl;
	}

VarDefs
	: VarDef ';' {
		auto defs = new vector<BaseAST*>();
		defs->push_back($1);
		$$ = defs;
	}
	| VarDef ',' VarDefs {
		$3->push_back($1);
		$$ = $3;
	}

VarDef
	: IDENT {
		auto def = new VarDefAST();
		def->ident = *unique_ptr<string>($1);
		$$ = def;
	}
	| IDENT ASSIGN InitVal {
		auto def = new VarDefAST();
        	def->ident = *unique_ptr<string>($1);
        	def->initVal = cast_ast<InitValAST>($3);
        	$$ = def;
	}
		| IDENT DimConstExps  {
        		auto def = new VarDefAST();
        		def->ident = *unique_ptr<string>($1);
        		def->setDims($2);
        		$$ = def;
        	}
        	| IDENT DimConstExps ASSIGN InitVal {
        		auto def = new VarDefAST();
                	def->ident = *unique_ptr<string>($1);
                	def->initVal = cast_ast<InitValAST>($4);
        		def->setDims($2);
                	$$ = def;
        	}

InitVal
	: Exp {
		auto initVal = new InitValAST();
		initVal->exp = cast_ast<ExpAST>($1);
		initVal->kind = InitValKind::Exp;
		$$ = initVal;
	}
	| '{' '}' {
		auto initVal = new InitValAST();
		initVal->setDims(NULL);
		initVal->kind = InitValKind::List;
		$$ = initVal;
        }
	| '{' InitVals '}' {
		auto initVal = new InitValAST();
		initVal->setDims($2);
		initVal->kind = InitValKind::List;
		$$ = initVal;
	}



FuncDef
    : FuncHeader ')' Block {
        auto func_def = new FuncDefAST();
        func_def->funcHeader = cast_ast<FuncHeaderAST>($1);
        func_def->block = cast_ast<BlockAST>($3);
        func_def->funcFParams = vector<unique_ptr<FuncFParamAST>>();
        $$ = func_def;
    }
    | FuncHeader FuncFParams ')' Block {
        auto func_def = new FuncDefAST();
        func_def->funcHeader = cast_ast<FuncHeaderAST>($1);
        func_def->block = cast_ast<BlockAST>($4);
        func_def->setFuncFParams($2);
        $$ = func_def;
    };

// 同上, 不再解释
FuncHeader
  : INT IDENT '(' {
    auto ft = new FuncHeaderAST();
    ft->kind = FuncTypeKind::Int;
    ft->ident = *unique_ptr<string>($2);
    $$ = ft;
  }
  | VOID IDENT '(' {
    auto ft = new FuncHeaderAST();
    ft->kind = FuncTypeKind::Void;
    ft->ident = *unique_ptr<string>($2);
    $$ = ft;
  };

FuncFParams
	: FuncFParam {
		$$ = new vector<BaseAST*>();
		$$->push_back($1);
	}
	| FuncFParam ',' FuncFParams {
		$3->push_back($1);
		$$ = $3;
	}

FuncFParam
	: INT IDENT {
		auto ffp = new FuncFParamAST();
		ffp->ident = *unique_ptr<string>($2);
		$$ = ffp;
	}
	| INT IDENT '[' ']'{
		auto ffp = new FuncFParamAST();
		ffp->ident = *unique_ptr<string>($2);
		ffp->setDims(NULL);
		$$ = ffp;
	}
	| INT IDENT '[' ']' DimConstExps {
		auto ffp = new FuncFParamAST();
		ffp->ident = *unique_ptr<string>($2);
		ffp->setDims($5);
		$$ = ffp;
	}



Block
  : '{' '}' {
    auto block = new BlockAST();
    block->items = vector<unique_ptr<BlockItemAST>>();
    $$ = block;
  }
  | '{' BlockItems '}' {
    auto block = new BlockAST();
    block->setBlockItems($2);
    $$ = block;
  };

BlockItems
	: BlockItem {
		$$ = new vector<BaseAST*>();
		$$->push_back($1);
	}
	| BlockItem BlockItems {
		$2->push_back($1);
		$$ = $2;
	}


BlockItem
	: Decl {
		auto bi = new BlockItemAST();
		bi->kind = BlockItemKind::Decl;
		bi->decl = cast_ast<DeclAST>($1);
		$$ = bi;
	}
	| Stmt {
		auto bi = new BlockItemAST();
        	bi->kind = BlockItemKind::Stmt;
        	bi->stmt = cast_ast<StmtAST>($1);
        	$$ = bi;
	}

Stmt
	: LVal ASSIGN Exp ';' {
         	auto stmt = new StmtAST();
                stmt->lVal = cast_ast<LValAST>($1);
                stmt->exp = cast_ast<ExpAST>($3);
                stmt->kind = StmtKind::Assign;
                $$ = stmt;
        }
        | Exp ';' {
                auto stmt = new StmtAST();
                stmt->kind = StmtKind::ExpStmt;
                stmt->exp = cast_ast<ExpAST>($1);
                $$ = stmt;
        }
        | ';' {
                auto stmt = new StmtAST();
                stmt->kind = StmtKind::ExpStmt;
                $$ = stmt;
        }
        | Block {
                auto stmt = new StmtAST();
                stmt->kind = StmtKind::Block;
                stmt->block = cast_ast<BlockAST>($1);
                $$ = stmt;
        }
        | IF '(' Exp ')' Stmt ELSE Stmt  {
                auto stmt = new StmtAST();
                stmt->kind = StmtKind::If;
		stmt->exp = cast_ast<ExpAST>($3);
                stmt->ifStmt = cast_ast<StmtAST>($5);
                stmt->elseStmt = cast_ast<StmtAST>($7);
                $$ = stmt;
        }
        | IF '(' Exp ')' Stmt  {
                auto stmt = new StmtAST();
                stmt->kind = StmtKind::If;
		stmt->exp = cast_ast<ExpAST>($3);
                stmt->ifStmt = cast_ast<StmtAST>($5);
//                stmt->elseStmt = cast_ast<StmtAST>($7);
                $$ = stmt;
        }
        | WHILE '(' Exp ')' Stmt  {
                auto stmt = new StmtAST();
                stmt->kind = StmtKind::While;
		stmt->exp = cast_ast<ExpAST>($3);
                stmt->whileStmt = cast_ast<StmtAST>($5);
                $$ = stmt;
        }
        | BREAK ';' {
        	auto stmt = new StmtAST();
        	stmt->kind = StmtKind::Break;
        	$$ = stmt;
        }
        | CONTINUE ';' {
        	auto stmt = new StmtAST();
        	stmt->kind = StmtKind::Continue;
        	$$ = stmt;
        }
	| RETURN Exp ';' {
		auto stmt = new StmtAST();
		stmt->exp = cast_ast<ExpAST>($2);
		stmt->kind = StmtKind::Return;
		$$ = stmt;
	}
	| RETURN ';' {
        	auto stmt = new StmtAST();
        	stmt->kind = StmtKind::Return;
        	$$ = stmt;
        };

Exp
	: LOrExp {
		auto exp = new ExpAST();
		exp->lOrExp = cast_ast<LOrExpAST>($1);
		$$ = exp;
	}
LVal
	: IDENT {
		auto lVal = new LValAST();
		lVal->ident = *unique_ptr<string>($1);
		lVal->setDims(NULL);
		$$ = lVal;
	}
	| IDENT DimExps  {
         	auto lVal = new LValAST();
         	lVal->ident = *unique_ptr<string>($1);
         	lVal->setDims($2);
         	$$ = lVal;
        }

PrimaryExp
	: '(' Exp ')' {
		auto primaryExp = new PrimaryExpAST();
		primaryExp->kind = PrimaryExpKind::Exp;
		primaryExp->exp = cast_ast<ExpAST>($2);
		$$ = primaryExp;
	}
	| LVal {
		auto primaryExp = new PrimaryExpAST();
                primaryExp->kind = PrimaryExpKind::LVal;
                primaryExp->lVal = cast_ast<LValAST>($1);
                $$ = primaryExp;
	}
	| Number {
		auto primaryExp = new PrimaryExpAST();
		primaryExp->kind = PrimaryExpKind::Number;
		primaryExp->number = $1;
		$$ = primaryExp;
	};

UnaryExp
	: PrimaryExp {
		auto unaryExp = new UnaryExpAST();
		unaryExp->kind = UnaryExpKind::Primary;
		unaryExp->primary = cast_ast<PrimaryExpAST>($1);
		$$ = unaryExp;
	}
	| IDENT '(' ')' {
		auto unaryExp = new UnaryExpAST();
		unaryExp->kind = UnaryExpKind::Call;
		unaryExp->ident = *unique_ptr<string>($1);
		unaryExp->setFuncRParams();
		$$ = unaryExp;
	}
	| IDENT '(' FuncRParams ')' {
		auto unaryExp = new UnaryExpAST();
		unaryExp->kind = UnaryExpKind::Call;
		unaryExp->ident = *unique_ptr<string>($1);
		unaryExp->setFuncRParams($3);
		$$ = unaryExp;
	}
	| UnaryOp UnaryExp {

		auto unaryExp = new UnaryExpAST();
		unaryExp->kind = UnaryExpKind::Unary;
		unaryExp->unary = cast_ast<UnaryExpAST>($2);
		unaryExp->unaryOp = $1;
		$$ = unaryExp;
	};

FuncRParams
	: Exp {
		$$ = new vector<BaseAST*>();
		$$->push_back($1);
	}
	| Exp ',' FuncRParams {
        	$3->push_back($1);
        	$$ = $3;
	}

MulExp:
	UnaryExp {
		auto exp = new MulExpAST();
		exp->kind = MulExpKind::Unary;
		exp->unary = cast_ast<UnaryExpAST>($1);
		$$ = exp;
	}
	| MulExp MulOp UnaryExp {
		auto exp = new MulExpAST();
		exp->kind = MulExpKind::MulUnary;
                exp->unary = cast_ast<UnaryExpAST>($3);
                exp->mul = cast_ast<MulExpAST>($1);
                exp->op = $2;
                $$ = exp;
	}

AddExp:
	MulExp {
        auto addExp = new AddExpAST();
        addExp->kind = AddExpKind::Mul;
        addExp->mul = cast_ast<MulExpAST>($1);
        $$ = addExp;
    }
    | AddExp AddOp MulExp {
        auto addExp = new AddExpAST();
        addExp->kind = AddExpKind::AddMul;
        addExp->add = cast_ast<AddExpAST>($1);
        addExp->mul = cast_ast<MulExpAST>($3);
        addExp->op = $2;
        $$ = addExp;
    }


RelExp
	: AddExp {
		auto rel = new RelExpAST();
		rel->kind = RelExpKind::Add;
		rel->add = cast_ast<AddExpAST>($1);
		$$ = rel;
	}
	| RelExp RelOp AddExp {
		auto rel = new RelExpAST();
		rel->kind = RelExpKind::RelAdd;
		rel->add = cast_ast<AddExpAST>($3);
		rel->rel = cast_ast<RelExpAST>($1);
		rel->op = *unique_ptr<string>($2);
		$$ = rel;
	}

EqExp
	: RelExp {
		auto eq = new EqExpAST();
		eq->kind = EqExpKind::Rel;
		eq->rel = cast_ast<RelExpAST>($1);
		$$ = eq;
	}
	| EqExp EqOp RelExp {
		auto eq = new EqExpAST();
		eq->kind = EqExpKind::EqRel;
		eq->eq = cast_ast<EqExpAST>($1);
		eq->rel = cast_ast<RelExpAST>($3);
		eq->op = *unique_ptr<string>($2);
		$$ = eq;
	}

LAndExp
	: EqExp {
		auto lAnd = new LAndExpAST();
		lAnd->kind = LAndExpKind::Eq;
		lAnd->eq = cast_ast<EqExpAST>($1);
		$$ = lAnd;
	}
	| LAndExp LAndOp EqExp {
		auto lAnd = new LAndExpAST();
		lAnd->kind = LAndExpKind::LAndEq;
		lAnd->lAnd = cast_ast<LAndExpAST>($1);
		lAnd->eq = cast_ast<EqExpAST>($3);
		$$ = lAnd;
	}

LOrExp
	: LAndExp {
		auto lOr = new LOrExpAST();
		lOr->kind = LOrExpKind::LAnd;
		lOr->lAnd = cast_ast<LAndExpAST>($1);
		$$ = lOr;
	}
	| LOrExp LOrOp LAndExp {
		auto lOr = new LOrExpAST();
		lOr->kind = LOrExpKind::LOrLAnd;
		lOr->lAnd = cast_ast<LAndExpAST>($3);
		lOr->lOr = cast_ast<LOrExpAST>($1);
		$$ = lOr;
	};





ConstExp
	: Exp {
		auto ce = new ConstExpAST();
		ce->exp = cast_ast<ExpAST>($1);
		$$ = ce;
	}






Number
  : INT_CONST {
    $$ = $1;
  };

AddOp
	: ADD { $$ = '+'; }
	| SUB { $$ = '-'; }

UnaryOp
	: ADD { $$ = '+'; }
	| SUB { $$ = '-'; }
	| NEG { $$ = '!'; }

MulOp
	: MUL { $$ = '*'; }
	| DIV { $$ = '/'; }
	| MOD { $$ = '%'; }

RelOp
	: LT { $$ = $1; }
	| GT { $$ = $1; }
	| LTE { $$ = $1; }
        | GTE { $$ = $1; }

EqOp
	: EQ {$$ = $1; }
	| NEQ {$$ = $1; }

LAndOp
	: AND { $$ = $1; }

LOrOp
	: OR { $$ = $1; }



%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {

	cerr << "error: " << s << endl;
}
