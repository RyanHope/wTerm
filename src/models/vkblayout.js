
var kbdLayouts = { };

function getKbdLayout(name, onload) {
	var layout = kbdLayouts[name] || kbdLayouts.default;
	if (layout.keys) {
		onload(layout.keys);
	} else {
		enyo.xhrGet({ url: layout.url, load: function(txt) {
			if (req.status == 200) {
				layout.keys = enyo.json.parse(txt);
				onload(layout.keys);
			}
		}});
	}
}

function kbdLayoutList() {
	var l = [], k;
	for (k in kbdLayouts) {
		if (kbdLayouts.hasOwnProperty(k)) {
			l.push({ caption: kbdLayouts[k].caption, value: k });
		}
	}
	return l;
}

kbdLayouts.default = { caption: 'Default (en-US)' };
kbdLayouts.default.keys = [
	[
		{symbols: [['Esc',SDLK._ESCAPE]], small: 1, extraClasses: 'escape'},
		{flex:1},
		{symbols: [['F1',SDLK._F1]], small: 1},
		{symbols: [['F2',SDLK._F2]], small: 1},
		{symbols: [['F3',SDLK._F3]], small: 1},
		{symbols: [['F4',SDLK._F4]], small: 1},
		{flex:1},
		{symbols: [['F5',SDLK._F5]], small: 1},
		{symbols: [['F6',SDLK._F6]], small: 1},
		{symbols: [['F7',SDLK._F7]], small: 1},
		{symbols: [['F8',SDLK._F8]], small: 1},
		{flex:1},
		{symbols: [['F9',SDLK._F9]], small: 1},
		{symbols: [['F10',SDLK._F10]], small: 1},
		{symbols: [['F11',SDLK._F11]], small: 1},
		{symbols: [['F12',SDLK._F12]], small: 1},
	],
	[{extraClasses: 'functionPadding'}],
	[
		{symbols: [['`',SDLK._BACKQUOTE],['~',SDLK._BACKQUOTE]], printable: true},
		{symbols: [['1',SDLK._1],['!',SDLK._EXCLAIM]], printable: true},
		{symbols: [['2',SDLK._2],['@',SDLK._AT]], printable: true},
		{symbols: [['3',SDLK._3],['#',SDLK._HASH]], printable: true},
		{symbols: [['4',SDLK._4],['$',SDLK._DOLLAR]], printable: true},
		{symbols: [['5',SDLK._5],['%',SDLK._PERCENT]], printable: true},
		{symbols: [['6',SDLK._6],['^',SDLK._CARET]], printable: true},
		{symbols: [['7',SDLK._7],['&',SDLK._AMPERSAND]], printable: true},
		{symbols: [['8',SDLK._8],['*',SDLK._ASTERISK]], printable: true},
		{symbols: [['9',SDLK._9],['(',SDLK._LEFTPAREN]], printable: true},
		{symbols: [['0',SDLK._0],[')',SDLK._RIGHTPAREN]], printable: true},
		{symbols: [['-',SDLK._MINUS],['_',SDLK._UNDERSCORE]], printable: true},
		{symbols: [['=',SDLK._EQUALS],['+',SDLK._PLUS]], printable: true},
		{symbols: [['Bksp<br><img src="images/key_backspace.png" class="keyImg"/>',SDLK._BACKSPACE]], extraClasses: 'backspace'},
	],
	[
		{symbols: [['Tab<br><img src="images/key_tab.png" class="keyImg"/>',SDLK._TAB]], extraClasses: 'tab'},
		{symbols: [['Q',SDLK._q]], printable: true},
		{symbols: [['W',SDLK._w]], printable: true},
		{symbols: [['E',SDLK._e]], printable: true},
		{symbols: [['R',SDLK._r]], printable: true},
		{symbols: [['T',SDLK._t]], printable: true},
		{symbols: [['Y',SDLK._y]], printable: true},
		{symbols: [['U',SDLK._u]], printable: true},
		{symbols: [['I',SDLK._i]], printable: true},
		{symbols: [['O',SDLK._o]], printable: true},
		{symbols: [['P',SDLK._p]], printable: true},
		{symbols: [['[',SDLK._LEFTBRACKET],['{',SDLK._LEFTBRACKET]], printable: true},
		{symbols: [[']',SDLK._RIGHTBRACKET],['}',SDLK._RIGHTBRACKET]], printable: true},
		{symbols: [['\\',SDLK._BACKSLASH],['|',SDLK._BACKSLASH]], printable: true, extraClasses: 'slash'},
	],
	[
		{symbols: [['Caps Lock',SDLK._CAPSLOCK]], modifier: 1, extraClasses: 'caps', toggling: true},
		{symbols: [['A',SDLK._a]], printable: true},
		{symbols: [['S',SDLK._s]], printable: true},
		{symbols: [['D',SDLK._d]], printable: true},
		{symbols: [['F',SDLK._f]], printable: true},
		{symbols: [['G',SDLK._g]], printable: true},
		{symbols: [['H',SDLK._h]], printable: true},
		{symbols: [['J',SDLK._j]], printable: true},
		{symbols: [['K',SDLK._k]], printable: true},
		{symbols: [['L',SDLK._l]], printable: true},
		{symbols: [[';',SDLK._SEMICOLON],[':',SDLK._COLON]], printable: true},
		{symbols: [["'",SDLK._QUOTE],['"',SDLK._QUOTEDBL]], printable: true},
		{symbols: [['Enter<br><img src="images/key_enter.png" class="keyImg"/>',SDLK._RETURN]], extraClasses: 'enter'},
	],
	[
		{symbols: [['Shift',SDLK._LSHIFT]], modifier: 1, extraClasses: 'shift-left'},
		{symbols: [['Z',SDLK._z]], printable: true},
		{symbols: [['X',SDLK._x]], printable: true},
		{symbols: [['C',SDLK._c]], printable: true},
		{symbols: [['V',SDLK._v]], printable: true},
		{symbols: [['B',SDLK._b]], printable: true},
		{symbols: [['N',SDLK._n]], printable: true},
		{symbols: [['M',SDLK._m]], printable: true},
		{symbols: [[',',SDLK._COMMA],['<',SDLK._LESS]], printable: true},
		{symbols: [['.',SDLK._PERIOD],['>',SDLK._GREATER],['<span class="fnBind">Del</span>',SDLK._DELETE]], printable: true},
		{symbols: [['/',SDLK._SLASH],['?',SDLK._QUESTION],['<span class="fnBind">Ins</span>',SDLK._INSERT]], printable: true},
		{symbols: [['Shift',SDLK._RSHIFT]], modifier: 1, extraClasses: 'shift-right'},
		{symbols: [['<img src="images/cursorUp.png" class="keyImg"/>',SDLK._UP],['<span class="fnBind">PgUp</span>',SDLK._PAGEUP]], extraClasses: 'arrow'},
	],
	[
		{symbols: [['Ctrl',SDLK._LCTRL]], modifier: 1, extraClasses: 'mod ctrl'},
		{symbols: [['Fn',SDLK._MODE]], modifier: 1, extraClasses: 'mod fn'},
		{symbols: [['Alt',SDLK._LALT]], modifier: 1, extraClasses: 'mod alt'},
		{symbols: [[' ',SDLK._SPACE]], printable: true, extraClasses: 'spacebar'},
		{symbols: [['Alt',SDLK._RALT]], modifier: 1, extraClasses: 'mod alt'},
		{symbols: [['Ctrl',SDLK._RCTRL]], modifier: 1, extraClasses: 'mod ctrl'},
		{symbols: [['<img src="images/cursorLeft.png" class="keyImg"/>',SDLK._LEFT],['<span class="fnBind">Home</span>',SDLK._HOME]], extraClasses: 'arrow'},
		{symbols: [['<img src="images/cursorRight.png" class="keyImg"/>',SDLK._RIGHT],['<span class="fnBind">End</span>',SDLK._END]], extraClasses: 'arrow'},
		{symbols: [['<img src="images/cursorDown.png" class="keyImg"/>',SDLK._DOWN],['<span class="fnBind">PgDn</span>',SDLK._PAGEDOWN]], extraClasses: 'arrow'},
	]
];

kbdLayouts.dvorak = { caption: 'Dvorak Simplified (en-US)' };
kbdLayouts.dvorak.keys = [
	[
		{symbols: [['Esc',SDLK._ESCAPE]], small: 1, extraClasses: 'escape'},
		{flex:1},
		{symbols: [['F1',SDLK._F1]], small: 1},
		{symbols: [['F2',SDLK._F2]], small: 1},
		{symbols: [['F3',SDLK._F3]], small: 1},
		{symbols: [['F4',SDLK._F4]], small: 1},
		{flex:1},
		{symbols: [['F5',SDLK._F5]], small: 1},
		{symbols: [['F6',SDLK._F6]], small: 1},
		{symbols: [['F7',SDLK._F7]], small: 1},
		{symbols: [['F8',SDLK._F8]], small: 1},
		{flex:1},
		{symbols: [['F9',SDLK._F9]], small: 1},
		{symbols: [['F10',SDLK._F10]], small: 1},
		{symbols: [['F11',SDLK._F11]], small: 1},
		{symbols: [['F12',SDLK._F12]], small: 1},
	],
	[{extraClasses: 'functionPadding'}],
	[
		{symbols: [['`',SDLK._BACKQUOTE],['~',SDLK._BACKQUOTE]], printable: true},
		{symbols: [['1',SDLK._1],['!',SDLK._EXCLAIM]], printable: true},
		{symbols: [['2',SDLK._2],['@',SDLK._AT]], printable: true},
		{symbols: [['3',SDLK._3],['#',SDLK._HASH]], printable: true},
		{symbols: [['4',SDLK._4],['$',SDLK._DOLLAR]], printable: true},
		{symbols: [['5',SDLK._5],['%',SDLK._PERCENT]], printable: true},
		{symbols: [['6',SDLK._6],['^',SDLK._CARET]], printable: true},
		{symbols: [['7',SDLK._7],['&',SDLK._AMPERSAND]], printable: true},
		{symbols: [['8',SDLK._8],['*',SDLK._ASTERISK]], printable: true},
		{symbols: [['9',SDLK._9],['(',SDLK._LEFTPAREN]], printable: true},
		{symbols: [['0',SDLK._0],[')',SDLK._RIGHTPAREN]], printable: true},
		{symbols: [['[',SDLK._LEFTBRACKET],['{',SDLK._LEFTBRACKET]]], printable: true},
		{symbols: [[']',SDLK._RIGHTBRACKET],['}',SDLK._RIGHTBRACKET]], printable: true},
		{symbols: [['Bksp<br><img src="images/key_backspace.png" class="keyImg"/>',SDLK._BACKSPACE]], extraClasses: 'backspace'},
	],
	[
		{symbols: [['Tab<br><img src="images/key_tab.png" class="keyImg"/>',SDLK._TAB]], extraClasses: 'tab'},
		{symbols: [["'",SDLK._QUOTE],['"',SDLK._QUOTEDBL]], printable: true},
		{symbols: [[',',SDLK._COMMA],['<',SDLK._LESS]], printable: true},
		{symbols: [['.',SDLK._PERIOD],['>',SDLK._GREATER]], printable: true},
		{symbols: [['P',SDLK._p]], printable: true},
		{symbols: [['Y',SDLK._y]], printable: true},
		{symbols: [['F',SDLK._f]], printable: true},
		{symbols: [['G',SDLK._g]], printable: true},
		{symbols: [['C',SDLK._c]], printable: true},
		{symbols: [['R',SDLK._r]], printable: true},
		{symbols: [['L',SDLK._l]], printable: true},
		{symbols: [['/',SDLK._SLASH],['?',SDLK._QUESTION]], printable: true},
		{symbols: [['=',SDLK._EQUALS],['+',SDLK._PLUS]], printable: true},
		{symbols: [['\\',SDLK._BACKSLASH],['|',SDLK._BACKSLASH]], printable: true, extraClasses: 'slash'},
	],
	[
		{symbols: [['Caps Lock',SDLK._CAPSLOCK]], modifier: 1, extraClasses: 'caps', toggling: true},
		{symbols: [['A',SDLK._a]], printable: true},
		{symbols: [['O',SDLK._o]], printable: true},
		{symbols: [['E',SDLK._e]], printable: true},
		{symbols: [['U',SDLK._u]], printable: true},
		{symbols: [['I',SDLK._i]], printable: true},
		{symbols: [['D',SDLK._d]], printable: true},
		{symbols: [['H',SDLK._h]], printable: true},
		{symbols: [['T',SDLK._t]], printable: true},
		{symbols: [['N',SDLK._n]], printable: true},
		{symbols: [['S',SDLK._s]], printable: true},
		{symbols: [['-',SDLK._MINUS],['_',SDLK._UNDERSCORE]], printable: true},
		{symbols: [['Enter<br><img src="images/key_enter.png" class="keyImg"/>',SDLK._RETURN]], extraClasses: 'enter'},
	],
	[
		{symbols: [['Shift',SDLK._LSHIFT]], modifier: 1, extraClasses: 'shift-left'},
		{symbols: [[';',SDLK._SEMICOLON],[':',SDLK._COLON]], printable: true},
		{symbols: [['Q',SDLK._q]], printable: true},
		{symbols: [['J',SDLK._j]], printable: true},
		{symbols: [['K',SDLK._k]], printable: true},
		{symbols: [['X',SDLK._x]], printable: true},
		{symbols: [['B',SDLK._b]], printable: true},
		{symbols: [['M',SDLK._m]], printable: true},
		{symbols: [['W',SDLK._w]], printable: true},
		{symbols: [['V',SDLK._v],['<span class="fnBind">Del</span>',SDLK._DELETE]], printable: true},
		{symbols: [['Z',SDLK._z],['<span class="fnBind">Ins</span>',SDLK._INSERT]], printable: true},
		{symbols: [['Shift',SDLK._RSHIFT]], modifier: 1, extraClasses: 'shift-right'},
		{symbols: [['<img src="images/cursorUp.png" class="keyImg"/>',SDLK._UP],['<span class="fnBind">PgUp</span>',SDLK._PAGEUP]], extraClasses: 'arrow'},
	],
	[
		{symbols: [['Ctrl',SDLK._LCTRL]], modifier: 1, extraClasses: 'mod ctrl'},
		{symbols: [['Fn',SDLK._MODE]], modifier: 1, extraClasses: 'mod fn'},
		{symbols: [['Alt',SDLK._LALT]], modifier: 1, extraClasses: 'mod alt'},
		{symbols: [[' ',SDLK._SPACE]], printable: true, extraClasses: 'spacebar'},
		{symbols: [['Alt',SDLK._RALT]], modifier: 1, extraClasses: 'mod alt'},
		{symbols: [['Ctrl',SDLK._RCTRL]], modifier: 1, extraClasses: 'mod ctrl'},
		{symbols: [['<img src="images/cursorLeft.png" class="keyImg"/>',SDLK._LEFT],['<span class="fnBind">Home</span>',SDLK._HOME]], extraClasses: 'arrow'},
		{symbols: [['<img src="images/cursorRight.png" class="keyImg"/>',SDLK._RIGHT],['<span class="fnBind">End</span>',SDLK._END]], extraClasses: 'arrow'},
		{symbols: [['<img src="images/cursorDown.png" class="keyImg"/>',SDLK._DOWN],['<span class="fnBind">PgDn</span>',SDLK._PAGEDOWN]], extraClasses: 'arrow'},
	]
];
