
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

kbdLayouts.default = { caption: 'default (en-US)' };
kbdLayouts.default.keys = [
	[
		{small: 1, content: 'Esc', sym: 27, unicode: '\x1B'},
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
	[
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
		{content: 'Bksp', flex: 1, sym: 8, unicode: '\x7f'},
	],
	[
		{content: 'Tab', flex: 1, sym: 9, unicode: '\x09'},
		{content: 'q', sym: 113},
		{content: 'w', sym: 119},
		{content: 'e', sym: 101},
		{content: 'r', sym: 114},
		{content: 't', sym: 116},
		{content: 'y', sym: 121},
		{content: 'u', sym: 117},
		{content: 'i', sym: 105},
		{content: 'o', sym: 111},
		{content: 'p', sym: 112},
		{content: '{<br>[', sym: 91, unicode: '['},
		{content: '}<br>]', sym: 93, unicode: ']'},
		{content: '|<br>\\', sym: 92, unicode: '\\'},
	],
	[
		{content: 'Caps Lock', flex: 1, toggling: true, ontouchstart: 'toggleCaps'},
		{content: 'a', sym: 97},
		{content: 's', sym: 115},
		{content: 'd', sym: 100},
		{content: 'f', sym: 102},
		{content: 'g', sym: 103},
		{content: 'h', sym: 104},
		{content: 'j', sym: 106},
		{content: 'k', sym: 107},
		{content: 'l', sym: 108},
		{content: ':<br>;', sym: 59, unicode: ';'},
		{content: '\"<br>\'', sym: 39, unicode: '\''},
		{content: 'Enter', flex: 1, sym: 13, unicode: '\x0D'},
	],
	[
		{content: 'Shift', extraClasses: 'shift-left', sym: 304, unicode: null},
		{content: 'z', sym: 122},
		{content: 'x', sym: 120},
		{content: 'c', sym: 99},
		{content: 'v', sym: 118},
		{content: 'b', sym: 98},
		{content: 'n', sym: 110},
		{content: 'm', sym: 109},
		{content: '&lt;<br>,', sym: 44, unicode: ','},
		{content: 'Up', sym: 273},
		{content: '&gt;<br>.', sym: 46, unicode: '.'},
		{content: 'Shift', extraClasses: 'shift-right', sym: 303, unicode: null},
	],
	[
		{content: 'Ctrl', extraClasses: 'ctrl-left', sym: 306, unicode: null},
		{content: 'Fn', sym: 313, unicode: null},
		{content: 'Alt', sym: 308, unicode: null},
		{content: 'Space', extraClasses: 'spacebar', sym: 32, unicode: ' '},
		{content: 'Left', sym: 276, unicode: null},
		{content: 'Down', sym: 274, unicode: null},
		{content: 'Right', sym: 275, unicode: null},
		{content: '~<br>`', sym: 96, unicode: '`'},
		{content: '?<br>/', sym: 47, unicode: '/'},
	]
];
