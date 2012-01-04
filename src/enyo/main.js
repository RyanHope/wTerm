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
    	this.createComponent({
			name: "prefs", 
			kind: "Preferences", 
			style: "width: 320px; top: 0px; bottom: 0; margin-bottom: 0px;", //width: 384px
			className: "enyo-bg",
			flyInFrom: "right",
			onOpen: "pulloutToggle",
			onClose: "closeRightPullout",
			prefs: this.prefs
		})
		this.createComponent({
    		name: 'terminal',
			kind: 'Terminal',
			prefs: this.prefs,
			bgcolor: '000000',
			width: window.innerWidth, height: 400 // 30x145
		})
		this.createComponent({kind: 'vkb', name: 'vkb', terminal: this.$.terminal})
		this.$.terminal.vkb = this.$.vkb
		this.$.prefs.terminal = this.$.terminal
	},
	
	toggleVKB: function() {
		this.$.vkb.setShowing(!this.$.vkb.showing)
		if (this.$.vkb.showing) {
			this.$.vkbToggle.setCaption('Hide Virtual Keyboard')
			this.$.terminal.resize(window.innerWidth, 400)
		} else {
			this.$.vkbToggle.setCaption('Show Virtual Keyboard')
			this.$.terminal.resize(window.innerWidth, window.innerHeight)
		}
	},

	openAbout: function() {
		this.$.about.openAtTopCenter()
		d = this.$.terminal.getDimensions()
		this.$.dimensions.setContent('<b>Rows x Cols:</b>  '+d[0]+' x '+d[1])
		this.$.fontsize.setContent('<b>Font Size:</b>  '+this.$.terminal.getFontSize())
	},

	openPrefs: function() {
		//this.$.preferences.openAtTopCenter()
		if (this.$.prefs.showing)
			this.$.prefs.close();
		else {
			//this.$.messages.hasNode();
			//this.$.prefs.domStyles['height'] = this.$.messages.node.clientHeight + 'px';
			this.$.prefs.open();
			//this.$.nicks.render();
		}
	},

	refresh: function() {
		var size = this.$.terminal.setFontSize(this.prefs.get('fontSize'))
		this.log("Font size: "+size)
	}

})
