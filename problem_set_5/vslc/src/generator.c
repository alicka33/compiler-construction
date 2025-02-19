#include "vslc.h"

// This header defines a bunch of macros we can use to emit assembly to stdout
#include "emit.h"

// In the System V calling convention, the first 6 integer parameters are passed in registers
#define NUM_REGISTER_PARAMS 6
static const char *REGISTER_PARAMS[6] = {RDI, RSI, RDX, RCX, R8, R9};

// Takes in a symbol of type SYMBOL_FUNCTION, and returns how many parameters the function takes
#define FUNC_PARAM_COUNT(func) ((func)->node->children[1]->n_children)

static void generate_stringtable ( void );
static void generate_global_variables ( void );
static void generate_function ( symbol_t *function );
static void generate_expression ( node_t *expression );
static void generate_statement ( node_t *node );
static void generate_main ( symbol_t *first );

/* Entry point for code generation */
void generate_program ( void )
{
    generate_stringtable ( );
    generate_global_variables ( );

    // This directive announces that the following assembly belongs to the executable code .text section.
    DIRECTIVE ( ".text" );
    // TODO: (Part of 2.3)
    // For each function in global_symbols, generate it using generate_function ()
    symbol_t *main = NULL;
    bool main_found = false;
    for (size_t i = 0; i < global_symbols->n_symbols; i++)
    {
        symbol_t *symbol = global_symbols->symbols[i];
        if (symbol->type == SYMBOL_FUNCTION)
        {
            generate_function ( symbol );
            if(!main_found){
                main = symbol;
                main_found = true;
            }
        }
    }

    // TODO: (Also part of 2.3)
    // In VSL, the topmost function in a program is its entry point.
    // We want to be able to take parameters from the command line,
    // and have them be sent into the entry point function.
    //
    // Due to the fact that parameters are all passed as strings,
    // and passed as the (argc, argv)-pair, we need to make a wrapper for our entry function.
    // This wrapper handles string -> int64_t conversion, and is already implemented.
    // call generate_main ( <entry point function symbol> );
    generate_main ( main );

}

/* Prints one .asciz entry for each string in the global string_list */
static void generate_stringtable ( void )
{
    // This section is where read-only string data is stored
    // It is called .rodata on Linux, and "__TEXT, __cstring" on macOS
    DIRECTIVE ( ".section %s", ASM_STRING_SECTION );

    // These strings are used by printf
    DIRECTIVE ( "intout: .asciz \"%s\"", "%ld" );
    DIRECTIVE ( "strout: .asciz \"%s\"", "%s" );
    // This string is used by the entry point-wrapper
    DIRECTIVE ( "errout: .asciz \"%s\"", "Wrong number of arguments" );

    // TODO 2.1: Print all strings in the program here, with labels you can refer to later
    for (size_t i = 0; i < string_list_len; i++)
    {
        DIRECTIVE ( "string%ld: .asciz %s", i, string_list[i] );
    }

}

/* Prints .zero entries in the .bss section to allocate room for global variables and arrays */
static void generate_global_variables ( void )
{
    // This section is where zero-initialized global variables lives
    // It is called .bss on linux, and "__DATA, __bss" on macOS
    DIRECTIVE ( ".section %s", ASM_BSS_SECTION );
    DIRECTIVE ( ".align 8" );

    // TODO 2.2: Fill this section with all global variables and global arrays
    // Give each a label you can find later, and the appropriate size.
    // Regular variables are 8 bytes, while arrays are 8 bytes per element.
    // Remember to mangle the name in some way, to avoid collisions if a variable is called e.g. "main"
    for (size_t i = 0; i < global_symbols->n_symbols; i++)
    {
        symbol_t *symbol = global_symbols->symbols[i];
        if (symbol->type == SYMBOL_GLOBAL_VAR)
        {
            DIRECTIVE ( ".%s: .zero 8", symbol->name );
        }
        else if (symbol->type == SYMBOL_GLOBAL_ARRAY)
        {
            void *array_size = symbol->node->children[1]->data;
            DIRECTIVE ( ".%s: .zero %d", symbol->name, *(int *)array_size * 8);
        }
    }



}

/* Global variable you can use to make the functon currently being generated accessible from anywhere */
static symbol_t *current_function;

/* Prints the entry point. preamble, statements and epilouge of the given function */
static void generate_function ( symbol_t *function )
{
    // TODO: 2.3

    // TODO: 2.3.1 Do the prologue, including call frame building and parameter pushing
    // Tip: use the definitions REGISTER_PARAMS and NUM_REGISTER_PARAMS at the top of this file
    LABEL(".%s", function->name);
    PUSHQ(RBP);
    MOVQ(RSP, RBP);
    int param_offset = 16;
    int param_count = 0;
    for (int i = 0; i < function->function_symtable->n_symbols; ++i) {
        symbol_t *symbol = function->function_symtable->symbols[i];
        if(symbol->type == SYMBOL_PARAMETER && param_count < NUM_REGISTER_PARAMS){
            PUSHQ(REGISTER_PARAMS[i]);
            param_offset += 8;
            param_count++;
        }else if (symbol->type == SYMBOL_PARAMETER){
            char param_offset_str[20];
            sprintf(param_offset_str, "-%d(%s)", param_offset, RBP);
            PUSHQ(param_offset_str);        // ????? do sprawdzenia
            param_offset += 8;
        }else if (symbol->type == SYMBOL_LOCAL_VAR){
            PUSHQ("$0");
        }

    }
    // TODO: 2.4 the function body can be sent to generate_statement()

    generate_statement(function->node->children[2]);


    // TODO: 2.3.2
    MOVQ(RBP, RSP);
    POPQ(RBP);
    RET;
}

static void generate_function_call ( node_t *call )
{
    // TODO 2.4.3
    int num_args = call->n_children - 1;
    for (int i = num_args - 1; i >= 0; --i) {
        generate_expression(call->children[i + 1]);
        if (i < NUM_REGISTER_PARAMS) {
            MOVQ(RAX, REGISTER_PARAMS[i]);
        } else {
            PUSHQ(RAX);
        }
    }

    char function_label[50];
    sprintf(function_label, ".%s", call->children[0]->data);
    CALL(function_label);

    if (num_args > NUM_REGISTER_PARAMS) {
        ADDQ("$" + (num_args - NUM_REGISTER_PARAMS) * 8, RSP);
    }
}

/* Generates code to evaluate the expression, and place the result in %rax */
static void generate_expression ( node_t *expression )
{
    // TODO: 2.4.1 Generate code for evaluating the given expression.
     char data[50];
     char data2[50];
     switch (expression->type) {
        case NUMBER_DATA:
            sprintf(data, "$%ld", *((long *)expression->data));
            MOVQ(data, RAX);
            break;
        case IDENTIFIER_DATA: {
            symbol_t *symbol = expression->symbol;
            if (symbol->type == SYMBOL_LOCAL_VAR) {
                sprintf(data, "$%d", -symbol->sequence_number * 8);
                MOVQ(data, RCX);
                ADDQ(RCX, RBP);
                sprintf(data2, "0(" RBP ")");
                LEAQ(data2, RCX);
                MOVQ(data2, RAX);
            } else if (symbol->type == SYMBOL_GLOBAL_VAR) {
                sprintf(data, ".%s(%rip)", symbol->name);
                MOVQ(data, RAX);
            } else {
                // fprintf(stderr, "error: unsupported symbol type for expression\n");
                // exit(EXIT_FAILURE);
                break;
            }
            break;
        }
        case ARRAY_INDEXING: {
            generate_expression(expression->children[1]);
            sprintf(data, ".%s(%rip)", expression->children[0]->data);
            // MOVQ(data, RCX);
            LEAQ(data, RCX);
            // MOVQ(RAX, RAX);
            MOVQ(MEM(RAX), RAX);
            break;
        }
        case EXPRESSION: {
            node_t *left = expression->children[0];
            node_t *right = expression->children[1];
            generate_expression(left);
            PUSHQ(RAX);
            generate_expression(right);
            POPQ(RCX);
            char* operator = (char*)expression->data;
            if(strcmp(operator,"+") == 0){
                ADDQ(RCX, RAX);
            }else if(strcmp(operator,"-") == 0){
                SUBQ(RCX, RAX);
            }else if(strcmp(operator,"*") == 0){
                IMULQ(RCX, RAX);
            }else if(strcmp(operator,"/") == 0){
                CQO;
                IDIVQ(RCX);
            }else if(strcmp(operator,"<<") == 0){
                SAL(RCX, RAX);
            }else if(strcmp(operator,">>") == 0){
                SAR(RCX, RAX);
            }else if(strcmp(operator,"&") == 0){
                ANDQ(RCX, RAX);
            }
            break;
        }
        case FUNCTION_CALL: {
            // printf("Function call\n");
            generate_function_call(expression);
            break;
        }
        default:
            // fprintf(stderr, "error: unsupported expression type %d\n", expression->type);
            // exit(EXIT_FAILURE);
            break;
    }
}

static void generate_assignment_statement ( node_t *statement )
{
    // TODO: 2.4.2
    // You can assign to both local variables, global variables and function parameters.
    // Use the IDENTIFIER_DATA's symbol to find out what kind of symbol you are assigning to.
    // The left hand side of an assignment statement may also be an ARRAY_INDEXING node.

    node_t *lhs = statement->children[0];
    node_t *rhs = statement->children[1];

    generate_expression(rhs);
    char label[50];
    char label2[50];
    if (lhs->type == IDENTIFIER_DATA) {
        symbol_t *symbol = lhs->symbol;
        switch (symbol->type) {
            case SYMBOL_LOCAL_VAR:
                sprintf(label, "$%ld", -symbol->sequence_number * 8);
                MOVQ(label, RCX);
                ADDQ(RCX, RBP);
                sprintf(label2, "0(" RBP ")");
                LEAQ(label2, RCX);
                MOVQ(label2, RAX);
                break;
            case SYMBOL_GLOBAL_VAR:
                sprintf(label, ".%s(%rip)", symbol->name);
                LEAQ(label, RSI);
                MOVQ(RAX, MEM(RIP));
                break;
            case SYMBOL_PARAMETER:
                if (symbol->sequence_number < NUM_REGISTER_PARAMS) {
                    MOVQ(RAX, REGISTER_PARAMS[symbol->sequence_number]);
                } else {
                    sprintf(label, "$%ld", (symbol->sequence_number - NUM_REGISTER_PARAMS + 2) * 8);
                    IMULQ(label, RCX);
                    ADDQ(RCX, RBP);
                    MOVQ(RAX, MEM(RBP));
                }
                break;
            default:
                break;
        }
    }
    else if (lhs->type == ARRAY_INDEXING) {
        generate_expression(lhs->children[1]);
        generate_expression(lhs->children[0]);
        symbol_t *array_symbol = lhs->children[0]->symbol;
        if (array_symbol->type == SYMBOL_GLOBAL_VAR) {
            sprintf(label, "$%s", array_symbol->name);
            MOVQ(label, RCX);
            ADDQ(RIP, RCX);
            MOVQ(MEM(RCX), RCX);
        } else if (array_symbol->type == SYMBOL_LOCAL_VAR) {
            sprintf(label, "$%ld", -array_symbol->sequence_number * 8);
            MOVQ(label, RCX);
            ADDQ(RBP, RCX);
            MOVQ(MEM(RCX), RCX);
        }
        IMULQ(IMM(8), RAX);
        ADDQ(RAX, RCX);
        MOVQ(RCX, MEM(RAX));
    }

}

static void generate_print_statement ( node_t *statement )
{
    // TODO: 2.4.4
    // Remember to call safe_printf instead of printf
    for (size_t i = 0; i < statement->n_children; ++i) {
        node_t *item = statement->children[i];
        if (item->type == STRING_LIST_REFERENCE) {
            LABEL("print_string_%ld", i);
            DIRECTIVE(ASM_STRING_SECTION);
            DIRECTIVE(".asciz %s\n", (char)item->data + "\"");
            LEAQ("$strout(%rip)", RDI);
            char data[50];
            sprintf(data, "string%ld(%rip)", i);
            LEAQ(data, RSI);
            CALL("safe_printf");
        } else if (item->type == EXPRESSION) {
            generate_expression(item);
            MOVQ("$intout", RDI);
        }
    }
    MOVQ("$0", RAX);
    CALL("safe_printf");
}

static void generate_return_statement ( node_t *statement )
{
    // TODO: 2.4.5 Store the value in %rax and jump to the function epilogue
    generate_expression(statement->children[0]);
    MOVQ(RAX, RAX);
    char label[50];
    if(current_function && current_function->name){
        snprintf(label, sizeof(label), ".%s_exit", current_function->name);
        JMP(label);
    }
}

/* Recursively generate the given statement node, and all sub-statements. */
static void generate_statement ( node_t *node )
{
    // TODO: 2.4
    switch (node->type)
    {
        case EXPRESSION:
            generate_expression(node);
            break;
        case ASSIGNMENT_STATEMENT:
            generate_assignment_statement(node);
            break;
        case PRINT_STATEMENT:
            generate_print_statement(node);
            break;
        case RETURN_STATEMENT:
            generate_return_statement(node);
            break;
        case FUNCTION_CALL:
            generate_function_call(node);
            break;
        default:
            for (size_t i = 0; i < node->n_children; i++){
                generate_statement(node->children[i]);
            }
            break;
    }
}

static void generate_safe_printf ( void )
{
    LABEL ( "safe_printf" );

    PUSHQ ( RBP );
    MOVQ ( RSP, RBP );
    // This is a bitmask that abuses how negative numbers work, to clear the last 4 bits
    // A stack pointer that is not 16-byte aligned, will be moved down to a 16-byte boundary
    ANDQ ( "$-16", RSP );
    EMIT ( "call printf" );
    // Cleanup the stack back to how it was
    MOVQ ( RBP, RSP );
    POPQ ( RBP );
    RET;
}

static void generate_main ( symbol_t *first )
{
    // Make the globally available main function
    LABEL ( "main" );

    // Save old base pointer, and set new base pointer
    PUSHQ ( RBP );
    MOVQ ( RSP, RBP );

    // Which registers argc and argv are passed in
    const char* argc = RDI;
    const char* argv = RSI;

    const size_t expected_args = FUNC_PARAM_COUNT ( first );

    SUBQ ( "$1", argc ); // argc counts the name of the binary, so subtract that
    EMIT ( "cmpq $%ld, %s", expected_args, argc );
    JNE ( "ABORT" ); // If the provdied number of arguments is not equal, go to the abort label

    if (expected_args == 0)
        goto skip_args; // No need to parse argv

    // Now we emit a loop to parse all parameters, and push them to the stack,
    // in right-to-left order

    // First move the argv pointer to the vert rightmost parameter
    EMIT( "addq $%ld, %s", expected_args*8, argv );

    // We use rcx as a counter, starting at the number of arguments
    MOVQ ( argc, RCX );
    LABEL ( "PARSE_ARGV" ); // A loop to parse all parameters
    PUSHQ ( argv ); // push registers to caller save them
    PUSHQ ( RCX );

    // Now call strtol to parse the argument
    EMIT ( "movq (%s), %s", argv, RDI ); // 1st argument, the char *
    MOVQ ( "$0", RSI ); // 2nd argument, a null pointer
    MOVQ ( "$10", RDX ); //3rd argument, we want base 10
    EMIT ( "call strtol" );

    // Restore caller saved registers
    POPQ ( RCX );
    POPQ ( argv );
    PUSHQ ( RAX ); // Store the parsed argument on the stack

    SUBQ ( "$8", argv ); // Point to the previous char*
    EMIT ( "loop PARSE_ARGV" ); // Loop uses RCX as a counter automatically

    // Now, pop up to 6 arguments into registers instead of stack
    for ( size_t i = 0; i < expected_args && i < NUM_REGISTER_PARAMS; i++ )
        POPQ ( REGISTER_PARAMS[i] );

    skip_args:

    EMIT ( "call .%s", first->name );
    MOVQ ( RAX, RDI ); // Move the return value of the function into RDI
    EMIT ( "call exit" ); // Exit with the return value as exit code

    LABEL ( "ABORT" ); // In case of incorrect number of arguments
    EMIT ( "leaq errout(%s), %s", RIP, RDI );
    EMIT ( "call puts" ); // print the errout string
    MOVQ ( "$1", RDI );
    EMIT ( "call exit" ); // Exit with return code 1

    generate_safe_printf();

    // Declares global symbols we use or emit, such as main, printf and putchar
    DIRECTIVE( "%s", ASM_DECLARE_SYMBOLS);
}