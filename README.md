# cartita-freecell
A freecell card game using Microsoft's game numbering system. Written in C++ and Gtkmm.
The UI is keyboard driven (no mouse support game) with the commands similar as those implemented in the legendary FCPro Windows game: 1 to 8 for columns, A to D for the four freecells, H for home.
The png images for the 52 cards I got from http://acbl.mybigcommerce.com/52-playing-cards/. The place is called American Contract Bridge League. The download is free and I'm very thankful.
To colorize console output (in the debugging stage) I also used https://termcolor.readthedocs.io/#. Thanks, too.
The algorithm to generate each game is taken from https://www.linusakesson.net/software/freecell.php, with very minor modifications. Thanks!
