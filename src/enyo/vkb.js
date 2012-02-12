enyo.kind({

	name: 'vkb',
	kind: enyo.Stateful,

	width: '100%',

	className: 'vkb',

	published: {
		caption: '',
		landscape: false,
		layout: null,
		phone: false
	},
	
	components: [
		{
			name : "sysSound",
			kind : "PalmService",
			service : "palm://com.palm.audio/systemsounds",
			method : "playFeedback"
		},
		{kind: 'CharSelector', name: 'charSelector'},
		{name: 'keys'}
	],

	setupLayout: function() {
		var components = [], i, j, comps, c, e;
		for (i = 0; i < this.layout.length; i++) {
			row = this.layout[i];
			comps = [];
			for (j = 0; j < row.length; j++) {
				e = row[j];
				if (e.content || e.symbols) {
					if (e.hasOwnProperty('toggling') && c.toggling)
						c = enyo.mixin({kind: 'vkbKey', className: '', isPhone: this.phone, ondown: 'vkbKeyToggle'}, e);
					else
						c = enyo.mixin({kind: 'vkbKey', className: '', isPhone: this.phone, ondown: 'vkbKeyDown', onup: 'vkbKeyUp'}, e);
				} else {
					c = enyo.mixin({className: ''}, e); /* simple flex or custom */
				}
				if (c.small) c.className += ' small';
				else if (c.micro) c.className += ' micro';
				if (c.extraClasses) c.className += ' ' + c.extraClasses;
				if (!c.hasOwnProperty('printable')) c.printable = false;
				if (!c.hasOwnProperty('unicode') && c.content) c.unicode = c.content.toLowerCase();
				comps.push(c);
			}
			components.push({layoutKind: 'HFlexLayout', pack: 'end', components: comps});
		}
		this.$.keys.destroy();
		this.createComponents([{kind: 'VFlexBox', name: 'keys', components: components}]);
	},

	initComponents: function() {
		this.inherited(arguments)
		this.landscapeChanged()
		this.setupLayout()
	},

	setState: function(inState, inValue) {
		this.addRemoveClass(inState, Boolean(inValue));
	},

	landscapeChanged: function() {
		this.stateChanged("landscape")
	},

	setTerminal: function(term) {
		this.terminal = term
		this.$.charSelector.terminal = term
	},

	vkbKeyToggle: function(inSender) {
		if (!inSender.down)
			this.vkbKeyUp(inSender)
		else
			this.vkbKeyDown(inSender)
	},

	isShift: function() {
		return ((this.modstate & SDLK.KMOD_LSHIFT) || (this.modstate & SDLK.KMOD_RSHIFT) || (this.modstate & SDLK.KMOD_CAPS))
	},
	isMode: function() {
		return (this.modstate & SDLK.KMOD_MODE)
	},

	processKey: function(inSender) {
		var symbols = inSender.symbols
		var symbol = null
		var sym = null
		var unicode = ''
		if (this.isShift()) {
			if (this.isMode()) {
				symbol = symbols[3] ? symbols[3] : symbols[2];
			}
			if (!symbol) symbol = symbols[1] ? symbols[1] : symbols[0];
		} else if (this.isMode()) {
			symbol = symbols[2] ? symbols[2] : symbols[0];
		} else {
			symbol = symbols[0];
		}
		sym = symbol[1]
		if (!sym) {
			sym = 0
			unicode = symbol[0]
			if (!this.isShift()) unicode = unicode.toLocaleLowerCase();
		}
		return [sym, unicode]
	},

	keyUp: function(sym, unicode) {
		var m = vkb.modKeys[sym];
		if (m) {
			if (m instanceof Array) {
				// toggle
				m = m[0];
				this.modstate = this.modstate ^ m;
			} else {
				this.modstate = this.modstate & ~m;
			}
		}
		this.terminal.keyUp(this.modstate, sym, unicode, 0);
	},

	vkbKeyUp: function(inSender) {
		var k = this.processKey(inSender);
		if (k[0] == SDLK._MENU) {
			this.$.charSelector.openAtCenter()
		} else {
			this.keyUp(k[0], k[1]);
		}
	},

	keyDown: function(sym, unicode, sound) {
		var m = vkb.modKeys[sym];
		if (m) {
			if (m instanceof Array) {
				// toggle
				m = m[0];
				this.modstate = this.modstate ^ m;
			} else {
				this.modstate = this.modstate | m;
			}
		}
		this.terminal.keyDown(this.modstate, sym, unicode, sound);
	},

	vkbKeyDown: function(inSender) {
		var k = this.processKey(inSender);
		if (k[0] != SDLK._MENU)
			this.keyDown(k[0], k[1], 1);
	}

})

enyo.application.vkbLayouts = []
enyo.depends('src/enyo/vkbLayouts/')

// array of one modifier: "LOCK"ing
vkb.modKeys = {};
vkb.modKeys[SDLK._NUMLOCK] = [SDLK.KMOD_NUM];
vkb.modKeys[SDLK._CAPSLOCK] = [SDLK.KMOD_CAPS];
// vkb.modKeys[SDLK._SCROLLOCK] = [SDLK.KMOD_];
vkb.modKeys[SDLK._RSHIFT] = SDLK.KMOD_RSHIFT;
vkb.modKeys[SDLK._LSHIFT] = SDLK.KMOD_LSHIFT;
vkb.modKeys[SDLK._RCTRL] = SDLK.KMOD_RCTRL;
vkb.modKeys[17] = SDLK.KMOD_LCTRL; // HP_"SYM"
vkb.modKeys[SDLK._LCTRL] = SDLK.KMOD_LCTRL;
vkb.modKeys[SDLK._RALT] = SDLK.KMOD_RALT;
vkb.modKeys[SDLK._WORLD_30] = SDLK.KMOD_LALT; // HP_???
vkb.modKeys[SDLK._LALT] = SDLK.KMOD_LALT;
vkb.modKeys[SDLK._RMETA] = SDLK.KMOD_RMETA;
vkb.modKeys[129] = SDLK.KMOD_LMETA; // HP_"ORANGE"
vkb.modKeys[SDLK._LMETA] = SDLK.KMOD_LMETA;
// vkb.modKeys[SDLK._LSUPER] = SDLK.KMOD_;
// vkb.modKeys[SDLK._RSUPER] = SDLK.KMOD_;
vkb.modKeys[SDLK._MODE] = SDLK.KMOD_MODE
// vkb.modKeys[SDLK._COMPOSE] = SDLK.KMOD_;
