
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
		terminal: null,
	},

	events: {
		ontouchstart: '',
		ontouchend: '',
		onmousedown: '',
		onmouseup: ''
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
			this.node.ontouchend = enyo.bind(this,'handleTouchend')
		}
	},

	handleTouchstart: function() {
		if (!this.disabled) {
			if (this.toggling)
				this.setDown(!this.down)
			else
				this.setDown(true)
			this.doTouchstart()
		}
	},

	handleTouchend: function(inEvent) {
		// Obviously don't want to do this every key input, only on those rare double inputs
		if (inEvent && inEvent.changedTouches && inEvent.changedTouches.length > 1)
		{
			for (var i=0; i < inEvent.changedTouches.length; i++)
			{
				// Try to find our parent div as our current HTMLElement could be any child of it
				var tobj = inEvent.changedTouches[i].target;
				while (tobj)
				{
					// would it be faster to just check if enyo.$[tobj.id] && enyo.$[tobj.id].kind == 'vkbKey'?
					if (/^wTermApp_vkb_vkbKey\d*$/.test(tobj.id))
						break;
					tobj = tobj.parentElement;
				}

				// Went all the way up to document and found nothing :(
				if (!tobj)
					continue;

				// Grab the enyo instance
				tobj = enyo.$[tobj.id];

				if (!tobj) // maybe pointless
					continue;

				if (tobj != this)
					tobj.handleTouchend(null);
			}

		}
		
		if (!this.disabled && !this.toggling) {
			this.setDown(false)
			this.doTouchend()
		}
	},

	mousedownHandler: function() {
		if (!this.disabled) {
			if (this.toggling)
				this.setDown(!this.down)
			else
				this.setDown(true)
			this.doMousedown()
		}
	},
	mouseupHandler: function() {
		if (!this.disabled && !this.toggling) {
			this.setDown(false)
			this.doMouseup()
		}
	},

	mouseoutHandler: function() {},
	mouseoverHandler: function() {},
	flickHandler: function() {},
	clickHandler: function() {},
	dragstartHandler: function() {},
	dragoverHandler: function() {},
	dragfinishHandler: function() {}

})
