#include "prepare.h"
#include <string.h>
PrepareResult prepare_statement(InputBuffer* input_buffer,Statement* statement) {
  if (strncmp(input_buffer->buffer, "insert",6) == 0) {
    return prepare_insert(input_buffer, statement);
  }
  else if(strncmp(input_buffer->buffer,"select",6)==0){
    return prepare_select(input_buffer, statement);
  }
  else if(strncmp(input_buffer->buffer,"delete",6)==0){
   	return prepare_delete(input_buffer, statement);  	   
  }  
  else if(strncmp(input_buffer->buffer,"update",6)==0){
   	return prepare_update(input_buffer, statement);
  }
  else return  PREPARE_UNRECOGNIZED_STATEMENT;
}
 
PrepareResult prepare_insert(InputBuffer* input_buffer, Statement* statement) {
  statement->type = STATEMENT_INSERT;
 
  char* keyword = strtok(input_buffer->buffer, " ");
  char* id_string = strtok(NULL, " ");
  char* username = strtok(NULL, " ");
  char* email = strtok(NULL, " ");
  char* check=strtok(NULL," ");
  if (id_string == NULL || username == NULL || email == NULL||check!=NULL) {
    return PREPARE_SYNTAX_ERROR;
  }
  for(uint32_t i=0;i<strlen(id_string);i++){
    if(isdigit(id_string[i])==0){
      return PREPARE_ILLEGAL_ID;
    }
  }
  int id = atoi(id_string);
  if(strlen(id_string)>=10){
  	return PREPARE_ID_TOO_LONG;
  }
  if (id < 0) {
    return PREPARE_NEGATIVE_ID;
  }
  if (strlen(username) > COLUMN_USERNAME_SIZE) {
    return PREPARE_STRING_TOO_LONG;
  }
  if (strlen(email) > COLUMN_EMAIL_SIZE) {
    return PREPARE_STRING_TOO_LONG;
  }
 
  statement->row_to_insert.id = id;
  strcpy(statement->row_to_insert.username, username);
  strcpy(statement->row_to_insert.email, email);
 
  return PREPARE_SUCCESS;
}
PrepareResult prepare_select(InputBuffer* input_buffer,Statement* statement){
  statement->type=STATEMENT_SELECT;
	
  char *keyword =strtok(input_buffer->buffer," ");
  char * args=strtok(NULL," ");
  char* check=strtok(NULL," ");
  if(check!=NULL){
	return PREPARE_SYNTAX_ERROR;
  }
  if(args==NULL){
	statement->row_to_select=ALL;
	return PREPARE_SUCCESS;
  }
  if(strcmp(args,"id")==0){
	statement->row_to_select=ID;
  }
  else if(strcmp(args,"username")==0){
	statement->row_to_select=USERNAME;
  }
  else if(strcmp(args,"email")==0){
	statement->row_to_select=EMAIL;
  }
  else return PREPARE_SYNTAX_ERROR;
	
  return PREPARE_SUCCESS;
}
 
PrepareResult prepare_delete(InputBuffer* input_buffer,Statement* statement){
  statement->type = STATEMENT_DELETE;
  char* keyword = strtok(input_buffer->buffer, " ");
  char* id_string = strtok(NULL, " ");
 
  char* check=strtok(NULL," ");
  if(check!=NULL){
	return PREPARE_SYNTAX_ERROR;
  }
  if (id_string == NULL ) {
    return PREPARE_SYNTAX_ERROR;
  }
 
  int id = atoi(id_string);
  for(uint32_t i=0;i<strlen(id_string);i++){
    if(isdigit(id_string[i])==0){
      return PREPARE_ILLEGAL_ID;
    }
  }
  if(strlen(id_string)>=10)
    return PREPARE_ID_TOO_LONG;
  
 
  statement->row_to_delete.id = id;
  
 
  return PREPARE_SUCCESS;
}
 
PrepareResult prepare_update(InputBuffer* input_buffer,Statement* statement){
  statement->type = STATEMENT_UPDATE;
  char* keyword = strtok(input_buffer->buffer, " ");
  char* id_string_old = strtok(NULL, " ");
  char* username_old = strtok(NULL, " ");
  char* email_old = strtok(NULL, " ");
  char* id_string_new = strtok(NULL, " ");
  char* username_new = strtok(NULL, " ");
  char* email_new = strtok(NULL, " ");
  char* check=strtok(NULL," ");
  if(check!=NULL){
	return PREPARE_SYNTAX_ERROR;
  }
  if (id_string_old == NULL || username_old == NULL || email_old == NULL) {
    return PREPARE_SYNTAX_ERROR;
  }
    
  if (id_string_new == NULL || username_new == NULL || email_new == NULL) {
    return PREPARE_SYNTAX_ERROR;
  }
    
  int id_old = atoi(id_string_old);
  int id_new = atoi(id_string_new);
  for(uint32_t i=0;i<strlen(id_string_old);i++){
    if(isdigit(id_string_old[i])==0){
      return PREPARE_ILLEGAL_ID;
    }
  }
  for(uint32_t i=0;i<strlen(id_string_new);i++){
    if(isdigit(id_string_new[i])==0){
      return PREPARE_ILLEGAL_ID;
    }
  }
  if(strlen(id_string_old)>=10||strlen(id_string_new)>=10)
    return PREPARE_ID_TOO_LONG;
  if (id_new < 0) {
    return PREPARE_NEGATIVE_ID;
  }
  if (strlen(username_old) > COLUMN_USERNAME_SIZE) {
    return PREPARE_STRING_TOO_LONG;
  }
  if (strlen(email_old) > COLUMN_EMAIL_SIZE) {
    return PREPARE_STRING_TOO_LONG;
  }
  if (strlen(username_new) > COLUMN_USERNAME_SIZE) {
    return PREPARE_STRING_TOO_LONG;
  }
  if (strlen(email_new) > COLUMN_EMAIL_SIZE) {
    return PREPARE_STRING_TOO_LONG;
  }
 
  statement->row_to_update_old.id = id_old;
  strcpy(statement->row_to_update_old.username, username_old);
  strcpy(statement->row_to_update_old.email, email_old);
  statement->row_to_update_new.id = id_new;
  strcpy(statement->row_to_update_new.username, username_new);
  strcpy(statement->row_to_update_new.email, email_new);
  return PREPARE_SUCCESS;
}
 
