### Overview

usok is a minified graphical implementation of the Sokoban game where the player must push boxes onto goal squares without getting the boxes or themselves stuck. The whole game - graphics included - is contained in usok.c, which is well under 2kb of C source (comments included).

### Screenshots
![Screenshot of usok Game](https://raw.githubusercontent.com/DanielWhite94/usok/master/screenshots/usok1.png)

![Screenshot of usok Game](https://raw.githubusercontent.com/DanielWhite94/usok/master/screenshots/usok2.png)

![Screenshot of usok Game](https://raw.githubusercontent.com/DanielWhite94/usok/master/screenshots/usok3.png)

![Screenshot of usok Game](https://raw.githubusercontent.com/DanielWhite94/usok/master/screenshots/usok4.png)

### Compiling and Using

To build use the included `Makefile`. To run pass a standard level file via stdin - e.g. `cat level | ./usok` - and then use the arrow keys to move.
