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
		this.inherited(arguments);
		this.addClass('vkb');
		this.large();
		this.loadLayout(this.prefs.get('kbdLayout'));
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
		getKbdLayout(name, function (layout) {
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
					if (!c.hasOwnProperty('modifier')) c.modifier = 0;
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
  	
  	keyUp: function(inSender) {
  		var key = inSender.symbols[0][1];
		if (!inSender.modifier) {
			if (this.modstate & this.KMOD_LSHIFT || this.modstate & this.KMOD_CAPS) {
				if (inSender.symbols.length == 1)
					key = inSender.symbols[0][1].toUpperCase();
				else if (inSender.symbols.length > 1)
					key = inSender.symbols[1][1];
			}
		}
  		if (key != null) {
  			if (typeof key == 'number') {
  				this.modstate = this.terminal.keyUp(key, null)
  			} else if (typeof key == 'string') {
  				this.modstate = this.terminal.keyUp(null, key)
  			}
  		}
  	},
  	
  	keyDown: function(inSender) {
  		var key = inSender.symbols[0][1];
		if (!inSender.modifier) {
			if (this.modstate & this.KMOD_LSHIFT || this.modstate & this.KMOD_CAPS) {
				if (inSender.symbols.length == 1)
					key = inSender.symbols[0][1].toUpperCase();
				else if (inSender.symbols.length > 1)
					key = inSender.symbols[1][1];
			}
		}
  		if (key != null) {
  			if (typeof key == 'number') {
  				this.modstate = this.terminal.keyDown(key, null)
  			} else if (typeof key == 'string') {
  				this.modstate = this.terminal.keyDown(null, key)
  			}
  		}
  	}
	
})
