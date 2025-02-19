%{
#include <vslc.h>

extern int yylineno;
extern char yytext[];
int yylex (void);

/* Function to report syntax errors */

int yyerror(const char *error)
{
    fprintf(stderr, "%s on line %d\n", error, yylineno);
    exit(EXIT_FAILURE);
}

%}

%token FUNC PRINT RETURN BREAK IF THEN ELSE WHILE DO VAR 
%token OPENBLOCK CLOSEBLOCK 
%token NUMBER IDENTIFIER STRING

%left '+' '-'
%left '*' '/'
%left '<' '>' 
%right UMINUS
%nonassoc IF THEN 
%nonassoc ELSE

%%

program :
    global_list { root = $1; }
    ;

global_list :
    global { $$ = node_create(LIST, NULL, 1, $1); }
    | global_list global { $$ = append_to_list_node($1, $2); }
    ;

global :
    function { $$ = $1; }
    | global_declaration { $$ = $1; }
    ;

global_declaration :
    VAR global_variable_list { $$ = node_create(GLOBAL_DECLARATION, NULL, 1, $2); }
    ;

global_variable_list :
    global_variable { $$ = node_create(LIST, NULL, 1, $1); }
    | global_variable_list ',' global_variable { $$ = append_to_list_node($1, $3); }
    ;

global_variable :
    identifier { $$ = $1; }
    | array_indexing { $$ = $1; }
    ;

array_indexing :
    identifier '[' expression ']' { $$ = node_create(ARRAY_INDEXING, NULL, 2, $1, $3); }
    ;

identifier :
    IDENTIFIER { $$ = node_create(IDENTIFIER_DATA, strdup(yytext), 0); }
    ;

function :
    FUNC identifier '(' parameter_list ')' statement { $$ = node_create(FUNCTION, NULL, 3, $2, $4, $6); }
    ;

parameter_list :
    variable_list { $$ = $1; }
    | /* Empty */ { $$ = node_create( LIST, NULL, 0); }
    ;

variable_list :
    identifier { $$ = node_create(LIST, NULL, 1, $1); }
    | variable_list ',' identifier { $$ = append_to_list_node( $1, $3 ); }
    ;

statement :
    assignment_statement { $$ = $1; }
    | return_statement { $$ = $1; }
    | print_statement { $$ = $1; }
    | if_statement { $$ = $1; }
    | while_statement { $$ = $1; }
    | break_statement { $$ = $1; }
    | function_call { $$ = $1; }
    | block { $$ = $1; }
    ;

block :
    OPENBLOCK local_declaration_list statement_list CLOSEBLOCK { $$ = node_create(BLOCK, NULL, 2, $2, $3); }
    | OPENBLOCK statement_list CLOSEBLOCK { $$ = node_create(BLOCK, NULL, 1, $2); }
    ;

local_declaration_list :
    local_declaration { $$ = node_create( LIST, NULL, 1, $1 ); }
    | local_declaration_list local_declaration { $$ = append_to_list_node($1, $2); }
    ;

local_declaration :
    VAR variable_list { $$ = $2; }
    ;

statement_list :
    statement { $$ = node_create(LIST, NULL, 1, $1); }
    | statement_list statement { $$ = append_to_list_node($1, $2); }
    ;

assignment_statement :
    identifier ':' '=' expression { $$ = node_create(ASSIGNMENT_STATEMENT, NULL, 2, $1, $4); }
    | array_indexing ':' '=' expression { $$ = node_create(ASSIGNMENT_STATEMENT, NULL, 2, $1, $4); }
    ;

return_statement :
    RETURN expression { $$ = node_create(RETURN_STATEMENT, NULL, 1, $2); }
    ;

print_statement :
    PRINT print_list { $$ = node_create(PRINT_STATEMENT, NULL, 1, $2); }
    ;

print_list :
    print_item { $$ = node_create(LIST, NULL, 1, $1); }
    | print_list ',' print_item { $$ = append_to_list_node($1, $3); }
    ;

print_item :
    expression { $$ = $1; }
    | string { $$ = $1; }
    ;

break_statement :
    BREAK { $$ = node_create(BREAK_STATEMENT, NULL, 0); }
    ;

if_statement :
    IF relation THEN statement { $$ = node_create(IF_STATEMENT, NULL, 2, $2, $4); }
    | IF relation THEN statement ELSE statement { $$ = node_create(IF_STATEMENT, NULL, 3, $2, $4, $6); }
    ;

while_statement :
    WHILE relation DO statement { $$ = node_create(WHILE_STATEMENT, NULL, 2, $2, $4); }
    ;

relation :
    expression '=' expression { $$ = node_create(RELATION, "=" , 2, $1, $3); }
    | expression '!' '=' expression { $$ = node_create(RELATION, "!=", 2, $1, $3); }
    | expression '<' expression { $$ = node_create(RELATION, "<", 2, $1, $3); }
    | expression '>' expression { $$ = node_create(RELATION, ">", 2, $1, $3); }
    ;

expression :
    expression '+' expression { $$ = node_create(EXPRESSION, "+", 2, $1, $3); }
    | expression '-' expression { $$ = node_create(EXPRESSION, "-", 2, $1, $3); }
    | expression '*' expression { $$ = node_create(EXPRESSION, "*", 2, $1, $3); }
    | expression '/' expression { $$ = node_create(EXPRESSION, "/", 2, $1, $3); }
    | expression '<' '<' expression { $$ = node_create(EXPRESSION, "<<", 2, $1, $4); }
    | expression '>' '>' expression { $$ = node_create(EXPRESSION, ">>", 2, $1, $4); }
    | '-' expression %prec UMINUS { $$ = node_create(EXPRESSION, "-", 1, $2); }
    | '(' expression ')' { $$ = $2; }
    | number { $$ = $1; }
    | identifier { $$ = $1; }
    | array_indexing { $$ = $1; }
    | function_call { $$ = $1; }
    ;

function_call :
    identifier '(' argument_list ')' { $$ = node_create(FUNCTION_CALL, NULL, 2, $1, $3); }
    ;

argument_list :
    expression_list { $$ = $1; }
    | /* Empty */ { $$ = node_create( LIST, NULL, 0 ); }
    ;

expression_list :
    expression { $$ = node_create(LIST, NULL, 1, $1); }
    | expression_list ',' expression { $$ = append_to_list_node($1, $3); }
    ;

string :
    STRING { $$ = node_create(STRING_DATA, strdup(yytext), 0); }
    ;

number :
    NUMBER { $$ = node_create(NUMBER_DATA, strdup(yytext), 0); }
    ;

%%

