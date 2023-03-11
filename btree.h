#ifndef BTREE_H
#define BTREE_H
#include"input.h"
#include"prepare.h"
#include"pager.h"
#include"const.h"
#include"stdbool.h"
#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

typedef enum { 
  EXECUTE_SUCCESS,
  EXECUTE_TABLE_FULL,
  EXECUTE_DUPLICATE_KEY,
  EXECUTE_NOT_FOUND,
  EXECUTE_PRIMARY_CHANGE
} ExecuteResult;

typedef struct {
  Pager* pager;
  uint32_t root_page_num;
} Table;

typedef struct {
   Table* table;
   uint32_t page_num;
   uint32_t cell_num;
   bool end_of_table;  
} Cursor;

typedef enum {
  NODE_INTERNAL,
  NODE_LEAF 
} NodeType;

 
const uint32_t ID_OFFSET = 0;
// const uint32_t PAGE_SIZE = 4096;
const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;
const uint32_t NODE_TYPE_SIZE = sizeof(uint8_t);
const uint32_t NODE_TYPE_OFFSET = 0;
const uint32_t IS_ROOT_SIZE = sizeof(uint8_t);
const uint32_t IS_ROOT_OFFSET = NODE_TYPE_SIZE;
const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);
const uint32_t PARENT_POINTER_OFFSET = IS_ROOT_OFFSET + IS_ROOT_SIZE;
const uint8_t COMMON_NODE_HEADER_SIZE =NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;
 
const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = COMMON_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_HEADER_SIZE =COMMON_NODE_HEADER_SIZE + LEAF_NODE_NUM_CELLS_SIZE;
 
const uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_KEY_OFFSET = 0;
const uint32_t LEAF_NODE_VALUE_SIZE = ROW_SIZE;
const uint32_t LEAF_NODE_VALUE_OFFSET =LEAF_NODE_KEY_OFFSET + LEAF_NODE_KEY_SIZE;
const uint32_t LEAF_NODE_CELL_SIZE = LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE;
const uint32_t LEAF_NODE_SPACE_FOR_CELLS = PAGE_SIZE - LEAF_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_MAX_CELLS =LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;
const uint32_t LEAF_NODE_LESS_CELLS =LEAF_NODE_MAX_CELLS/2; 
void free_table(Table* table) ;


void db_close(Table* table);
Cursor* table_start(Table* table);
Cursor* table_end(Table* table);
void* cursor_value(Cursor* cursor);
void cursor_advance(Cursor* cursor);
uint32_t* leaf_node_num_cells(void* node);
void* leaf_node_cell(void* node, uint32_t cell_num);
uint32_t* leaf_node_key(void* node, uint32_t cell_num);
void* leaf_node_value(void* node, uint32_t cell_num);
void leaf_node_insert(Cursor* cursor, uint32_t key, Row* value);
void print_constants();
void print_leaf_node(void* node);
Cursor* table_find(Table* table, uint32_t key);
Cursor* leaf_node_find(Table* table, uint32_t page_num, uint32_t key);
NodeType get_node_type(void* node);
void set_node_type(void* node, NodeType type);
void leaf_node_delete(Cursor* cursor, uint32_t key, Row* value);
Table* db_open(const char* filename) {
  Pager* pager = pager_open(filename);
  Table* table = malloc(sizeof(Table));
  table->pager = pager;
  table->root_page_num = 0;
  if (pager->num_pages == 0) {
    void* root_node = get_page(pager, 0);
    initialize_leaf_node(root_node);
  }
  return table;
}
 
 

void db_close(Table* table) {
  Pager* pager = table->pager;
 
  for (uint32_t i = 0; i < pager->num_pages; i++) {
    if (pager->pages[i] == NULL) {
      continue;
    }
    pager_flush(pager, i);
    free(pager->pages[i]);
    pager->pages[i] = NULL;
  }
 
  int result = close(pager->file_descriptor);
  if (result == -1) {
    printf("Error closing db file.\n");
    exit(EXIT_FAILURE);
  }
  for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
    void* page = pager->pages[i];
    if (page) {
      free(page);
      pager->pages[i] = NULL;
    }
  }
  free(pager);
  free(table);
}
 
 

 
void serialize_row(Row* source, void* destination) {
  memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
  memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
  memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}
 
void deserialize_row(void* source, Row* destination) {
  memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
  memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
  memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}
 
ExecuteResult execute_insert(Statement* statement, Table* table) {
  Row row;
  void* node = get_page(table->pager, table->root_page_num);
  uint32_t num_cells = (*leaf_node_num_cells(node));
  if (num_cells >= LEAF_NODE_MAX_CELLS) {
    return EXECUTE_TABLE_FULL;
  }
  Row* row_to_insert = &(statement->row_to_insert);
  uint32_t key_to_insert = row_to_insert->id;
  Cursor* cursor = table_find(table, key_to_insert);
  if (cursor->cell_num < num_cells) {
    uint32_t key_at_index = *leaf_node_key(node, cursor->cell_num);
    if (key_at_index == key_to_insert) {
      return EXECUTE_DUPLICATE_KEY;
    }
  }
  
  leaf_node_insert(cursor, row_to_insert->id, row_to_insert);
  free(cursor);
  return EXECUTE_SUCCESS;
}
 
ExecuteResult execute_select(Statement* statement, Table* table) {
  Cursor* cursor = table_start(table);
  Row row;
  void* node = get_page(table->pager, table->root_page_num);
  uint32_t num_cells = *leaf_node_num_cells(node);
  for(uint32_t i=0;i<num_cells;i++) {
    cursor->cell_num =i;
    deserialize_row(leaf_node_value(node, cursor->cell_num),&row);
    if(statement->row_to_select==ID){
      printf("(%d)\n", row.id);
    } 
    else if(statement->row_to_select==USERNAME){
      printf("(%s)\n", row.username);
    }
    else if(statement->row_to_select==EMAIL){
      printf("(%s)\n", row.email);
    }
    else if(statement->row_to_select==ALL){
      printf("(%d, %s, %s)\n", row.id, row.username, row.email);
    }
    
  } 
  free(cursor);
  return EXECUTE_SUCCESS;
}
 
ExecuteResult execute_delete(Statement* statement, Table* table) {
 
  bool flag=false; 
 
  void* node = get_page(table->pager, table->root_page_num);
  uint32_t num_cells = (*leaf_node_num_cells(node));
  Row* row_to_delete = &(statement->row_to_delete);
  uint32_t key_to_delete= row_to_delete->id;
  Cursor* cursor = table_find(table, key_to_delete);
  if (cursor->cell_num < num_cells) {
    uint32_t key_at_index = *leaf_node_key(node, cursor->cell_num);
    if (key_at_index == key_to_delete) {
      flag=true;
    }
  }
  if(flag==false)
  return EXECUTE_NOT_FOUND;
  leaf_node_delete(cursor, row_to_delete->id, row_to_delete);
  free(cursor);
  return EXECUTE_SUCCESS;
}
 
ExecuteResult execute_update(Statement* statement, Table* table) {
  Row row;
  bool flag=false; 
  Row* row_to_update = &(statement->row_to_update_old);
  void* node = get_page(table->pager, table->root_page_num);
  uint32_t key_to_update = row_to_update->id;
  Cursor* cursor = table_find(table, key_to_update);
  Row* row_to_update_old = &(statement->row_to_update_old);
 
  uint32_t num_cells = *leaf_node_num_cells(node);
  if(statement->row_to_update_new.id!=statement->row_to_update_old.id){
    return EXECUTE_PRIMARY_CHANGE;
  }
 
  for(uint32_t i=0;i<num_cells;i++) {
    cursor->cell_num =i;
    deserialize_row(leaf_node_value(node, cursor->cell_num),&row);
		if(strcmp(statement->row_to_update_old.username,row.username )==0&&strcmp(statement->row_to_update_old.email,row.email)==0)
		{  
           flag=true;
		   strcpy(row.username,statement->row_to_update_new.username);	
           strcpy(row.email,statement->row_to_update_new.email);
           serialize_row(&row, leaf_node_value(node, cursor->cell_num));
            return EXECUTE_SUCCESS;
        }
        serialize_row(&row, leaf_node_value(node, cursor->cell_num));
        cursor_advance(cursor);
    }
    free(cursor);
    return EXECUTE_NOT_FOUND;
	
}
 
ExecuteResult execute_statement(Statement* statement, Table* table) {
  switch (statement->type) {
    case (STATEMENT_INSERT):
      return execute_insert(statement, table);
    case (STATEMENT_SELECT):
      return execute_select(statement, table);
    case (STATEMENT_DELETE):
	  return execute_delete(statement, table);  
	case (STATEMENT_UPDATE):
	  return execute_update(statement, table);
  }
 
}
Cursor* table_start(Table* table) {
  Cursor* cursor = malloc(sizeof(Cursor));
  cursor->table = table;
  cursor->page_num = table->root_page_num;
  cursor->cell_num = 0;
  void* root_node = get_page(table->pager, table->root_page_num);
  uint32_t num_cells = *leaf_node_num_cells(root_node);
  cursor->end_of_table = (num_cells == 0);
 
  return cursor;
}
 
void* cursor_value(Cursor* cursor) {
 
  uint32_t page_num = cursor->page_num;
  void* page = get_page(cursor->table->pager, page_num);
 
  return leaf_node_value(page, cursor->cell_num);
}
 
void cursor_advance(Cursor* cursor) {
  uint32_t page_num = cursor->page_num;
  void* node = get_page(cursor->table->pager, page_num);
  cursor->cell_num += 1;
  if (cursor->cell_num >= (*leaf_node_num_cells(node))) {
    cursor->end_of_table = true;
  }
}
 
uint32_t* leaf_node_num_cells(void* node) {
  return node + LEAF_NODE_NUM_CELLS_OFFSET;
}
 
void* leaf_node_cell(void* node, uint32_t cell_num) {
  return node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}
 
uint32_t* leaf_node_key(void* node, uint32_t cell_num) {
  return leaf_node_cell(node, cell_num);
}
 
void* leaf_node_value(void* node, uint32_t cell_num) {
  return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}
void initialize_leaf_node(void* node) {
  set_node_type(node, NODE_LEAF);
  *leaf_node_num_cells(node) = 0; 
}
 
void leaf_node_insert(Cursor* cursor, uint32_t key, Row* value) {
  void* node = get_page(cursor->table->pager, cursor->page_num);
 
  uint32_t num_cells = *leaf_node_num_cells(node);
  if (num_cells >= LEAF_NODE_MAX_CELLS) {
    printf("Need to implement splitting a leaf node.\n");
    exit(EXIT_FAILURE);
  }
  if (cursor->cell_num < num_cells) {
    for (uint32_t i = num_cells; i > cursor->cell_num; i--) {
      memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1),
             LEAF_NODE_CELL_SIZE);
    }
  }
 
  *(leaf_node_num_cells(node)) += 1;
  *(leaf_node_key(node, cursor->cell_num)) = key;
  serialize_row(value, leaf_node_value(node, cursor->cell_num));
}
 
void leaf_node_delete(Cursor* cursor, uint32_t key, Row* value) {
  void* node = get_page(cursor->table->pager, cursor->page_num);
 
  uint32_t num_cells = *leaf_node_num_cells(node);
  if (num_cells <= LEAF_NODE_LESS_CELLS) {
    printf("Need to implement inserting a leaf node.\n");
    exit(EXIT_FAILURE);
  }
  
  if (cursor->cell_num<num_cells) {
    for (uint32_t  i = cursor->cell_num; i <=num_cells;i++) {
    if(*(leaf_node_key(node, i)) > key){
	
      memcpy(leaf_node_cell(node, i-1), leaf_node_cell(node, i),LEAF_NODE_CELL_SIZE);
        }
    }
}
 
  *(leaf_node_num_cells(node)) -= 1;
  cursor->cell_num-=1; 
  
}
 
Cursor* table_find(Table* table, uint32_t key) {
  uint32_t root_page_num = table->root_page_num;
  void* root_node = get_page(table->pager, root_page_num);
  return leaf_node_find(table, root_page_num, key);
  if (get_node_type(root_node) == NODE_LEAF) {
    return leaf_node_find(table, root_page_num, key);
  } else {
    printf("Need to implement searching an internal node\n");
    exit(EXIT_FAILURE);
  }
}
 
Cursor* leaf_node_find(Table* table, uint32_t page_num, uint32_t key) {
  void* node = get_page(table->pager, page_num);
  uint32_t num_cells = *leaf_node_num_cells(node);
 
  Cursor* cursor = malloc(sizeof(Cursor));
  cursor->table = table;
  cursor->page_num = page_num;
 
  uint32_t min_index = 0;
  uint32_t one_past_max_index = num_cells;
  while (one_past_max_index != min_index) {
    uint32_t index = (min_index + one_past_max_index) / 2;
    uint32_t key_at_index = *leaf_node_key(node, index);
    if (key == key_at_index) {
      cursor->cell_num = index;
      return cursor;
    }
    if (key < key_at_index) {
      one_past_max_index = index;
    } else {
      min_index = index + 1;
    }
  }
  cursor->cell_num = min_index;
  return cursor;
}
 
NodeType get_node_type(void* node) {
  uint8_t value = *((uint8_t*)(node + NODE_TYPE_OFFSET));
  return (NodeType)value;
}
 
void set_node_type(void* node, NodeType type) {
  uint8_t value = type;
  *((uint8_t*)(node + NODE_TYPE_OFFSET)) = value;
}
#endif