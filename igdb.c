#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "database.h"
#define HANDLE_MAX_LENGTH 21
#define COMMENT_MAX_LENGTH 64
#define BUFFER_SIZE 256

void list_records(const Database *db) { 
	const int handleWidth = 20;
    	const int followersWidth = 10;
    	const int lastModifiedWidth = 20;
    	const int commentWidth = 30;
	
	// Header
    	printf("%-20s | %-10s | %-20s | %-30s\n",
           	"HANDLE", "FOLLOWERS", "LAST MODIFIED", "COMMENT");
    	// Static separator line
	printf("---------------------|------------|----------------------|--------------------------------|\n");	
	for (int i = 0; i < db->size; i++) {
    		char dateStr[21];
        	time_t timestamp = (time_t)db->records[i].date_last_modified;
        	struct tm *timeInfo = localtime(&timestamp);
		strftime(dateStr, sizeof(dateStr), "%Y-%m-%d %H:%M", timeInfo);

		printf("%-20.20s | %10lu | %-20s | %-30.30s\n",
           		db->records[i].handle,
               		db->records[i].follower_count,
               		dateStr,
               		db->records[i].comment);
	}

}

void add_record(Database *db, const char *input) {
    char handle[HANDLE_MAX_LENGTH];
    char buffer[BUFFER_SIZE];
    unsigned long followers = 0;
    char comment[COMMENT_MAX_LENGTH];
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    // Use sscanf to parse the handle and follower count from the line
    if (sscanf(input, "%255s %lu", buffer, &followers) < 2) {
        printf("Invalid input. Expected format: 'add <handle> <followers_count>'.\n");
        return;
    }

    // Check if handle exceeds max length
    if (strlen(buffer) >= HANDLE_MAX_LENGTH) {
    	printf("Invalid handle. The maximum length allowed is %d characters.\n", HANDLE_MAX_LENGTH - 1);
	return;
    }
    
    // Copy the valid handle into the handle variable
    strncpy(handle, buffer, HANDLE_MAX_LENGTH - 1);
    handle[HANDLE_MAX_LENGTH - 1] = '\0';

    // Check if handle already exists
    if (db_lookup(db, handle)) {
        printf("Error: Handle '%s' already exists.\n", handle);
        return;
    }

    // Prompt for comment
    printf("Enter comment for '%s': ", handle);
    nread = getline(&line, &len, stdin);
    if (nread == -1) {
        printf("Error reading comment.\n");
        free(line);
        return;
    }
    
    // Remove newline character from comment, if present
    line[strcspn(line, "\n")] = '\0';

    // Copy the comment ensuring it doesn't exceed max length
    strncpy(comment, line, COMMENT_MAX_LENGTH - 1);
    comment[COMMENT_MAX_LENGTH - 1] = '\0'; 
    free(line);

    if (strchr(comment, ',') != NULL) {
        printf("Error: Comment cannot contain commas.\n");
        return;
    }

    // Initialize Record
    Record newRecord = {
    .follower_count = followers,  
    .date_last_modified = (unsigned long)time(NULL)  
    };

    // Copy handle
    strncpy(newRecord.handle, handle, HANDLE_MAX_LENGTH - 1);
    newRecord.handle[HANDLE_MAX_LENGTH - 1] = '\0'; 

    // Copy Comment
    strncpy(newRecord.comment, comment, COMMENT_MAX_LENGTH - 1);
    newRecord.comment[COMMENT_MAX_LENGTH - 1] = '\0';    
    
    // Add record
    db_append(db, &newRecord);
    printf("Record '%s' added successfully.\n", handle);	

}

void update_record (Database *db, const char *input) { 
    char handle[HANDLE_MAX_LENGTH];
    unsigned long newFollowers;
    char comment[COMMENT_MAX_LENGTH];
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    // sscanf to parse the handle and follower count from the line
    if (sscanf(input, "%31s %lu", handle, &newFollowers) < 2) {
        printf("Invalid input. Expected format: 'update <handle> <followers_count>'.\n");
        return;
    }
    

    Record *recordToUpdate = db_lookup(db, handle);
    if (recordToUpdate == NULL) {
        printf("Error: No entry with handle '%s'.\n", handle);
        return;
    }

    // Prompt for comment
    printf("Enter comment for '%s': ", handle);
    line = NULL; 
    nread = getline(&line, &len, stdin);
    if (nread == -1) {
        printf("Error reading comment.\n");
        free(line);
        return;
    }

    line[strcspn(line, "\n")] = '\0';

    // Copy the comment ensuring it doesn't exceed max length
    strncpy(comment, line, COMMENT_MAX_LENGTH - 1);
    comment[COMMENT_MAX_LENGTH - 1] = '\0';
    free(line);
    
    if (strchr(comment, ',') != NULL) {
        printf("Error: Comment cannot contain commas.\n");
        return;
    }
 
    // Update the record
    recordToUpdate->follower_count = newFollowers;
    recordToUpdate->date_last_modified = (unsigned long)time(NULL);
    strncpy(recordToUpdate->comment, comment, sizeof(recordToUpdate->comment) - 1);
    recordToUpdate->comment[sizeof(recordToUpdate->comment) - 1] = '\0'; 
    printf("Record '%s' updated successfully.\n", handle);



}

int main_loop(Database * db) {
    char action[BUFFER_SIZE];  
    int unsavedChanges = 0;

    while (1) { 
	 printf("> ");

	// Read user input using fgets
        if (fgets(action, BUFFER_SIZE, stdin) == NULL) {
            printf("Error reading input or EOF reached.\n");
            continue;  
        }

        // Remove newline character, if present
        action[strcspn(action, "\n")] = '\0';

        // Check user action
        if (strcmp(action, "list") == 0) {
            list_records(db);
	}
	
	else if (strncmp(action, "add", 3) == 0) {
            add_record(db, action + 4);
	    unsavedChanges = 1; 
        }

	else if (strncmp(action, "update", 6) == 0) {
            update_record(db, action + 7);
	    unsavedChanges = 1;
    	}

	else if (strcmp(action, "save") == 0) {
            db_write_csv(db, "database.csv");
	    printf("Database has been successfully saved.\n");
	    unsavedChanges = 0; 
	}

	else if (strcmp(action, "exit") == 0) {
	    if (unsavedChanges == 1) { 
	 	printf("There are unsaved changes. Type 'save' to save or 'exit' to quit without saving.\n");	
		unsavedChanges = 0; 
            } 
	    
	    else {
                break;
            }
        } 
	
	else {
            printf("Unknown command.\n");
	}
    }	
	    
    return 0;
}

    
	

int main()
{
    Database db = db_create();      
    db_load_csv(&db, "database.csv");
    printf("Loaded %d records.\n", db.size); // Print the number of records
    return main_loop(&db);
	

}

