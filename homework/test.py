import curses

def main(stdscr):
    # Clear screen
    stdscr.clear()
    curses.curs_set(0)  # Hide the cursor

    # Get screen height and width
    height, width = stdscr.getmaxyx()

    # Initial position of the character
    x = width // 2
    y = height // 2
    char = '@'

    while True:
        stdscr.clear()
        stdscr.addstr(0, 0, "Use arrow keys to move '@'. Press 'q' to quit.")
        stdscr.addstr(y, x, char)
        stdscr.refresh()

        key = stdscr.getch()

        if key == ord('q'):
            break
        elif key == curses.KEY_UP and y > 1:
            y -= 1
        elif key == curses.KEY_DOWN and y < height - 1:
            y += 1
        elif key == curses.KEY_LEFT and x > 0:
            x -= 1
        elif key == curses.KEY_RIGHT and x < width - 1:
            x += 1

# Run the curses application
curses.wrapper(main)