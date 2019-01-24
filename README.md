# cartita-freecell
A freecell card game using Microsoft's game numbering system. Written in C++ and Gtkmm.

The UI is keyboard driven (no mouse support yet) with the commands similar to those implemented in the legendary FCPro Windows game (http://www.solitairelaboratory.com/fcpro.html): 1 to 8 for columns, A to D for the four freecells, H for home. You move the last card in the third column from the left to, say, freecell C, by typing "3C". Or you move it to column 8 by typing "38". It's easy and intuitive. Backspace undoes, Tab redoes, Esc deselects previously selected card and you can scale the cards using + and -.

To select a game, just right click anywhere. You'll be presented with a very minimalistic menu!

All moves are animated.

The png images for the 52 cards I got from http://acbl.mybigcommerce.com/52-playing-cards/. The place is called American Contract Bridge League. The download is free and I'm very thankful.

To colorize console output (in the debugging stage) I also used https://termcolor.readthedocs.io/#. Thanks, too.

The algorithm to generate each game is taken from https://www.linusakesson.net/software/freecell.php, with very minor modifications. Thanks!

There is a makefile included, the program doesn't require any extra libraries other than those installed with Gtkmm. I compiled and tested it on Ubuntu 18.04 Mate, it should work everywhere else.

The license terms are: use it however you want, it's completely free.

If you have questions, don't doubt to ask: jorgitus.none@gmail.com
