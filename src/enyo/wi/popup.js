
enyo.kind({
	name:				'wi.Popup',
	kind:				enyo.Popup,
	scrim:				true,
	modal:				true,
	className:			'enyo-popup enyo-popup-float wi-popup',
	showHideMode:		'transition',
	openClassName:		'open',
	
	published: {
		buttons: '',
		header: 'Popup',
		content: '',
		scroller: true,
	},
	
	chrome: [],
	
	initComponents: function() {
		if (this.scroller) {
			this.chrome = [
				{name: 'container', kind: 'VFlexBox', className: 'container', components:[
					{name: 'header', content: 'Popup', kind: 'Header', className: 'header', allowHtml: true},
			    	{name: 'scroller', kind: 'FadeScroller', className: 'scroller', flex: 1, autoVertical: true, horizontal: false, components: [
						{name: 'client', kind: enyo.Control, style: 'min-height: 100px;', allowHtml: true},
					]},
					{name: 'toolbar', kind: 'Toolbar', className: 'enyo-toolbar-light toolbar'},
				]},
			];
		}
		else {
			this.chrome = [
				{name: 'container', kind: 'VFlexBox', className: 'container', components:[
					{name: 'header', content: 'Popup', kind: 'Header', className: 'header', allowHtml: true},
					{name: 'client', kind: enyo.Control, style: 'min-height: 100px;', allowHtml: true},
					{name: 'toolbar', kind: 'Toolbar', className: 'enyo-toolbar-light toolbar'},
				]},
			];
		}
	    this.inherited(arguments);
	},
	
	render: function() {
	    this.inherited(arguments);
		this.$.header.setContent(this.header);
		this.$.toolbar.createComponents(this.buttons, {owner: this});
		this.$.toolbar.render();
	},
	componentsReady: function() {
		this.inherited(arguments);
		if (this.content) {
			this.$.client.setContent(this.content);
		}
	},
	
	setHeight: function(height) {
		this.setStyle('height: ' + height + ';');
	},
	setContent: function(text) {
		this.content = text;
	},
	
	pop: function() {
		this.openAtCenter();
	}
});
