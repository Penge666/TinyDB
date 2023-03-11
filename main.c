
#include"input.h"
#include"prepare.h"
#include"pager.h"
#include"const.h"
#include"btree.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <limits.h>


typedef enum {
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;
MetaCommandResult do_meta_command(InputBuffer* input_buffer,Table* table) {
  if (strcmp(input_buffer->buffer, ".exit") == 0) {
  	close_input_buffer(input_buffer);
	db_close(table);
    exit(EXIT_SUCCESS);
  } else if (strcmp(input_buffer->buffer, ".btree") == 0) {
    printf("Tree:\n");
    print_leaf_node(get_page(table->pager, 0));
    return META_COMMAND_SUCCESS;
  } else if (strcmp(input_buffer->buffer, ".constants") == 0) {
    printf("Constants:\n");
    print_constants();
    return META_COMMAND_SUCCESS;
  } else {
    return META_COMMAND_UNRECOGNIZED_COMMAND;
  }
}
 
void print_prompt() { 
  printf("sqlite> ");
}

void print_constants() {
  printf("ROW_SIZE: %d\n", ROW_SIZE);
  printf("COMMON_NODE_HEADER_SIZE: %d\n", COMMON_NODE_HEADER_SIZE);
  printf("LEAF_NODE_HEADER_SIZE: %d\n", LEAF_NODE_HEADER_SIZE);
  printf("LEAF_NODE_CELL_SIZE: %d\n", LEAF_NODE_CELL_SIZE);
  printf("LEAF_NODE_SPACE_FOR_CELLS: %d\n", LEAF_NODE_SPACE_FOR_CELLS);
  printf("LEAF_NODE_MAX_CELLS: %d\n", LEAF_NODE_MAX_CELLS);
}
void print_leaf_node(void* node) {
  uint32_t num_cells = *leaf_node_num_cells(node);
  printf("leaf (size %d)\n", num_cells);
  for (uint32_t i = 0; i < num_cells; i++) {
    uint32_t key = *leaf_node_key(node, i);
    printf("  - %d : %d\n", i, key);
  }
}
 
 
int main(int argc, char* argv[]) {
   if (argc != 2) {
     printf("Must supply a database filename.\n");
     exit(EXIT_FAILURE);
   }
 
   char* filename = argv[1];
   Table* table = db_open(filename);
   InputBuffer* input_buffer = new_input_buffer();
   while (true) {
     print_prompt();
     read_input(input_buffer);
 
     if (input_buffer->buffer[0] == '.') {
       switch (do_meta_command(input_buffer,table)) {
         case (META_COMMAND_SUCCESS):
           continue;
         case (META_COMMAND_UNRECOGNIZED_COMMAND):
           printf("Error: Unrecognized command '%s'.\n", input_buffer->buffer);
           continue;
       }
     }
 
     Statement statement;
     switch (prepare_statement(input_buffer, &statement)) {
       case (PREPARE_SUCCESS):
         break;
       case (PREPARE_STRING_TOO_LONG):
           printf("Error: String is too long.\n");
           continue;
         case (PREPARE_NEGATIVE_ID):
           printf("Error: ID must be positive.\n");
           continue;
         case (PREPARE_ID_TOO_LONG):
           printf("Error: Id is too long.\n");
           continue;
         case (PREPARE_SYNTAX_ERROR):
           printf("Error: Syntax error.\n");
	  	   continue; 
         case (PREPARE_UNRECOGNIZED_STATEMENT):
           printf("Error: Unrecognized keyword at start of '%s'.\n",input_buffer->buffer);
           continue;
         case (PREPARE_ILLEGAL_ID):
           printf("Error: ILLEGAL ID.\n");
           continue;
      }
 
   
      switch (execute_statement(&statement, table)) {
        case (EXECUTE_SUCCESS):
          printf("Executed.\n");
          break;
        case (EXECUTE_PRIMARY_CHANGE):
          printf("Error: Can't change primary key.\n");
		  break; 
        case (EXECUTE_DUPLICATE_KEY):
          printf("Error: Duplicate key.\n");
          break;
        case (EXECUTE_NOT_FOUND):
	      printf("Error: Not found.\n");
		  break;   
        case (EXECUTE_TABLE_FULL):
          printf("Error: Table full.\n");
          break;
      }
   
  }
}