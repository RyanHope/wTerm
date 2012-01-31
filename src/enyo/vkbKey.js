
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
		symbols: null
	},

	events: {
		ontouchstart: '',
		ontouchend: '',
		onmousedown: '',
		onmouseup: '',
		onmouseout: ''
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
			this.node.ontouchstart = enyo.bind(this,'handleTouchstart')
			//this.node.ontouchmove = this.doTouchMove;
			this.node.ontouchend = enyo.bind(this,'handleTouchend')
		}
	},

	handleTouchstart: function() {
		if (!this.disabled) {
			if (this.toggling)
				this.setDown(!this.down)
			else
				this.setDown(true)
			return this.doTouchstart()
		}
	},

	handleTouchend: function() {
		if (!this.disabled && !this.toggling) {
			this.setDown(false)
			return this.doTouchend()
		}
	},

	mousedownHandler: function() {
		if (!this.disabled) {
			if (this.toggling)
				this.setDown(!this.down)
			else
				this.setDown(true)
			return this.doMousedown()
		}
    },
	mouseupHandler: function() {
		if (!this.disabled && !this.toggling) {
			this.setDown(false)
			return this.doMouseup()
		}
	},
	mouseoutHandler: function() {
		if (!this.disabled && !this.toggling) {
			this.setDown(false)
			return this.doMouseout()
		}
	},
	
	mouseoverHandler: function() {},
	flickHandler: function() {},
	clickHandler: function() {},
	dragstartHandler: function() {},
	dragoverHandler: function() {},
	dragfinishHandler: function() {}

})
