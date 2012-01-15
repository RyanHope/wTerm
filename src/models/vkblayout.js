
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
		{symbols: [['`',SDLK._BACKQUOTE],['~',SDLK._BACKQUOTE]]},
		{symbols: [['1',SDLK._1],['!',SDLK._EXCLAIM]]},
		{symbols: [['2',SDLK._2],['@',SDLK._AT]]},
		{symbols: [['3',SDLK._3],['#',SDLK._HASH]]},
		{symbols: [['4',SDLK._4],['$',SDLK._DOLLAR]]},
		{symbols: [['5',SDLK._5],['%',SDLK._PERCENT]]},
		{symbols: [['6',SDLK._6],['^',SDLK._CARET]]},
		{symbols: [['7',SDLK._7],['&',SDLK._AMPERSAND]]},
		{symbols: [['8',SDLK._8],['*',SDLK._ASTERISK]]},
		{symbols: [['9',SDLK._9],['(',SDLK._LEFTPAREN]]},
		{symbols: [['0',SDLK._0],[')',SDLK._RIGHTPAREN]]},
		{symbols: [['-',SDLK._MINUS],['_',SDLK._UNDERSCORE]]},
		{symbols: [['=',SDLK._EQUALS],['+',SDLK._PLUS]]},
		{symbols: [['Bksp<br><img src="images/key_backspace.png" class="keyImg"/>',SDLK._BACKSPACE]], extraClasses: 'backspace'},
	],
	[
		{symbols: [['Tab<br><img src="images/key_tab.png" class="keyImg"/>',SDLK._TAB]], extraClasses: 'tab'},
		{symbols: [['Q',SDLK._q]]},
		{symbols: [['W',SDLK._w]]},
		{symbols: [['E',SDLK._e]]},
		{symbols: [['R',SDLK._r]]},
		{symbols: [['T',SDLK._t]]},
		{symbols: [['Y',SDLK._y]]},
		{symbols: [['U',SDLK._u]]},
		{symbols: [['I',SDLK._i]]},
		{symbols: [['O',SDLK._o]]},
		{symbols: [['P',SDLK._p]]},
		{symbols: [['[',SDLK._LEFTBRACKET],['{',SDLK._LEFTBRACKET]]},
		{symbols: [[']',SDLK._RIGHTBRACKET],['}',SDLK._RIGHTBRACKET]]},
		{symbols: [['\\',SDLK._BACKSLASH],['|',SDLK._BACKSLASH]], extraClasses: 'slash'},
	],
	[
		{symbols: [['Caps Lock',SDLK._CAPSLOCK]], modifier: 1, extraClasses: 'caps', toggling: true},
		{symbols: [['A',SDLK._a]]},
		{symbols: [['S',SDLK._s]]},
		{symbols: [['D',SDLK._d]]},
		{symbols: [['F',SDLK._f]]},
		{symbols: [['G',SDLK._g]]},
		{symbols: [['H',SDLK._h]]},
		{symbols: [['J',SDLK._j]]},
		{symbols: [['K',SDLK._k]]},
		{symbols: [['L',SDLK._l]]},
		{symbols: [[';',SDLK._SEMICOLON],[':',SDLK._COLON]]},
		{symbols: [["'",SDLK._QUOTE],['"',SDLK._QUOTEDBL]]},
		{symbols: [['Enter<br><img src="images/key_enter.png" class="keyImg"/>',SDLK._RETURN]], extraClasses: 'enter'},
	],
	[
		{symbols: [['Shift',SDLK._LSHIFT]], modifier: 1, extraClasses: 'shift-left'},
		{symbols: [['Z',SDLK._z]]},
		{symbols: [['X',SDLK._x]]},
		{symbols: [['C',SDLK._c]]},
		{symbols: [['V',SDLK._v]]},
		{symbols: [['B',SDLK._b]]},
		{symbols: [['N',SDLK._n]]},
		{symbols: [['M',SDLK._m]]},
		{symbols: [[',',SDLK._COMMA],['<',SDLK._LESS]]},
		{symbols: [['.',SDLK._PERIOD],['>',SDLK._GREATER]]},
		{symbols: [['/',SDLK._SLASH],['?',SDLK._QUESTION],['<span class="fnBind">Ins</span>',SDLK._INSERT]], extraClasses: 'arrow'},
		{symbols: [['<img src="images/cursorUp.png" class="keyImg"/>',SDLK._UP],['<span class="fnBind">PgUp</span>',SDLK._PAGEUP]], extraClasses: 'arrow'},
		{symbols: [['Shift',SDLK._RSHIFT],['<span class="fnBind">Del</span>',SDLK._DELETE]], modifier: 1, extraClasses: 'arrow'},
	],
	[
		{symbols: [['Ctrl',SDLK._LCTRL]], modifier: 1, extraClasses: 'mod ctrl'},
		{symbols: [['Fn',SDLK._MODE]], modifier: 1, extraClasses: 'mod fn'},
		{symbols: [['Alt',SDLK._LALT]], modifier: 1, extraClasses: 'mod alt'},
		{symbols: [['',' ']], extraClasses: 'spacebar'},
		{symbols: [['Meta',SDLK._RMETA]], modifier: 1, extraClasses: 'mod meta'},
		{symbols: [['Super',SDLK._RSUPER]], modifier: 1, extraClasses: 'mod super'},
		{symbols: [['<img src="images/cursorLeft.png" class="keyImg"/>',SDLK._LEFT],['<span class="fnBind">Home</span>',SDLK._HOME]], extraClasses: 'arrow'},
		{symbols: [['<img src="images/cursorDown.png" class="keyImg"/>',SDLK._DOWN],['<span class="fnBind">PgDn</span>',SDLK._PAGEDOWN]], extraClasses: 'arrow'},
		{symbols: [['<img src="images/cursorRight.png" class="keyImg"/>',SDLK._RIGHT],['<span class="fnBind">End</span>',SDLK._END]], extraClasses: 'arrow'},
	]
];
