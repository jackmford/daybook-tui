#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>


// need a way to define "streaks"
// they should have days
//
// -> a TUI is just controlled printing
//
struct termios original;

struct Item {
	char *entry;
	int selected;
};

void drawScreen(int selected) {
	// \033 == ESC
	// \033[2J clears the screen
	// \033[H moves the cursor to the home position
	printf("\033[2J\033[H");
	printf("Daybook\r\n");
	printf("%s[ ] Study C\r\n", selected == 0 ? "> " : " ");
	printf("%s[ ] Run\r\n", selected == 1 ? "> " : " ");
	printf("%s[ ] Read\r\n", selected == 2 ? "> " : " ");
	fflush(stdout);

}

void disableRaw() {
	printf("\033[?25h"); // show cursor
	fflush(stdout);
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


// TODO: [ ] add support for custom items
// TODO: [ ] figure out how to remove cursor
int main(int argc, char* argv[]) {
	enableRaw();
	int selected = 0;
	
	// messing with custom items
	// TODO: create an array of items and draw those to screen
	struct Item t = {"Read", 0};
	while (1) {
		drawScreen(selected);
		int c = getchar();
		if (c == 'q') {
			break;
		}
		if (c == 'j') {
			// TODO: fix this to be of len items
			if (selected<2) selected++;
		}
		if (c == 'k') {
			if (selected > 0) selected--;
		}
		// TODO: add an enter that draws [x]
		// There will need to be some amount of state here to save presses
	}
	printf("%s %d", t.entry, t.selected);
	printf("\r\n");
	return 0;
}
