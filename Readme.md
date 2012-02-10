wTerm (webOS Terminal Emulator)
===============================

wTerm is a Enyo PDK/Hybrid Terminal Emulator for the webOS platform.

[![paypal donation][paypal-logo]][paypal]
[paypal-logo]: https://www.paypalobjects.com/en_US/i/btn/btn_donate_LG.gif
[paypal]: https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=VU4L7VTGSR5C2


Features
--------

* Hardware accelerated with OpenGL
* Custom virtual keyboard
* Customizable virtual keyboard layout (coming soon)
* Customizable font sizes
* Customizable color schemes
* Customizable key bindings

Main Contributors
-----------------

* Stefan Bühler
* Will Dietz
* Ryan Hope (Project Lead)
* Brian Kearney

VKB Layout Contributors
-----------------------

* Chris Jowett (Dvorak)
* Yohan Dubanchet (French)

Safe Root Access
----------------

By default, wTerm logs into a non-root user (wterm). For safe root access follow the procedure below:

1. Open the wTerm preferences dialog from the app menu.
1. Click on the 'Misc' tab.
1. Change the 'Default Exec Command' to 'login -f root'.
1. Restart wTerm.
1. Run 'passwd' to set a root password.
1. Run 'vi /etc/group', add 'wterm' to the root group.
1. Change the 'Default Exec Command' back to 'login -f wterm'.
1. Restart wTerm.


Screenshots
-----------

<a href="https://github.com/RyanHope/wTerm/raw/master/screenshots/htop.png"><img src="https://github.com/RyanHope/wTerm/raw/master/screenshots/htop.png" width=25%></a>
<a href="https://github.com/RyanHope/wTerm/raw/master/screenshots/irssi.png"><img src="https://github.com/RyanHope/wTerm/raw/master/screenshots/irssi.png" width=25%></a>
<a href="https://github.com/RyanHope/wTerm/raw/master/screenshots/midnight_commander.png"><img src="https://github.com/RyanHope/wTerm/raw/master/screenshots/midnight_commander.png" width=25%></a><br>
<a href="https://github.com/RyanHope/wTerm/raw/master/screenshots/vim.png"><img src="https://github.com/RyanHope/wTerm/raw/master/screenshots/vim.png" width=25%></a>
<a href="https://github.com/RyanHope/wTerm/raw/master/screenshots/emacs.png"><img src="https://github.com/RyanHope/wTerm/raw/master/screenshots/emacs.png" width=25%></a>
<a href="https://github.com/RyanHope/wTerm/raw/master/screenshots/exhibition.png"><img src="https://github.com/RyanHope/wTerm/raw/master/screenshots/exhibition.png" width=25%></a><br>
<a href="https://github.com/RyanHope/wTerm/raw/master/screenshots/stack_of_wterms.png"><img src="https://github.com/RyanHope/wTerm/raw/master/screenshots/stack_of_wterms.png" width=25%></a>
<a href="https://github.com/RyanHope/wTerm/raw/master/screenshots/just_type.png"><img src="https://github.com/RyanHope/wTerm/raw/master/screenshots/just_type.png" width=25%></a>
