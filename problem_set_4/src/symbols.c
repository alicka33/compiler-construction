#include <vslc.h>

/* Global symbol table and string list */
symbol_table_t *global_symbols;
char **string_list;
size_t string_list_len;
size_t string_list_capacity;

static void find_globals ( void );
static void bind_names ( symbol_table_t *local_symbols, node_t *root );
static void print_symbol_table ( symbol_table_t *table, int nesting );
static void destroy_symbol_tables ( void );

static size_t add_string ( char* string );
static void print_string_list ( void );
static void destroy_string_list ( void );

/* External interface */

/* Creates a global symbol table, and local symbol tables for each function.
 * While building the symbol tables:
 *  - All usages of symbols are bound to their symbol table entries.
 *  - All strings are entered into the string_list
 */

void create_tables ( void )
{
    find_globals();
    for (size_t i = 0; i < root->n_children; i++){
        node_t *child = root->children[i];
        if(child->type == FUNCTION){
            symbol_t *symbol = symbol_hashmap_lookup(global_symbols->hashmap, child->children[0]->data);
            bind_names(symbol->function_symtable, child->children[2]);
        }
    }
   
}

/* Prints the global symbol table, and the local symbol tables for each function.
 * Also prints the global string list.
 * Finally prints out the AST again, with bound symbols.
 */
void print_tables ( void )
{
    print_symbol_table ( global_symbols, 0 );
    printf("\n == STRING LIST == \n");
    print_string_list ();
    printf("\n == BOUND SYNTAX TREE == \n");
    print_syntax_tree ();
}

/* Destroys all symbol tables and the global string list */
void destroy_tables ( void )
{
    destroy_symbol_tables ( );
    destroy_string_list ( );
}

/* Internal matters */

/* Goes through all global declarations in the syntax tree, adding them to the global symbol table.
 * When adding functions, local symbol tables are created, and symbols for the functions parameters are added.
 */

static void find_globals_helper(node_t *node);

static void find_globals ( void )
{
    global_symbols = symbol_table_init ( );
    find_globals_helper(root);
}

node_t *get_parents(node_t *node) {
    if (node->children[0]->type == IDENTIFIER_DATA) {
        return node;
    } else {
        for (size_t i = 0; i < node->n_children; i++) {
            node_t *child = node->children[i];
            node_t *result = get_parents(child);
            if (result != NULL) {
                return result;
            }
        }
    }
    return NULL;
}


static void find_globals_helper(node_t *node) {
    if (node == NULL) {
        return;
    }

    if (node->type == GLOBAL_DECLARATION) { 
        
        node_t *identifiers_parent = get_parents(node);
        symtype_t indentifiers_type = identifiers_parent->type == ARRAY_INDEXING ? SYMBOL_GLOBAL_ARRAY : SYMBOL_GLOBAL_VAR;
        for (size_t i = 0; i < identifiers_parent->n_children; i++)
        {
            node_t *identifier_data_node = identifiers_parent->children[i];
            if(identifier_data_node->type == IDENTIFIER_DATA){
                symbol_t *symbol = malloc(sizeof(symbol_t));
                symbol->name = (char*)identifier_data_node->data;
                symbol->type = indentifiers_type;
                symbol->node = identifier_data_node;
                symbol_table_insert(global_symbols, symbol);
            }
        }
       
    } else if (node->type == FUNCTION) {
        char *function_name = node->children[0]->data;

        symbol_t *symbol = malloc(sizeof(symbol_t));
        symbol->name = function_name;
        symbol->type = SYMBOL_FUNCTION;
        symbol->node = node;

        symbol_table_t *local_symbols = symbol_table_init();
        symbol->function_symtable = local_symbols;
        local_symbols->hashmap->backup = global_symbols->hashmap;

        symbol_table_insert(global_symbols, symbol);
        
        node_t *parameters = node->children[1];
        for (size_t i = 0; i < parameters->n_children; ++i) {
            char *parameter_name = parameters->children[i]->data;

            symbol_t *param_symbol = malloc(sizeof(symbol_t));

            param_symbol->name = parameter_name;
            param_symbol->type = SYMBOL_PARAMETER;
            param_symbol->node = parameters->children[i];
            param_symbol->function_symtable = local_symbols;

            symbol_table_insert(local_symbols, param_symbol);

        }
        
    }

    for (size_t i = 0; i < node->n_children; ++i) {
        find_globals_helper(node->children[i]);
    }
}

/* A recursive function that traverses the body of a function, and:
 *  - Adds variable declarations to the function's local symbol table.
 *  - Pushes and pops local variable scopes when entering and leaving blocks.
 *  - Binds all other IDENTIFIER_DATA nodes to the symbol it references.
 *  - Moves STRING_DATA nodes' data into the global string list,
 *    and replaces the node with a STRING_LIST_REFERENCE node.
 *    This node's data is a pointer to the char* stored in the string list.
 */

void bind_if_is_statement(node_t *node, symbol_table_t *local_symbols) {
    if (node->type == IDENTIFIER_DATA) {
        symbol_t *symbol = symbol_hashmap_lookup(local_symbols->hashmap, (char*)node->data);
        node->symbol = symbol;
    }
    else if(node->type == STRING_DATA){
        size_t position = add_string(node->data);
        node->type = STRING_LIST_REFERENCE;
        node->data = (void*) position;
    }
    else {  
        for (size_t i = 0; i < node->n_children; i++) {
            bind_if_is_statement(node->children[i], local_symbols);
        }
    }
}

static void bind_names ( symbol_table_t *local_symbols, node_t *node )
{

    if (node->type == IDENTIFIER_DATA) {
        
        char *identifier_name = (char*)node->data;

        symbol_t *symbol = malloc(sizeof(symbol_t));
        symbol->name = identifier_name;
        symbol->node = node;
        symbol->function_symtable = local_symbols; 
        symbol->type = SYMBOL_LOCAL_VAR;
        
        symbol_table_insert(local_symbols, symbol);
        
    }
    else if (node->type == STRING_DATA) {
        size_t position = add_string(node->data);

        node->type = STRING_LIST_REFERENCE;
        node->data = (void*) position;
    }
    else if (node->type >= 6 && node->type <= 11) {
        bind_if_is_statement(node, local_symbols);
    }
    else if (node->type == BLOCK){
        symbol_hashmap_t *block_scope = symbol_hashmap_init();
        block_scope->backup = local_symbols->hashmap;
        local_symbols->hashmap = block_scope;

        for (size_t i = 0; i < node->n_children; i++) {
            bind_names(local_symbols, node->children[i]);
        }

        local_symbols->hashmap = block_scope->backup;
        symbol_hashmap_destroy(block_scope);
    }
    else {
        for (size_t i = 0; i < node->n_children; i++) {
            bind_names(local_symbols, node->children[i]);
        }
    }
    
}

/* Prints the given symbol table, with sequence number, symbol names and types.
 * When printing function symbols, its local symbol table is recursively printed, with indentation.
 */
static void print_symbol_table ( symbol_table_t *table, int nesting )
{

    if (table == NULL) {
        return;
    }

    for (size_t i = 0; i < table->n_symbols; ++i) {
        symbol_t *symbol = table->symbols[i];
        
        for (int j = 0; j < nesting; ++j) {
            printf("    ");
        }

        printf("%zu: %s(%s)\n", symbol->sequence_number, SYMBOL_TYPE_NAMES[symbol->type], symbol->name);

        if (symbol->type == SYMBOL_FUNCTION && symbol->function_symtable != NULL) {
            print_symbol_table(symbol->function_symtable, nesting + 1);
        }
    }
}

static void destroy_symbol_table(symbol_table_t *table) {
    if (table) {
        for (size_t i = 0; i < table->n_symbols; i++) {
            symbol_t *symbol = table->symbols[i];
            if(symbol->type == SYMBOL_FUNCTION && symbol->function_symtable != NULL){
                destroy_symbol_table(symbol->function_symtable);
            }
            if (symbol) {
                free(symbol);
            }
        }
        if(table->hashmap != NULL){
            symbol_hashmap_destroy(table->hashmap);
        }
        free(table->symbols);
        free(table);
    }
}

/* Frees up the memory used by the global symbol table, all local symbol tables, and their symbols */
static void destroy_symbol_tables ( void )
{
    destroy_symbol_table(global_symbols);

}

/* Adds the given string to the global string list, resizing if needed.
 * Takes ownership of the string, and returns its position in the string list.
 */
static size_t add_string ( char *string )
{
    
    if (string_list_len >= string_list_capacity) {
        size_t new_capacity = string_list_capacity * 2 + 8;
        char **new_string_list = realloc(string_list, new_capacity * sizeof(char *));
        if (new_string_list == NULL) {
            fprintf(stderr, "Error: Failed to resize string list\n");
            exit(EXIT_FAILURE);
        }
        string_list = new_string_list;
        string_list_capacity = new_capacity;
    }

    string_list[string_list_len] = string;
    string_list_len++;

    return string_list_len - 1;
}

/* Prints all strings added to the global string list */
static void print_string_list ( void )
{
    for (size_t i = 0; i < string_list_len; ++i) {
        printf("%zu: %s\n", i, string_list[i]);
    }
}

/* Frees all strings in the global string list, and the string list itself */
static void destroy_string_list ( void )
{
    for (size_t i = 0; i < string_list_len; ++i) {
        free(string_list[i]);
    }
    free(string_list);
}
