#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>


// need a way to define "streaks"
// they should have days
//
// -> a TUI is just controlled printing
#define MAX_ITEMS 32
#define JOURNAL_SIZE 512

struct termios original;

struct Item {
	int id;
	char entry[128];
	int checked;
};

// TODO: pull DB func into separate file and make header file
void drawScreen(struct Item *items, int item_count, int selected, char *journal) {
	// \033 == ESC
	// \033[2J clears the screen
	// \033[H moves the cursor to the home position
	printf("\033[2J\033[H");
	printf("\033[1m-----------\033[0m\r\n"); // bold it
	printf("\033[1m| Daybook |\033[0m\r\n"); // bold it
	printf("\033[1m-----------\033[0m\r\n"); // bold it
	
	printf("\r\nHabits\r\n\n");

	for (int i = 0; i < item_count; i++) {
		printf("%s[%c] %s\r\n",
				selected == i ? "> " : " ",
				items[i].checked ? 'x' : ' ',
				items[i].entry);
	}

	printf("\r\n");
	printf("Journal\r\n");
	printf("-------\r\n");

	if (journal[0] == '\0') {
	printf("(empty)\r\n");
	} else {
	printf("%s\r\n", journal);
	}

	printf("\r\n");
	printf("j/k move · space toggle · e edit journal · q quit\r\n");
	fflush(stdout);

}

void disableRaw() {
	printf("\033[?25h"); // show cursor
	fflush(stdout);
	printf("\r\n");
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}

void enableRaw() {
	struct termios raw = original;
	// these are bitwise 
	// ~ == bitwise NOT
	// | is bitwise OR
	// & is bitwise AND
	// so we are turning those bits OFF against the c_lflag bitmap
	raw.c_lflag &= ~(ICANON | ECHO);
	raw.c_cc[VMIN] = 1;
	raw.c_cc[VTIME] = 0;
	printf("\033[?25l"); // hide cursor
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void setupTerminal() {
	tcgetattr(STDIN_FILENO, &original);
	atexit(disableRaw);	
	enableRaw();
}


int loadItems(sqlite3 *db, struct Item *items, int max_items) {
    sqlite3_stmt *stmt;
    int count = 0;

    const char *sql = "SELECT id, entry, checked FROM items ORDER BY id;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare loadItems: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_items) {
	int id = sqlite3_column_int(stmt, 0);
        const unsigned char *entry = sqlite3_column_text(stmt, 1);
        int checked = sqlite3_column_int(stmt, 2);

	items[count].id = id;
        snprintf(items[count].entry, sizeof(items[count].entry), "%s", entry);
        items[count].checked = checked;

        count++;
    }

    sqlite3_finalize(stmt);
    return count;
}

// TODO: save check on keypress
// TODO: make item entries dated
void saveItems(sqlite3 *db, struct Item *items, int item_count) {
    sqlite3_stmt *stmt;

    const char *sql = "UPDATE items SET checked = ? WHERE id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare saveItems: %s\n", sqlite3_errmsg(db));
        return;
    }

    for (int i = 0; i < item_count; i++) {
        sqlite3_bind_int(stmt, 1, items[i].checked);
        sqlite3_bind_int(stmt, 2, items[i].id);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fprintf(stderr, "Failed to update item: %s\n", sqlite3_errmsg(db));
        }

        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
    }

    sqlite3_finalize(stmt);
}

void saveJournal(sqlite3 *db, const char *journal) {
    if (journal[0] == '\0') {
        return;
    }

    sqlite3_stmt *stmt;

    const char *sql = "INSERT INTO journal (entry) VALUES (?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare saveJournal: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, journal, -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Failed to save journal: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

void loadLatestJournal(sqlite3 *db, char *journal, int size) {
    sqlite3_stmt *stmt;

    const char *sql = "SELECT entry FROM journal ORDER BY created_at DESC, id DESC LIMIT 1;";

    journal[0] = '\0';

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare loadLatestJournal: %s\n", sqlite3_errmsg(db));
        return;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *entry = sqlite3_column_text(stmt, 0);
        snprintf(journal, size, "%s", entry);
    }

    sqlite3_finalize(stmt);
}

void editJournal(char *journal, int size) {
    disableRaw();

    printf("\r\nJournal entry:\r\n> ");
    fflush(stdout);

    if (fgets(journal, size, stdin) != NULL) {
	// remove trailing \n from enter press
        journal[strcspn(journal, "\n")] = '\0';
    }

    enableRaw();
}

int getItemCount(sqlite3 *db) {
    sqlite3_stmt *stmt;
    int count = 0;

    const char *sql = "SELECT COUNT(*) FROM items;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
    }

    sqlite3_finalize(stmt);
    return count;
}

void seedDefaultItems(sqlite3 *db) {
    if (getItemCount(db) > 0) {
        return;
    }

    const char *sql =
        "INSERT INTO items (entry, checked) VALUES "
        "('Study C', 0),"
        "('Run', 0),"
        "('Read', 0);";

    char *err_msg = NULL;

    if (sqlite3_exec(db, sql, 0, 0, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "SQL error seeding items: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
}

void initDb(sqlite3 *db) {
    char *err_msg = NULL;

    const char *items_sql =
        "CREATE TABLE IF NOT EXISTS items ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "entry TEXT NOT NULL,"
        "checked INTEGER NOT NULL DEFAULT 0"
        ");";

    const char *journal_sql =
        "CREATE TABLE IF NOT EXISTS journal ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "entry TEXT NOT NULL,"
        "created_at TEXT DEFAULT CURRENT_TIMESTAMP"
        ");";

    if (sqlite3_exec(db, items_sql, 0, 0, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "SQL error creating items table: %s\n", err_msg);
        sqlite3_free(err_msg);
    }

    if (sqlite3_exec(db, journal_sql, 0, 0, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "SQL error creating journal table: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
}

// TODO: [ ] add support for custom items
// TODO: [x] figure out how to remove cursor
int main(int argc, char* argv[]) {
	sqlite3 *db;

	if (sqlite3_open("daybook.db", &db) != SQLITE_OK) {
        	fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        	return 1;
    	}

	initDb(db);
	seedDefaultItems(db);

	setupTerminal();
	
	int selected = 0;
	struct Item items[MAX_ITEMS];
	char journal[JOURNAL_SIZE] = "";

	int item_count = loadItems(db, items, MAX_ITEMS);
	loadLatestJournal(db, journal, JOURNAL_SIZE);

	while (1) {
		drawScreen(items, item_count, selected, journal);
		int c = getchar();
		if (c == 'q') {
			saveItems(db, items, item_count);
			saveJournal(db, journal);
			break;
		}
		if (c == 'j') {
			if (selected<item_count-1) selected++;
		}
		if (c == 'k') {
			if (selected > 0) selected--;
		}
		if (c == ' ') {
			items[selected].checked = !items[selected].checked;
		}
		if (c == 'e') {
			editJournal(journal, JOURNAL_SIZE);
		}
	}

	sqlite3_close(db);
	return 0;
}
