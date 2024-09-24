/**
 * Print the string on the tty.
 * \param s The pointer to the string
 */
extern void printk(const char* s);

/**
 * Print a character on the tty.
 * \param c The character
 */
extern void putchark(int c);

/**
 * Poll the keyboard to get a single character from input, if there is any.
 * \return -1 if keyboard is not ready, or a positive ascii code of the character.
 */
extern int getchark();