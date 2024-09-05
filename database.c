#include "database.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

Database db_create() {
        Database db;
        db.size = 0;
        db.capacity = 4;
        db.records = (Record*)malloc(db.capacity * sizeof(Record));
        return db;
}

void db_append(Database * db, Record const * item) {
        // If resizing needed
        if (db->size == db->capacity) {
                db->capacity *= 2;
                db->records = (Record*)realloc(db->records, db->capacity * sizeof(Record));
    }

    // Append item
    db->records[db->size] = *item;
    db->size++;
}

Record* db_index(Database * db, int index) {
        if (index >= 0 && index < db->size) {
                return &db->records[index];
        } else {
                // Index out of bounds
                return NULL;
        }
}

Record* db_lookup(Database * db, char const * handle) {
        for (int i = 0; i < db->size; i++) {
		if (strcmp(db->records[i].handle, handle) == 0) 
                        return &db->records[i];
               	
	}

        // Handle not found
        return NULL;
}


void db_free(Database *db) {
	free(db->records);
	db->records = NULL;
	db->size = 0;
	db->capacity = 0;
}

Record parse_record(char const *line) {
	Record record = {0}; 
    	char *tempLine = strdup(line); // Duplicate the line
    	char *token;
	char *endptr;

	// Handle
    	token = strtok(tempLine, ",");
    	if (token != NULL) {
        	strncpy(record.handle, token, sizeof(record.handle) - 1);
        	record.handle[sizeof(record.handle) - 1] = '\0'; 
    	}

    	// Follower count
    	token = strtok(NULL, ",");
    	if (token != NULL) 
		record.follower_count = strtoul(token, NULL, 10);
	
	// Comment
    	token = strtok(NULL, "\"");
    	if (token != NULL) {
        	strncpy(record.comment, token, sizeof(record.comment) - 1);
        	endptr = strtok(NULL, "\"");
        	if (endptr != NULL && *endptr != ',') {
            		strncat(record.comment, ",", sizeof(record.comment) - strlen(record.comment) - 1);
            		strncat(record.comment, endptr, sizeof(record.comment) - strlen(record.comment) - 1);
        	}
        	record.comment[sizeof(record.comment) - 1] = '\0'; 
    	}

	// Date last modified
	token = strtok(NULL, ",");
	if (token != NULL) {
    		errno = 0;
		record.date_last_modified = strtoul(token, NULL, 10);
    	}

    	free(tempLine); 
    	return record;

}


void db_load_csv(Database *db, char const *path) {
	FILE *file = fopen(path, "r");
	if (file == NULL) {
		fprintf(stderr, "Failed to open the file: %s\n", path); 
		return;
	}

	char *line = NULL;
    	size_t len = 0;
    	while (getline(&line, &len, file) != -1) {
        	Record record = parse_record(line);
        	db_append(db, &record);
    	}

    	free(line); 
    	fclose(file);

}

void db_write_csv(Database * db, char const * path) {
	FILE *file = fopen(path, "w"); 
    	if (file == NULL) {
        	fprintf(stderr, "Failed to open the file for writing: %s\n", path);
        	return;
    	}
	
	for (int i = 0; i < db->size; i++) {
        Record *record = &db->records[i];
        // Writing each record as a CSV line. 
        fprintf(file, "%s,%lu,\"%s\",%lu\n", 
		record->handle, 
                record->follower_count, 
                record->comment, 
                record->date_last_modified);
    }

    fclose(file);	


}

