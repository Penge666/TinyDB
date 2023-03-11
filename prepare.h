#ifndef PREPARE_H
#define PREPARE_H

#include "input.h"
#include <stdio.h>
#include <stdint.h>
#include "const.h"

typedef enum { 
  STATEMENT_INSERT, 
  STATEMENT_SELECT,
  STATEMENT_DELETE,
  STATEMENT_UPDATE
} StatementType;

typedef struct {
  uint32_t id;
  char username[COLUMN_USERNAME_SIZE+1];
  char email[COLUMN_EMAIL_SIZE+1];
} Row;

typedef enum {
  ID,	
  USERNAME,
  EMAIL,
  ALL	
} SelectResult;

typedef struct {
  StatementType type;
  Row row_to_insert;
  SelectResult row_to_select; 
  Row row_to_delete;
  Row row_to_update_old;
  Row row_to_update_new;
} Statement;
typedef enum { 
  PREPARE_SUCCESS, 
  PREPARE_NEGATIVE_ID,
  PREPARE_ILLEGAL_ID,
  PREPARE_ID_TOO_LONG,
  PREPARE_SYNTAX_ERROR,
  PREPARE_STRING_TOO_LONG,
  PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

PrepareResult prepare_insert(InputBuffer* input_buffer, Statement* statement);
PrepareResult prepare_select(InputBuffer* input_buffer,Statement* statement);
PrepareResult prepare_delete(InputBuffer* input_buffer,Statement* statement);
PrepareResult prepare_update(InputBuffer* input_buffer,Statement* statement);
PrepareResult prepare_statement(InputBuffer* input_buffer,Statement* statement);
#endif