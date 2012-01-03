enyo.kind({

	name: "wTerm",
	kind: enyo.VFlexBox,
	align: 'center',

	prefs: new Prefs(),

	components: [
		{kind: "AppMenu", components: [
			{caption: "About", onclick: "openAbout"},
			{name: 'vkbToggle', caption: "Hide Virtual Keyboard", onclick: 'toggleVKB'},
			{caption: "Preferences", onclick: "openPrefs", onClose: 'refresh'}
		]},
		{
			kind: 'Popup2',
			name: 'about',
			scrim: true,
			components: [
				{style: 'text-align: center; padding-bottom: 12px;', allowHtml: true, content: '<b><u>wTerm v'+enyo.fetchAppInfo().version+'</u></b>'},
				{name: 'fontsize', allowHtml: true, style: 'font-size: 80%;'},
				{name: 'dimensions', allowHtml: true, style: 'font-size: 80%;'},
			]
		}
	],

	initComponents: function() {
    	this.inherited(arguments)
    	this.createComponent({kind: 'Preferences', name: 'preferences', prefs: this.prefs, onClose: 'refresh'})
		this.createComponent({
    		name: 'terminal',
			kind: 'Terminal',
			prefs: this.prefs,
			bgcolor: '000000',
			width: 1020, height: 390 // 30x145
		})
		this.createComponent({kind: 'vkb', name: 'vkb', terminal: this.$.terminal})
		this.$.terminal.vkb = this.$.vkb
	},
	
	toggleVKB: function() {
		this.$.vkb.setShowing(!this.$.vkb.showing)
		if (this.$.vkb.showing)
			this.$.vkbToggle.setCaption('Hide Virtual Keyboard')
		else
			this.$.vkbToggle.setCaption('Show Virtual Keyboard')
	},

	openAbout: function() {
		this.$.about.openAtTopCenter()
		d = this.$.terminal.getDimensions()
		this.$.dimensions.setContent('<b>Rows x Cols:</b>  '+d[0]+' x '+d[1])
		this.$.fontsize.setContent('<b>Font Size:</b>  '+this.$.terminal.getFontSize())
	},

	openPrefs: function() {
		this.$.preferences.openAtTopCenter()
	},

	refresh: function() {
		var size = this.$.terminal.setFontSize(this.prefs.get('fontSize'))
		this.log("Font size: "+size)
	}

})
