
function vbkKeyContent(symbol) {
	if (!symbol) return '';
	return (3 <= symbol.length) ? symbol[2] : symbol[0];
}

function arrayRemoveNull(l) {
	var r = [], i;
	for (i = 0; i < l.length; i++) if (l[i] !== null) r.push(l[i]);
	return r;
}

enyo.kind({
	name: 'vkbKey',
	kind: enyo.Button,
	className: '',
	allowHtml: true,
	layoutKind: 'HFlexLayout',
	pack: 'center',
	align: 'center',

	published: {
		down: false,
		depressed: false,
		toggling: false,
		disabled: false,
		sym: -1,
		symbols: null,
		isPhone: false,
	},

	events: {
		ondown: '',
		onup: '',
	},

	create: function() {
		this.inherited(arguments);
		this.addClass('enyo-button');
		this.addClass('key');
	},

	initComponents: function() {
		this.inherited(arguments)
		if (this.symbols) {
			if (!this.visual) this.visual = arrayRemoveNull(this.symbols);
			switch(this.visual.length) {
			case 1:
				this.createComponents([
					{flex: 1, content: vbkKeyContent(this.visual[0])}
				]);
				break;
			case 2:
				this.createComponents([
					{flex: 1, kind: 'VFlexBox', components: [
						{content: vbkKeyContent(this.visual[1])},
						{content: vbkKeyContent(this.visual[0])},
					]}
				]);
				break;
			case 3:
				this.createComponents([
					{flex: 1, kind: 'VFlexBox', components: [
						{content: vbkKeyContent(this.visual[1])},
						{content: vbkKeyContent(this.visual[0])},
					]},
					{flex: 1, kind: 'VFlexBox', pack: 'start', components: [
						{content: vbkKeyContent(this.visual[2]), flex: 1}
					]},
				]);
				break;
			case 4:
				this.createComponents([
					{flex: 1, kind: 'VFlexBox', components: [
						{content: vbkKeyContent(this.visual[1])},
						{content: vbkKeyContent(this.visual[0])},
					]},
					{flex: 1, kind: 'VFlexBox', components: [
						{content: vbkKeyContent(this.visual[3])},
						{content: vbkKeyContent(this.visual[2])}
					]}
				]);
				break;
		}
	    }
	},

	rendered: function() {
		this.inherited(arguments);
		if (this.hasNode()) {
			if (this.isPhone) {
				this.node.addEventListener("mousedown", enyo.bind(this, this.handleDownEvent), false);
				this.node.addEventListener("mouseout", enyo.bind(this, this.handleUpEvent), false);
				this.node.addEventListener("mouseup", enyo.bind(this, this.handleUpEvent), false);
			}
		}
	},

	handleUpEvent: function(inEvent) {
		if (!this.disabled && !this.toggling) {
			this.setDown(false)
			this.doUp()
		}
	},

	handleDownEvent: function(inEvent) {
		if (!this.disabled) {
			if (this.toggling)
				this.setDown(!this.down)
			else
				this.setDown(true)
			this.doDown()
		}
	},

	mousedownHandler: function() {},
	mouseupHandler: function() {},
	mouseoutHandler: function() {},	
	mouseoverHandler: function() {},
	flickHandler: function() {},
	clickHandler: function() {},
	dragstartHandler: function() {},
	dragoverHandler: function() {},
	dragfinishHandler: function() {}

})
