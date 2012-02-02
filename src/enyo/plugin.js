enyo.kind({

	name: 'Terminal',
	kind: enyo.Hybrid,

	currentColors: [],
	currentKeys: [],

	events: {
		onPluginReady:'',
		onPluginConnected:'',
		onPluginDisconnected:'',
		onWindowTitleChanged:''
	},

	hybridReady: function() {
		this.addCallback("OSCevent", enyo.bind(this, "OSCevent"))
		this.setColors()
		this.setKeys()
		this.setScrollBufferLines(enyo.application.p.get('bufferlines'))
		this.pluginStatusChangedCallback('ready')
	},

	rendered: function() {
		this.pluginReady = false;
		if (this.hasNode()) {
			if (this.passTouchEvents) {
				this.node.addEventListener("touchstart", this.nullTouchHandler);
				this.node.addEventListener("touchend", this.nullTouchHandler);
				this.node.addEventListener("touchmove", this.nullTouchHandler);
			}
			this.node.ready = enyo.bind(this, this.hybridReady)
			this.deferredCallbacks.forEach(function(cb) {this.node[cb.name] = cb.callback;}, this);
		}
	},

	focus: function() {
		if (this.hasNode()) {
			this.hasNode().focus();
		}
	},

	keydownHandler: function(inSender, inEvent) {
		if (enyo.fetchDeviceInfo().platformVersionMajor == 2 && enyo.fetchDeviceInfo().platformVersionMinor == 2)
			this.keyDown(inEvent.keyCode, String.fromCharCode(parseInt(inEvent.keyIdentifier.substr(2), 16)))
	},

	keyupHandler: function(inSender, inEvent) {
		if (enyo.fetchDeviceInfo().platformVersionMajor == 2 && enyo.fetchDeviceInfo().platformVersionMinor == 2)
			this.keyUp(inEvent.keyCode, String.fromCharCode(parseInt(inEvent.keyIdentifier.substr(2), 16)))
	},

	/**
	 * Plugin Callbacks
	 */
	OSCevent: function(value, txt) {
		switch(parseInt(value,10)) {
			case 0:
			case 1:
			case 2:
				this.doWindowTitleChanged(txt);
				break;
		}
	},

	/**
	 * Plugin Methods
	 */
	setScrollBufferLines: function(lines) {
		this.callPluginMethodDeferred(null, 'setScrollBufferLines', lines)
	},

	getDimensions: function() {
		return enyo.json.parse(this.callPluginMethodDeferred(null, 'getDimensions'))
	},

	getFontSize: function() {
		return parseInt(this.callPluginMethodDeferred(null, 'getFontSize'),10)
	},

	setFontSize: function(fontSize) {
		return parseInt(this.callPluginMethodDeferred(null, 'setFontSize', fontSize),10)
	},

	setActive: function(active) {
		if (this.pluginReady) this.callPluginMethodDeferred(null, 'setActive', active);
	},

	inject: function(command) {
		this.callPluginMethodDeferred(null, 'inject', command)
	},

	hasPassword: function(user) {
		return parseInt(this.callPluginMethod('userHasPassword', user), 10)
	},

	setPassword: function(user, password) {
		this.callPluginMethod('userSetPassword', user, password)
	},

	addToGroup: function(user, group) {
		this.callPluginMethod('userAddToGroup', user, group)
	},

	setupSU: function(enable) {
		this.callPluginMethod('setupSU', enable)
	},

	pushKeyEvent: function(type,sym,unicode) {
		return parseInt(this.callPluginMethod('pushKeyEvent',type,sym,unicode))
	},

	/**
	 * Wrapper/Helper Methods
	 */
	keyDown: function(sym,unicode) {
		return this.pushKeyEvent(1,sym,unicode)
	},

	keyUp: function(sym,unicode) {
		return this.pushKeyEvent(0,sym,unicode)
	},

	resize: function(width, height) {
		this.setWidth(width)
		this.setHeight(height)
	},

	/**
	 * Random Shit
	 */
	hsvToRgb: function(h, s, v) {
		var r, g, b;
		var i;
		var f, p, q, t;

		// Make sure our arguments stay in-range
		h = Math.max(0, Math.min(360, h));
		s = Math.max(0, Math.min(100, s));
		v = Math.max(0, Math.min(100, v));

		// We accept saturation and value arguments from 0 to 100 because that's
		// how Photoshop represents those values. Internally, however, the
		// saturation and value are calculated from a range of 0 to 1. We make
		// That conversion here.
		s /= 100;
		v /= 100;

		if(s == 0) {
			// Achromatic (grey)
			r = g = b = v;
			return [Math.round(r * 255), Math.round(g * 255), Math.round(b * 255)];
		}

		h /= 60; // sector 0 to 5
		i = Math.floor(h);
		f = h - i; // factorial part of h
		p = v * (1 - s);
		q = v * (1 - s * f);
		t = v * (1 - s * (1 - f));

		switch(i) {
			case 0:
				r = v;
				g = t;
				b = p;
				break;

			case 1:
				r = q;
				g = v;
				b = p;
				break;

			case 2:
				r = p;
				g = v;
				b = t;
				break;

			case 3:
				r = p;
				g = q;
				b = v;
				break;

			case 4:
				r = t;
				g = p;
				b = v;
				break;

			default: // case 5:
				r = v;
				g = p;
				b = q;
		}

		return [Math.round(r * 255), Math.round(g * 255), Math.round(b * 255)];
	},

	setColors: function() {
		var colorScheme = enyo.application.p.get('colorScheme')
		var colorSchemes = enyo.application.p.get('colorSchemes')
		this.currentColors = colorSchemes[colorScheme]
		if (colorScheme == 'Black on Random Light')
			this.currentColors[17] = this.currentColors[19] = this.hsvToRgb(Math.floor(Math.random()*256),34,247)
		for (i in this.currentColors)
			this.callPluginMethodDeferred(null, 'setColor', i, this.currentColors[i][0], this.currentColors[i][1], this.currentColors[i][2])
	},

	decodeEscape: function(str) {
		return str.replace(/\\x([0-9A-Fa-f]{2})/g, function() {
	        return String.fromCharCode(parseInt(arguments[1], 16));
	    });
	},

	setKeys: function() {
		var inputScheme = enyo.application.p.get('inputScheme')
		var inputSchemes = enyo.application.p.get('inputSchemes')
		this.currentKeys = inputSchemes[inputScheme]
		for (var i=0; i<this.currentKeys.length; i++)
			this.callPluginMethodDeferred(null, 'setKey', i, this.decodeEscape(this.currentKeys[i]))
	},

})