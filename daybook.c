#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>


// need a way to define "streaks"
// they should have days
//
// -> a TUI is just controlled printing
#define MAX_ITEMS 32

struct termios original;

struct Item {
	char entry[128];
	int checked;
};

void drawScreen(struct Item *items, int item_count, int selected) {
	// \033 == ESC
	// \033[2J clears the screen
	// \033[H moves the cursor to the home position
	printf("\033[2J\033[H");
	printf("\033[1m-----------\033[0m\r\n"); // bold it
	printf("\033[1m| Daybook |\033[0m\r\n"); // bold it
	printf("\033[1m-----------\033[0m\r\n"); // bold it

	for (int i = 0; i < item_count; i++) {
		printf("%s[%c] %s\r\n",
				selected == i ? "> " : " ",
				items[i].checked ? 'x' : ' ',
				items[i].entry);
	}
	fflush(stdout);

}

void disableRaw() {
	printf("\033[?25h"); // show cursor
	fflush(stdout);
	printf("\r\n");
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}

void enableRaw() {
	tcgetattr(STDIN_FILENO, &original);
	atexit(disableRaw);	
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

int loadItems(const char *filename, struct Item *items, int max_items) {
	FILE *file = fopen(filename, "r");

	if (file == NULL) {
		return 0;
	}

	char line[256];
	int count = 0;

	fgets(line, sizeof(line), file);

	while(fgets(line, sizeof(line), file) != NULL && count < max_items) {
		char entry[128];
		int checked;

		if (sscanf(line, "%127[^,],%d", entry, &checked) == 2) {
			// write string data from entry to items[entry]
			snprintf(items[count].entry, sizeof(items[count].entry), "%s", entry);
			items[count].checked = checked;
			count++;
		}
	}
	fclose(file);
	return count;
}

void saveItems(const char *filename, struct Item *items, int item_count) {
	FILE *file = fopen(filename, "w");

	if (file == NULL) {
		return;
	}

	fprintf(file, "entry,checked\n");

	for (int i = 0; i < item_count; i++) {
		fprintf(file, "%s,%d\n", items[i].entry, items[i].checked);
	}

	fclose(file);
}



// TODO: [ ] add support for custom items
// TODO: [x] figure out how to remove cursor
int main(int argc, char* argv[]) {
	enableRaw();
	int selected = 0;
	struct Item items[MAX_ITEMS];

	int item_count = loadItems("dayitems.csv", items, MAX_ITEMS);

	while (1) {
		drawScreen(items, item_count, selected);
		int c = getchar();
		if (c == 'q') {
			saveItems("dayitems.csv", items, item_count);
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
	}

	return 0;
}
