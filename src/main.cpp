#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <fstream>
#include <koopa.h>
#include "ast.h"
#include "ir/program.h"
#include "risk/program.h"


#ifndef __APPLE__
#include <malloc.h>
#endif


using namespace std;

extern FILE *yyin;

extern int yyparse(unique_ptr<BaseAST> &ast);


koopa_error_code_t kec = 0;
koopa_program_t kpt = 0;

int main(int argc, const char *argv[]) {

    assert(argc == 5);
    auto mode = argv[1];
    auto input = argv[2];
    auto output = argv[4];

    cout << mode << "  " << input << "  " << output << endl;


    yyin = fopen(input, "r");
    assert(yyin);

    unique_ptr<BaseAST> ast;
    auto parse_result = yyparse(ast);
    assert(!parse_result);

    auto compUnit = unique_ptr<CompUnitAST>((CompUnitAST *) ast.release());

    auto prog = ir::Program();
    prog.visit(compUnit);

    if (mode[1] == 'k') {
        auto outputFile = ofstream(output);
        outputFile << prog << endl;
        outputFile.close();
    } else if (mode[1] == 'r') {

        auto ss = ostringstream();
        ss << prog << endl;
        auto irStr = ss.str();
        auto ir = irStr.c_str();

        koopa_program_t program;
        koopa_error_code_t ret = koopa_parse_from_string(ir, &program);
        assert(ret == KOOPA_EC_SUCCESS);
        koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
        koopa_raw_program_t raw = koopa_build_raw_program(builder, program);

        auto riskStream = ostringstream();
        auto rProgram = risk::Program();


        cout << riskStream.str() << endl;

        auto outputFile = ofstream(output);
        rProgram.visit(raw, outputFile);
        outputFile.close();

        koopa_delete_program(program);
        koopa_delete_raw_program_builder(builder);
    }


/*


//    koopa_dump_to_stdout(program);*/

    return 0;
}
