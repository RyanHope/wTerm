enyo.kind({

	name: "wTermApp",
	kind: enyo.VFlexBox,

	showVKB: false,

	components: [
		{
			kind: "ApplicationEvents",
			onWindowRotated: "setup",
			onKeydown: "onBtKeyDown",
			onWindowActivated: 'windowActivated',
			onWindowDeactivated: 'windowDeactivated',
		},
		{
			name : "getPreferencesCall",
			kind : "PalmService",
			service : "palm://com.palm.systemservice/",
			subscribe: true,
			method : "getPreferences",
			onSuccess : "prefCallSuccess",
		},
		{
			kind: 'Popup2',
			name: 'about',
			scrim: true,
			components: [
				{style: 'text-align: center; padding-bottom: 6px; font-size: 120%;', allowHtml: true, content: '<img src="images/icon-64.png"/ style="vertical-align: middle; padding-right: 1em;"><b><u>wTerm v'+enyo.fetchAppInfo().version+'</u></b>'},
				{style: 'padding: 4px; text-align: center; font-size: 90%', content: '<a href="https://github.com/PuffTheMagic/wTerm">Project Home</a>'},
				{style: 'padding: 4px; text-align: center; font-size: 90%', content: '<a href="https://github.com/PuffTheMagic/wTerm/issues">Issues</a>'},
				{style: 'padding: 4px; text-align: center; font-size: 90%', content: '<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=VU4L7VTGSR5C2">Donate</a>'},
				{style: 'text-align: center; padding-top: 24px; font-style: italic; font-size: 60%', allowHtml: true, content: '&copy; 2011-2012 WebOS Internals'}
			]
		},
		{
			kind: 'Popup2',
			name: 'launchWarning',
			modal: true,
			scrim: true,
			autoClose: false,
			dismissWithClick: false,
			components: [
				{style: 'text-align: center; padding-bottom: 12px; font-size: 120%;', allowHtml: true, content: '<b><u>Notice!</u></b>'},
				{name: 'warninig', allowHtml: true, content: 'Another application is trying to run the following command(s):'},
				{name: 'command', allowHtml: true, style: 'font-family: monospace; padding-left: 1em; padding-bottom: 1em'},
				{kind: 'HFlexBox', components: [
					{kind: 'HFlexBox', flex: 2, align: 'center', components: [
						{kind: "CheckBox", name: 'launchParamsCheckbox', onChange: 'launchParamWarn'},
						{style: 'font-size: 80%; padding-left: 1em;', content: 'Do not show this warning again.'},
					]},
					{kind: 'HFlexBox', flex: 1, components: [
						{kind: 'Button', flex: 1, className: 'enyo-button-negative', content: 'Cancel', onclick: 'warningCancel'},
						{kind: 'Button', flex: 1, className: 'enyo-button-affirmative ', content: 'Ok', onclick: 'warningOk'},
					]}
				]}
			]
		}
	],
	launchParamWarn: function() {
		PREFS.set('launchParamsOK', this.$.launchParamsCheckbox.checked)
	},
	warningCancel: function() {
		this.$.launchWarning.close()
		window.close()
	},
	warningOk: function() {
		this.$.launchWarning.close()
		this.$.terminal.inject(enyo.windowParams.command)
	},

	newTerm: function(inSender, inEvent, params, reactivate) {
		enyo.application.m.launch(true)
	},

	windowActivated: function() {
		this.$.terminal.setActive(1)
	},
	windowDeactivated: function() {
		this.$.terminal.setActive(0)
	},

	initComponents: function() {
		this.inherited(arguments)
		this.showVKB = PREFS.get('showVKB')
		this.createComponent({
			name: "prefs",
			kind: "Preferences",
			style: "width: 320px; top: 0px; bottom: 0; margin-bottom: 0px;", //width: 384px
			className: "enyo-bg",
			flyInFrom: "right",
			onOpen: "pulloutToggle",
			onClose: "closeRightPullout"
		})
		this.createComponent({
		name: 'terminal',
			kind: 'Terminal',
			bgcolor: '000000',
			width: window.innerWidth,
			height: 400,
			onPluginReady: 'pluginReady',
			exec: enyo.windowParams.root ? 'login -f root' : PREFS.get('exec')
		})
		this.createComponent({kind: 'vkb', name: 'vkb', terminal: this.$.terminal, showing: true})
		this.$.terminal.vkb = this.$.vkb
		this.$.prefs.terminal = this.$.terminal
		this.$.prefs.vkb = this.$.vkb
		this.createComponent({
			kind: "AppMenu", components: [
				{caption: "New Terminal", onclick: "newTerm"},
				{caption: "Preferences", onclick: "openPrefs"},
				{name: 'vkbToggle', caption: this.getVKBMenuText(), onclick: 'toggleVKB'},
				{caption: "About", onclick: "openAbout"}
			]
		})
		this.setup()
	},

	pluginReady: function() {
		if (enyo.windowParams.command) {
			if (PREFS.get('launchParamsOK')) {
				this.$.terminal.inject(enyo.windowParams.command)
			} else {
				this.$.launchWarning.openAtTopCenter()
				this.$.command.setContent(enyo.windowParams.command)
			}
		}
	},

	setupKeyboard: function(portrait) {
		if (portrait) {
			this.$.vkb.small()
			if (this.showVKB)
				this.$.terminal.resize(window.innerWidth, 722)
			else
				this.$.terminal.resize(window.innerWidth, window.innerHeight)
		} else {
			this.$.vkb.large()
			if (this.showVKB)
				this.$.terminal.resize(window.innerWidth, 400)
			else
				this.$.terminal.resize(window.innerWidth, window.innerHeight)
		}
	},

	prefCallSuccess: function(inSender, inResponse) {
		if (inResponse.rotationLock == 3 || inResponse.rotationLock == 4)
			this.setupKeyboard(false)
		else if (inResponse.rotationLock == 5 || inResponse.rotationLock == 6)
			this.setupKeyboard(true)
		else {
			var o = enyo.getWindowOrientation()
			if (o == 'up' || o == 'down')
				this.setupKeyboard(false)
			else
				this.setupKeyboard(true)
		}
	},

	getVKBMenuText: function() {
		return this.showVKB ? 'Hide Virtual Keyboard' : 'Show Virtual Keyboard'
	},

	setVKBMenu: function() {
		this.$.vkbToggle.setCaption(this.getVKBMenuText())
	},

	toggleVKB: function() {
		this.showVKB = !this.showVKB
		PREFS.set('showVKB', this.showVKB)
		this.setVKBMenu()
		this.setup()
	},

	openAbout: function() {
		this.$.about.openAtTopCenter()
	},

	openPrefs: function() {
		if (this.$.prefs.showing)
			this.$.prefs.close();
		else
			this.$.prefs.open();
	},

	setup: function() {
		this.$.getPreferencesCall.call({"keys":["rotationLock"]});
	},

	onBtKeyDown: function(context, event) {
		if (this.$.terminal.$.plugin.hasNode())
		{
			this.$.terminal.$.plugin.node.focus();
			this.$.terminal.$.plugin.node.dispatchEvent(event);
		}
	}

})
