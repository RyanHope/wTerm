enyo.kind({

	name: "wTerm",
	kind: enyo.VFlexBox,
	align: 'center',

	prefs: new Prefs(),

	components: [
		{kind: "AppMenu", components: [
			{caption: "About", onclick: "openAbout"}
			//{caption: "Preferences", onclick: "openPrefs"}
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
        this.createComponent({
        	name: 'terminal',
			kind: 'Terminal',
			bgcolor: '000000',
			width: 1020, height: 400 // 30x145
		})
		this.createComponent({kind: 'vkb', name: 'vkb', terminal: this.$.terminal})
		this.$.terminal.vkb = this.$.vkb
		this.createComponent({kind: 'Preferences', name: 'preferences', prefs: this.prefs, onClose: 'refresh'})
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
	}

})
