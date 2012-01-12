enyo.kind({
	
	name: "vkb",
  	kind: 'VFlexBox',
  	flex: 1,
  	width: '100%',
  	
	shift: 1,
	ctrl: 2,
	alt: 4,
	fn: 8,
	caps: 16,
  	
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
					if (e.content) {
						c = enyo.mixin({kind: 'vkbKey', className: '', ontouchstart: 'keyDown', ontouchend: 'keyUp'}, e);
					} else {
						c = enyo.mixin({className: ''}, e); /* simple flex or custom */
					}
					if (c.small) c.className += ' small';
					if (c.extraClasses) c.className += ' ' + c.extraClasses;
					if (!c.hasOwnProperty('unicode')) c.unicode = c.content;
					comps.push(c);
				}
				components.push({layoutKind: 'HFlexLayout', pack: 'end', components: comps});
			}
			this.destroyComponents();
			this.createComponents(components);
			this.render();
		}.bind(this));
	},

  	keyUp: function(inSender) {
		this.terminal.keyUp(inSender.sym, null)
  	},
  	
  	keyDown: function(inSender) {
		this.terminal.keyDown(inSender.sym, inSender.unicode)
  	},
  	
/*	btnClick: function(inSender, inEvent) {
		var key = inSender.content.split('<br>').reverse()
		switch (key[0]) {
			case 'Tab':
				this.terminal.write('\x09')
				break
			case 'Up':
				(this.terminal.modes['appkeys']) ? this.terminal.write('\033OA') : this.terminal.write('\033[A')					
				break
			case 'Down':
				(this.terminal.modes['appkeys']) ? this.terminal.write('\033OB') : this.terminal.write('\033[B')
				break
			case 'Left':
				(this.terminal.modes['appkeys']) ? this.terminal.write('\033OD') : this.terminal.write('\033[D')
				break
			case 'Right':
				(this.terminal.modes['appkeys']) ? this.terminal.write('\033OC') : this.terminal.write('\033[C')
				break
			case 'Enter':
				if (this.terminal.modes['newline']) {
					this.terminal.write('\x0d')
					this.terminal.write('\x0a')
				} else
					this.terminal.write('\x0d')
				break
			case 'Esc':
				this.terminal.write('\x1b')
				break
			case 'Bksp':
				this.terminal.write('\x7f')
				break
			case 'Space':
				this.terminal.write(' ')
				break
			default:
				if (this.mode == this.ctrl) {
					var base = key[0].toUpperCase().charCodeAt(0)
					if (key.length>1 && (key[1]=="@" || key[1]=="~" || key[1]=="^" || key[1]=="?" || key[1]=="_"))
						base = key[1].charCodeAt(0)
					if (base > 63 && base < 96)
						this.terminal.write(String.fromCharCode(base-64))
				} else if (this.mode == this.caps || this.mode == this.shift) {
					if (key.length>1) {
						this.terminal.write(key[1])
					} else {
						this.terminal.write(key[0].toUpperCase())
					}
				} else {
					this.terminal.write(key[0])
				}
		}
		
	},
*/
	// Handle special modifier key states here

	toggleCaps: function(inSender, inEvent) {
		if (inSender.down )
			this.mode += this.caps
		else
			this.mode -= this.caps
	},
	
	shiftDown: function() {
		this.mode += this.shift
	},
	
	shiftUp: function() {
		this.mode -= this.shift
	},
	
	fnDown: function() {
		this.mode += this.fn
	},
	
	fnUp: function() {
		this.mode -= this.fn
	},
	
	altDown: function() {
		this.mode += this.alt
	},
	
	altUp: function() {
		this.mode -= this.alt
	},
	
	ctrlDown: function() {
		this.mode += this.ctrl
	},
	
	ctrlUp: function() {
		this.mode -= this.ctrl
	}
	
})
