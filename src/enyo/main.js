enyo.kind({

	name: "wTerm",
	kind: enyo.VFlexBox,
	align: 'center',

	prefs: new Prefs(),

	components: [
		{kind: "AppMenu", components: [
			//{caption: "Preferences", onclick: "openPrefs"}
		]}
	],

  	initComponents: function() {
        this.inherited(arguments)
        this.createComponent({
        	name: 'terminal',
			kind: 'Terminal',
			width: 1020, height: 400 // 127x25
		})
		this.createComponent({kind: 'vkb', name: 'vkb', terminal: this.$.terminal})
		this.$.terminal.vkb = this.$.vkb
		this.createComponent({kind: 'Preferences', name: 'preferences', prefs: this.prefs, onClose: 'refresh'})
	},

	openPrefs: function() {
		this.$.preferences.openAtTopCenter()
	},

	refresh: function() {
	}

})
