
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
		{small: 1, extraClasses: 'escape', content: 'Esc', sym: 27, unicode: '\x1B'},
		{flex:1},
		{small: 1, content: 'F1', sym: 282, unicode: null},
		{small: 1, content: 'F2', sym: 283, unicode: null},
		{small: 1, content: 'F3', sym: 294, unicode: null},
		{small: 1, content: 'F4', sym: 285, unicode: null},
		{flex:1},
		{small: 1, content: 'F5', sym: 286, unicode: null},
		{small: 1, content: 'F6', sym: 287, unicode: null},
		{small: 1, content: 'F7', sym: 288, unicode: null},
		{small: 1, content: 'F8', sym: 289, unicode: null},
		{flex:1},
		{small: 1, content: 'F9', sym: 290, unicode: null},
		{small: 1, content: 'F10', sym: 291, unicode: null},
		{small: 1, content: 'F11', sym: 292, unicode: null},
		{small: 1, content: 'F12', sym: 293, unicode: null}
	],
	[{extraClasses: 'functionPadding'}],
	[
		{content: '~<br>`', sym: 96, unicode: '`'},
		{content: '!<br>1', sym: 48, unicode: '1'},
		{content: '@<br>2', sym: 49, unicode: '2'},
		{content: '#<br>3', sym: 50, unicode: '3'},
		{content: '$<br>4', sym: 51, unicode: '4'},
		{content: '%<br>5', sym: 52, unicode: '5'},
		{content: '^<br>6', sym: 53, unicode: '6'},
		{content: '&<br>7', sym: 54, unicode: '7'},
		{content: '*<br>8', sym: 55, unicode: '8'},
		{content: '(<br>9', sym: 56, unicode: '9'},
		{content: ')<br>0', sym: 48, unicode: '0'},
		{content: '_<br>-', sym: 45, unicode: '-'},
		{content: '+<br>=', sym: 61, unicode: '='},
		{content: 'Bksp<br><img src="images/key_backspace.png" class="keyImg"/>', extraClasses: 'backspace', sym: 8, unicode: '\x7f'},
	],
	[
		{content: 'Tab<br><img src="images/key_tab.png" class="keyImg"/>', extraClasses: 'tab', sym: 9, unicode: '\x09'},
		{content: 'Q', sym: 113},
		{content: 'W', sym: 119},
		{content: 'E', sym: 101},
		{content: 'R', sym: 114},
		{content: 'T', sym: 116},
		{content: 'Y', sym: 121},
		{content: 'U', sym: 117},
		{content: 'I', sym: 105},
		{content: 'O', sym: 111},
		{content: 'P', sym: 112},
		{content: '{<br>[', sym: 91, unicode: '['},
		{content: '}<br>]', sym: 93, unicode: ']'},
		{content: '|<br>\\', sym: 92, unicode: '\\'},
	],
	[
		{content: 'Caps Lock', extraClasses: 'caps', toggling: true, ontouchstart: 'toggleCaps'},
		{content: 'A', sym: 97},
		{content: 'S', sym: 115},
		{content: 'D', sym: 100},
		{content: 'F', sym: 102},
		{content: 'G', sym: 103},
		{content: 'H', sym: 104},
		{content: 'J', sym: 106},
		{content: 'K', sym: 107},
		{content: 'L', sym: 108},
		{content: ':<br>;', sym: 59, unicode: ';'},
		{content: '\"<br>\'', sym: 39, unicode: '\''},
		{content: 'Enter<br><img src="images/key_enter.png" class="keyImg"/>', extraClasses: 'enter', sym: 13, unicode: '\x0D'},
	],
	[
		{content: 'Shift', extraClasses: 'shift-left', sym: 304, unicode: null},
		{content: 'Z', sym: 122},
		{content: 'X', sym: 120},
		{content: 'C', sym: 99},
		{content: 'V', sym: 118},
		{content: 'B', sym: 98},
		{content: 'N', sym: 110},
		{content: 'M', sym: 109},
		{content: '&lt;<br>,', sym: 44, unicode: ','},
		{content: '&gt;<br>.', sym: 46, unicode: '.'},
		{content: '?&nbsp;&nbsp;&nbsp;<span class="fnBind">Ins</span><br>/', extraClasses: 'arrow', sym: 47, unicode: '/'},
		{content: '<span class="fnBind">PgUp</span><br><img src="images/cursorUp.png" class="keyImg"/>', extraClasses: 'arrow', sym: 273},
		{content: 'Del', extraClasses: 'arrow'},
	],
	[
		{content: 'Ctrl', extraClasses: 'ctrl-left', sym: 306, unicode: null},
		{content: 'Fn', extraClasses: 'fn', sym: 313, unicode: null},
		{content: 'Alt', extraClasses: 'alt', sym: 308, unicode: null},
		{content: 'Space', extraClasses: 'spacebar', sym: 32, unicode: ' '},
		{content: 'Meta', extraClasses: 'meta', sym: 313, unicode: null},
		{content: '<span class="fnBind">Home</span><br><img src="images/cursorLeft.png" class="keyImg"/>', extraClasses: 'arrow', sym: 276, unicode: null},
		{content: '<span class="fnBind">PgDn</span><br><img src="images/cursorDown.png" class="keyImg"/>', extraClasses: 'arrow', sym: 274, unicode: null},
		{content: '<span class="fnBind">End</span><br><img src="images/cursorRight.png" class="keyImg"/>', extraClasses: 'arrow', sym: 275, unicode: null},
	]
];
