
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
		{symbols: [['Esc','\x1B']], small: 1, extraClasses: 'escape'},
		{flex:1},
		{symbols: [['F1',282]], small: 1},
		{symbols: [['F2',283]], small: 1},
		{symbols: [['F3',294]], small: 1},
		{symbols: [['F4',285]], small: 1},
		{flex:1},
		{symbols: [['F5',286]], small: 1},
		{symbols: [['F6',287]], small: 1},
		{symbols: [['F7',288]], small: 1},
		{symbols: [['F8',289]], small: 1},
		{flex:1},
		{symbols: [['F9',290]], small: 1},
		{symbols: [['F10',291]], small: 1},
		{symbols: [['F11',292]], small: 1},
		{symbols: [['F12',293]], small: 1},
	],
	[{extraClasses: 'functionPadding'}],
	[
		{symbols: [['`','`'],['~','~']]},
		{symbols: [['1','1'],['!','!']]},
		{symbols: [['2','2'],['@','@']]},
		{symbols: [['3','3'],['#','#']]},
		{symbols: [['4','4'],['$','$']]},
		{symbols: [['5','5'],['%','%']]},
		{symbols: [['6','6'],['^','^']]},
		{symbols: [['7','7'],['&','&']]},
		{symbols: [['8','8'],['*','*']]},
		{symbols: [['9','9'],['(','(']]},
		{symbols: [['0','0'],[')',')']]},
		{symbols: [['-','-'],['_','_']]},
		{symbols: [['=','='],['+','+']]},
		{symbols: [['Bksp<br><img src="images/key_backspace.png" class="keyImg"/>','\x08']], extraClasses: 'backspace'},
	],
	[
		{symbols: [['Tab<br><img src="images/key_tab.png" class="keyImg"/>','\x09']], extraClasses: 'tab'},
		{symbols: [['Q','q']]},
		{symbols: [['W','w']]},
		{symbols: [['E','e']]},
		{symbols: [['R','r']]},
		{symbols: [['T','t']]},
		{symbols: [['Y','y']]},
		{symbols: [['U','u']]},
		{symbols: [['I','i']]},
		{symbols: [['O','o']]},
		{symbols: [['P','p']]},
		{symbols: [['[','['],['{','{']]},
		{symbols: [[']',']'],['}','}']]},
		{symbols: [['\\','\\'],['|','|']], extraClasses: 'slash'},
	],
	[
		{symbols: [['Caps Lock',301]], modifier: 1, extraClasses: 'caps', toggling: true},
		{symbols: [['A','a']]},
		{symbols: [['S','s']]},
		{symbols: [['D','d']]},
		{symbols: [['F','f']]},
		{symbols: [['G','g']]},
		{symbols: [['H','h']]},
		{symbols: [['J','j']]},
		{symbols: [['K','k']]},
		{symbols: [['L','l']]},
		{symbols: [[';',';'],[':',':']]},
		{symbols: [["'","'"],['"','"']]},
		{symbols: [['Enter<br><img src="images/key_enter.png" class="keyImg"/>','\x0D']], extraClasses: 'enter'},
	],
	[
		{symbols: [['Shift',304]], modifier: 1, extraClasses: 'shift-left'},
		{symbols: [['Z','z']]},
		{symbols: [['X','x']]},
		{symbols: [['C','c']]},
		{symbols: [['V','v']]},
		{symbols: [['B','b']]},
		{symbols: [['N','n']]},
		{symbols: [['M','m']]},
		{symbols: [[',',','],['<','<']]},
		{symbols: [['.','.'],['>','>']]},
		{symbols: [[';',';'],[':',':']]},
		{symbols: [['/','/'],['?','?'],['<span class="fnBind">Ins</span>',null]], extraClasses: 'arrow'},
		{symbols: [['<img src="images/cursorUp.png" class="keyImg"/>',273],['<span class="fnBind">PgUp</span>',null]], extraClasses: 'arrow'},
		{symbols: [['Shift',304],['<span class="fnBind">Del</span>',null]], modifier: 1, extraClasses: 'arrow'},
	],
	[
		{symbols: [['Ctrl',306]], modifier: 1, extraClasses: 'mod ctrl'},
		{symbols: [['Fn',313]], modifier: 1, extraClasses: 'mod fn'},
		{symbols: [['Alt',308]], modifier: 1, extraClasses: 'mod alt'},
		{symbols: [['',' ']], extraClasses: 'spacebar'},
		{symbols: [['Meta',310]], modifier: 1, extraClasses: 'mod meta'},
		{symbols: [['Super',314]], modifier: 1, extraClasses: 'mod super'},
		{symbols: [['<img src="images/cursorLeft.png" class="keyImg"/>',276],['<span class="fnBind">Home</span>',null]], extraClasses: 'arrow'},
		{symbols: [['<img src="images/cursorDown.png" class="keyImg"/>',274],['<span class="fnBind">PgDn</span>',null]], extraClasses: 'arrow'},
		{symbols: [['<img src="images/cursorRight.png" class="keyImg"/>',275],['<span class="fnBind">End</span>',null]], extraClasses: 'arrow'},
	]
];
