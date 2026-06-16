# log

## work session #1

- added simple Item struct to dynamically print items onto the screen
- used \033[1m to bold Daybook text and \033[0m ANSI escape code to clear formatting after the bolding

- TODO: implement some kind of "item library" file system that makes it truly dynamic

## work session #2

- added a csv file that we can load and save items to, preserving their state
- used fgets() to read the lines
  - sscanf() to scan the data from the line into variables
  - snprintf() to write the string data from sscanf() to the items array
- saveItems() uses fprintf to just write to file stream
- put items[] onto the stack with MAX_ITEMS for simplicity, can put it on the heap if we need resizing down the line
