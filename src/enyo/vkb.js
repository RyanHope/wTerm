enyo.kind({

	name: "vkb",
	kind: 'VFlexBox',
	flex: 1,
	width: '100%',

	modstate: 0,

	KMOD_NONE: 0x0000,
	KMOD_LSHIFT: 0x0001,
	KMOD_RSHIFT: 0x0002,
	KMOD_LCTRL: 0x0040,
	KMOD_RCTRL: 0x0080,
	KMOD_LALT: 0x0100,
	KMOD_RALT: 0x0200,
	KMOD_LMETA: 0x0400,
	KMOD_RMETA: 0x0800,
	KMOD_NUM: 0x1000,
	KMOD_CAPS: 0x2000,
	KMOD_MODE: 0x4000,

	published: {
		terminal: null,
		mode: 0
	},

	create: function() {
		this._layoutName = '';
		this.inherited(arguments);
		this.addClass('vkb');
		this.large();
		this.loadLayout(enyo.application.p.get('kbdLayout'));
	},

	large: function() {
		this._large = true;
		this.addClass('large');
		this.removeClass('small');
	},
	small: function() {
		this._large = false;
		this.addClass('small');
		this.removeClass('large');
	},

	loadLayout: function(name) {
		this._layoutName = name;
		getKbdLayout(name, function (layout) {
			if (name != this._layoutName) return;
			var components = [], i, j, comps, c, e;
			for (i = 0; i < layout.length; i++) {
				row = layout[i];
				comps = [];
				for (j = 0; j < row.length; j++) {
					e = row[j];
					if (e.content || e.symbols) {
						if (e.hasOwnProperty('toggling') && c.toggling)
							c = enyo.mixin({kind: 'vkbKey', className: '', ontouchstart: 'keyToggle'}, e);
						else
							c = enyo.mixin({kind: 'vkbKey', className: '', ontouchstart: 'keyDown', ontouchend: 'keyUp'}, e);
					} else {
						c = enyo.mixin({className: ''}, e); /* simple flex or custom */
					}
					if (c.small) c.className += ' small';
					if (c.extraClasses) c.className += ' ' + c.extraClasses;
					if (!c.hasOwnProperty('printable')) c.printable = false;
					if (!c.hasOwnProperty('unicode') && c.content) c.unicode = c.content.toLowerCase();
					comps.push(c);
				}
				components.push({layoutKind: 'HFlexLayout', pack: 'end', components: comps});
			}
			this.destroyComponents();
			this.createComponents(components);
			this.render();
		}.bind(this));
	},

	keyToggle: function(inSender) {
		if (!inSender.down)
			this.keyUp(inSender)
		else
			this.keyDown(inSender)
	},

	isShift: function() {
		return ((this.modstate & this.KMOD_LSHIFT) || (this.modstate & this.KMOD_RSHIFT) || (this.modstate & this.KMOD_CAPS))
	},
	isMode: function() {
		return (this.modstate & this.KMOD_MODE)
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

	keyUp: function(inSender) {
		var k = this.processKey(inSender)
		this.modstate = this.terminal.keyUp(k[0], k[1])
	},

	keyDown: function(inSender) {
		var k = this.processKey(inSender)
		this.modstate = this.terminal.keyDown(k[0], k[1])
	}

})
