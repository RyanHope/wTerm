enyo.kind({

	name: "wTermApp",
	kind: enyo.VFlexBox,
	
	_isPhone: null,
	_orientation: null,
	_rotationLock: null,
	_landscape: null,
	_vkbClass: null,
	_showVKB: true,
	_windowTitle: null,
	
	_headerHeight: 29,
	
	components: [
		{
			name : "sysSound",
			kind : "PalmService",
			service : "palm://com.palm.audio/systemsounds",
			method : "playFeedback"
		},
		{
			name : "getPreferencesCall",
			kind : "PalmService",
			subscribe: true,
			service : "palm://com.palm.systemservice/"
		},
		{
			name: "prefs",
			kind: "PrefsPullout",
			style: "width: 320px; position: absolute; margin-bottom: 0px; right: 0", //width: 384px
			className: "enyo-bg",
			flyInFrom: "right",
			onOpen: "pulloutToggle",
			onClose: "closeRightPullout",
			onVKBLayoutChange: 'VKBLayoutChange'
		},
		{
			kind: 'Popup2',
			name: 'about',
			scrim: true,
			components: [
				{style: 'text-align: center; padding-bottom: 6px; font-size: 120%;', allowHtml: true, content: '<img src="images/icon-64.png"/ style="vertical-align: middle; padding-right: 1em;"><b><u>wTerm v'+enyo.fetchAppInfo().version+'</u></b>'},
				{style: 'padding: 4px; text-align: center; font-size: 90%', content: '<a href="https://github.com/RyanHope/wTerm">Project Home</a>'},
				{style: 'padding: 4px; text-align: center; font-size: 90%', content: '<a href="https://github.com/RyanHope/wTerm/issues">Issues</a>'},
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
		enyo.application.p.set('launchParamsOK', this.$.launchParamsCheckbox.checked)
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
	
	create: function() {
		this.inherited(arguments)
		enyo.setFullScreen(true)
		this._isPhone = (enyo.fetchDeviceInfo().keyboardAvailable || enyo.fetchDeviceInfo().keyboardSlider)
		this._showVKB = enyo.application.p.get('showVKB')
		if (enyo.fetchDeviceInfo().platformVersionMajor == 2 && enyo.fetchDeviceInfo().platformVersionMinor == 1)
			enyo.setAllowedOrientation("up")
		this.getRotationLock()
	},
	
	initComponents: function() {
		this.inherited(arguments)
		this.createComponent({
			kind: "AppMenu", components: [
				{caption: "New Terminal", onclick: "newTerm"},
				{caption: "Preferences", onclick: "openPrefs"},
				{caption: "Setup", onclick: "openSetup"},
				//{name: 'vkbToggle', caption: this.getVKBMenuText(), onclick: 'toggleVKB'},
				{caption: "Paste", onclick: "doPaste"},
				{caption: "About", onclick: "openAbout"}
			]
		})
	},
	
	createVKB: function(createTerm) {
		this.createComponent({
			kind: enyo.application.p.get('kbdLayout'),
			name: 'vkb',
			landscape: this._landscape,
			phone: this._isPhone
		})
		this.$.vkb.render()
		if (createTerm)
			this.createTerminal(this.$.vkb.node.clientHeight)
		this.finalize()
	},
	
	createTerminal: function(vkbHeight) {
		if (this.$.terminal) {
			this.$.terminal.setHeight(window.innerHeight - vkbHeight - this._headerHeight);
			return;
		}
		var exec = enyo.application.p.get('exec')
		if (enyo.windowParams.root && !enyo.windowParams.command)
			exec = 'login -f root'
		else if (enyo.windowParams.command)
			exec = 'login -f wterm'
		var term = this.createComponent({
			name: 'terminal',
			kind: 'Terminal',
			executable: 'wterm',
			width: window.innerWidth,
			height: window.innerHeight - vkbHeight - this._headerHeight,
			onBell: 'bell',
			onPluginReady: 'pluginReady',
			onWindowTitleChanged: 'windowTitleChanged',
			allowKeyboardFocus: true,
			bgcolor: '000000',
			params: [enyo.application.p.get('fontSize').toString(10), exec]
		})
		term.prepend = true
		term.render()
		this.createComponent({
			kind: "ApplicationEvents",
			onWindowActivated: 'windowActivated',
			onWindowDeactivated: 'windowDeactivated',
			onKeydown: 'dispatchKeyInput',
			onKeyup: 'dispatchKeyInput'
		})
		var header = this.createComponent({
			kind: "HFlexBox",
			name: 'toolbar',
			style: 'background-color: black; width: 100%; padding: 0px; margin: 0px; height: '+this._headerHeight+'px',
			components: [{
				kind: "HFlexBox",
				pack: 'center',
				align: 'center',
				className: 'termToolbar',
				components: [
					{kind: 'CustomButton', layoutKind: 'HFlexLayout', className: 'menutext', onclick: 'showAppMenu', components: [
						{content: 'Menu'},
						{className: 'arrowUp'}
					]},
					{name: "termTitle", flex: 1, className: "title", content: 'wTerm'},
					{kind: 'CustomButton', layoutKind: 'HFlexLayout', className: 'menutext', onclick: 'toggleVKB', components: [
						{name: 'vkbButtonTxt', content: this.getVKBMenuText()},
						{name: 'vkbButtonImg', className: this.getVKBMenuClass()}
					]},
				]
			}]
		})
		header.prepend = true
		header.render()
	},
	
	showAppMenu: function(inSender, inEvent) {
		this.$.appMenu.openAtControl(inSender, {top: 28})
	},

	finalize: function() {
		this.$.vkb.setTerminal(this.$.terminal)
		this.$.terminal.vkb = this.$.vkb
		this.$.prefs.vkb = this.$.vkb
		this.$.prefs.terminal = this.$.terminal
	},

	windowTitleChanged: function(inSender, txt) {
		this.$.termTitle.setContent(txt)
	},

	bell: function() {
		this.$.sysSound.call({"name": "error_02"})
	},

	pluginReady: function() {
		if (enyo.windowParams.command) {
			if (enyo.application.p.get('launchParamsOK')) {
				this.$.terminal.inject(enyo.windowParams.command)
			} else {
				this.$.launchWarning.openAtTopCenter()
				this.$.command.setContent(enyo.windowParams.command)
			}
		}
	},

	setupKeyboard: function() {
		if (typeof this.$.vkb != 'undefined' && this.$.vkb.hasNode()) {
			var width = window.innerWidth
			var height = window.innerHeight - this._headerHeight
			this.$.vkb.setLandscape(this._landscape)
			if (this._showVKB)
				height = height - this.$.vkb.hasNode().scrollHeight
			if (typeof this.$.terminal != 'undefined' && this.$.terminal.hasNode())
				this.$.terminal.resize(width, height)
		} else {
			enyo.asyncMethod(this, enyo.bind(this, this.setupKeyboard))
		}
	},

	getRotationLock: function() {
		this.$.getPreferencesCall.call(
			{"keys": ["rotationLock"]},
			{method : "getPreferences",onSuccess : "rotationLockReponse"}
		)
	},

	rotationLockReponse: function(inSender, inResponse) {		
		var tmpOrientation = enyo.getWindowOrientation()
		if (typeof inResponse.rotationLock != 'undefined')
			this._rotationLock = inResponse.rotationLock
		else
			this._rotationLock = 0
		if (this._rotationLock == 3)
			tmpOrientation = 'up'
		else if (this._rotationLock == 4)
			tmpOrientation = 'down'
		else if (this._rotationLock == 5)
			tmpOrientation = 'left'
		else if (this._rotationLock == 6)
			tmpOrientation = 'right'
		if (this._landscape == null || this._orientation != tmpOrientation) {
			this._orientation = tmpOrientation
			this.processOrientation()
		}
		if (typeof inResponse.returnValue === 'undefined')
			this.refresh()
		else if (inResponse.returnValue)
			this.createVKB(true)
	},

	getVKBMenuText: function() {
		return this._showVKB ? 'Hide VKB' : 'Show VKB'
	},
	
	getVKBMenuClass: function() {
		return this._showVKB ? 'arrowDown' : 'arrowUp'
	},

	setVKBMenu: function() {
		//this.$.vkbToggle.setCaption(this.getVKBMenuText())
		this.$.vkbButtonTxt.setContent(this.getVKBMenuText())
		this.$.vkbButtonImg.setClassName(this.getVKBMenuClass())
	},

	toggleVKB: function() {
		this._showVKB = !this._showVKB
		enyo.application.p.set('showVKB', this._showVKB)
		this.setVKBMenu()
		this.resizeHandler()
	},

	openAbout: function() {
		this.$.about.openAtTopCenter()
	},

	openSetup: function() {
		enyo.windows.openWindow('app.html', null, {setup: true}, null);
	},
	
	openPrefs: function() {
		if (this.$.prefs.showing)
			this.$.prefs.close();
		else
			this.$.prefs.open();
	},

	VKBLayoutChange: function() {
		this.$.vkb.destroy()
		this.createVKB(false)
		this.resized()
	},

	doPaste: function () {
		enyo.dom.getClipboard(enyo.bind(this, this.handleClipboard))
	},

	handleClipboard: function (clipData) {
		this.$.terminal.inject(unescape(clipData), 1)
	},

	refresh: function() {
		this.setupKeyboard()
		this.$.prefs.updateHeight()
	},
	
	processOrientation: function() {
		if (this._orientation == 'up' || this._orientation == 'down')
			this._landscape = !this._isPhone
		else
			this._landscape = this._isPhone
	},
	
	resizeHandler: function(inSender, inEvent) {
		var tmpOrientation = enyo.getWindowOrientation()
		if (this._rotationLock == 0 && (this._orientation != tmpOrientation))
			this._orientation = tmpOrientation
		this.processOrientation()
		this.refresh()
	},

	dispatchKeyInput: function(inSender, inEvent) {
		this.$.terminal.focus();
		this.$.terminal.hasNode().dispatchEvent(inEvent);
	},

	rendered: function() {
		this.inherited(arguments);
		if (this.hasNode() && !this._isPhone) {
			this.node.addEventListener("touchstart", enyo.bind(this, this.handleTouchstart), false);
			this.node.addEventListener("touchend", enyo.bind(this, this.handleTouchend), false);
			this.node.addEventListener("touchcancel", enyo.bind(this, this.handleTouchend), false);
		}
	},

	getEnyoObjectFromElement: function(inElement) {
		var enyoObject = null;
		while (inElement)
		{
			enyoObject = enyo.$[inElement.id];
			if (enyoObject && enyoObject.kind == 'vkbKey')
				return enyoObject;
			inElement = inElement.parentElement;
		}
		return null;
	},

	handleTouchstart: function(inEvent) {
		for (var i = 0; i < inEvent.changedTouches.length; i++)
		{
			var enyoObject = this.getEnyoObjectFromElement(inEvent.changedTouches[i].target);
			if (enyoObject)
				enyoObject.handleDownEvent.call(enyoObject, inEvent);
		}
	},

	handleTouchend: function(inEvent) {
		for (var i = 0; i < inEvent.changedTouches.length; i++)
		{
			var enyoObject = this.getEnyoObjectFromElement(inEvent.changedTouches[i].target);
			if (enyoObject)
				enyoObject.handleUpEvent.call(enyoObject, inEvent);
		}
	}
})
