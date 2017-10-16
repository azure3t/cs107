/***************************************************************************
 * Cal's Most Excellent and Insidious Bomb, Version N.0
 * Copyright 2016, Weenies Incorporated. All rights reserved.
 ***************************************************************************/
#include "util.h"
#include <error.h>
#include <stdio.h>

/*
 * Note to self: Remember to erase this file so my victims will have no
 * idea what is going on, and so they will all blow up in a
 * spectaculary fiendish explosion. -- Oski
 */

int main(int argc, char *argv[])
{
    /* When run with no arguments, bomb reads input from stdin */
    if (argc == 1)
        infile = stdin;

    /* When run with one argument <file>, the bomb reads from <file>
     * until EOF, and then switches to standard input. Thus, as you
     * defuse each level, you can add its defusing string to <file> and
     * avoid having to retype it. */
    else if (!(infile = fopen(argv[1], "r")))
        error(1, 0, "'%s': No such file", argv[1]);

    if (argc > 2)
        error(0, 0, "ignoring excess arguments...");

    /* Do all sorts of secret stuff that makes the bomb harder to defuse */
    initialize_bomb();

    printf("\nWARNING: This program is armed and dangerous. Explosion penalties apply.\nProceed at your own risk.\n");

    printf("\nThe 5 fiendish levels of fun begin now. Good luck!\n");
    char *input;

    /* Hit me with your best shot! */
    input = read_line();        // read user's input
    level_1(input);             // run the level
    level_defused(1, input);    // drat! they figured that one out
    printf("Level 1 defused. How about the next one?\n");

    /* The second level is harder.... */
    input = read_line();
    level_2(input);
    level_defused(2, input);
    printf("Level 2 disarmed. Keep going!\n");

    /* I guess this is too easy so far. Let's take it up a notch! */
    input = read_line();
    level_3(input);
    level_defused(3, input);
    printf("Level 3 disconnected. Onto the next one!\n");

    /* Oh yeah? No one will ever figure out how to defuse this... */
    input = read_line();
    level_4(input);
    level_defused(4, input);
    printf("Level 4 disemboweled. And lastly...\n");

    /* This level will never be used, since no one will get past the
     * earlier ones. But just in case, make this one extra hard. */
    input = read_line();
    level_5(input);
    level_defused(5, input);
    printf("Level 5 deactivated. This bomb is all done for. Great job!\n");

    /* Inconceivable! I had no idea those Stanford students could be so clever.
     * Is it really possible they solved it all? Let me see if I have anything
     * else I could throw at them... Eh, well, maybe next time. */
    finish_bomb();
    return 0;
}
