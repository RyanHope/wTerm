enyo.application.vkbLayouts = []
enyo.depends('src/enyo/vkbLayouts/')

enyo.kind({
	
	name: 'vkb',
	kind: enyo.Stateful,
	
	width: '100%',
	
	cssNamespace: 'vkb',
	
	published: {
		caption: '',
		landscape: false,
		layout: null,
	},
	
	setupLayout: function() {
		var components = [], i, j, comps, c, e;
		for (i = 0; i < this.layout.length; i++) {
			row = this.layout[i];
			comps = [];
			for (j = 0; j < row.length; j++) {
				e = row[j];
				if (e.content || e.symbols) {
					if (e.hasOwnProperty('toggling') && c.toggling)
						c = enyo.mixin({kind: 'vkbKey', className: '', isPhone: false, ondown: 'vkbKeyToggle'}, e);
					else
						c = enyo.mixin({kind: 'vkbKey', className: '', isPhone: false, ondown: 'vkbKeyDown', onup: 'vkbKeyUp'}, e);
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
		this.createComponents([{kind: 'VFlexBox', name: 'keys', components: components}]);
	},
	
	initComponents: function() {
		this.inherited(arguments)
		this.setupLayout()
		this.landscapeChanged()
	},
	
	landscapeChanged: function() {
		this.stateChanged("landscape")
	},
	
	setTerminal: function(term) {
		this.terminal = term
	},
	
})