enyo.kind({
	
	name: "vkb",
  	kind: 'VFlexBox',
  	flex: 1,
  	width: '100%',
  	
	modstate: 0,
  	
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
						c = enyo.mixin({kind: 'vkbKey', className: '', ontouchstart: 'keyDown', ontouchend: 'keyUp'}, e);
					} else {
						c = enyo.mixin({className: ''}, e); /* simple flex or custom */
					}
					if (c.small) c.className += ' small';
					if (c.extraClasses) c.className += ' ' + c.extraClasses;
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
  	
  	keyUp: function(inSender) {
		var key = inSender.symbols[0][1];
  		if (key != null) {
  			if (typeof key == 'number') {
  				this.modstate = this.terminal.keyDown(key, null)
  			} else if (typeof key == 'string') {
  				this.modstate = this.terminal.keyDown(null, key)
  			}
  		}
  		this.log('Modstate: '+ this.modstate)
  	},
  	
  	keyDown: function(inSender) {
  		var key = inSender.symbols[0][1];
  		if (key != null) {
  			if (typeof key == 'number') {
  				this.modstate = this.terminal.keyDown(key, null)
  			} else if (typeof key == 'string') {
  				this.modstate = this.terminal.keyDown(null, key)
  			}
  		}
  		this.log('Modstate: '+ this.modstate)
  	}
	
})
