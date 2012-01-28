enyo.kind({
	
	name: 'Terminal',
	kind: enyo.Control,
			
	published: {
		width: 0,
		height: 0,
		vkb: null,
		currentColors: [],
		currentKeys: [],
		isReady: false,
		exec: null,
	},
		
	events: {
		onPluginReady:''
	},

	initComponents: function() {
		this.createComponent({
			name: 'plugin',
			kind: enyo.Hybrid,
			executable: 'wterm',
			onPluginReady: 'pluginReady',
			onPluginConnected: 'pluginConnected',
			onPluginDisconnected: 'pluginDisconnected',
			allowKeyboardFocus: true,
			killTransparency: true,
			passTouchEvents: true,
			width: this.width,
			height: this.height,
			params: [enyo.application.p.get('fontSize').toString(10), this.exec]
		})
	},
		
  	pluginReady: function(inSender, inResponse, inRequest) {
  		this.isReady = true
		this.setColors()
		this.setKeys()
		this.setScrollBufferLines(enyo.application.p.get('bufferlines'))
  		this.doPluginReady()
  	},
  	pluginConnected: function(inSender, inResponse, inRequest) {
  	},
  	pluginDisconnected: function(inSender, inResponse, inRequest) {
  		this.error('Terminal Plugin Disconnected')
  	},

  	pushKeyEvent: function(type,sym,unicode) {
  		return parseInt(this.$.plugin.callPluginMethod('pushKeyEvent',type,sym,unicode))
  	},
  	keyDown: function(sym,unicode) {
  		return this.pushKeyEvent(1,sym,unicode)
  	},
  	keyUp: function(sym,unicode) {
  		return this.pushKeyEvent(0,sym,unicode)
  	},
  	
  	resize: function(width, height) {
  		this.$.plugin.setWidth(width)
  		this.$.plugin.setHeight(height)
  	},
  	
	setScrollBufferLines: function(lines) {
		this.$.plugin.callPluginMethodDeferred(null, 'setScrollBufferLines', lines)
	},

  	getDimensions: function() {
  		return enyo.json.parse(this.$.plugin.callPluginMethodDeferred(null, 'getDimensions'))
  	},
  	
  	getFontSize: function() {
  		return parseInt(this.$.plugin.callPluginMethodDeferred(null, 'getFontSize'),10)
  	},
  	
  	setFontSize: function(fontSize) {
  		return parseInt(this.$.plugin.callPluginMethodDeferred(null, 'setFontSize', fontSize),10)
  	},
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
  			this.$.plugin.callPluginMethodDeferred(null, 'setColor', i, this.currentColors[i][0], this.currentColors[i][1], this.currentColors[i][2])
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
			this.$.plugin.callPluginMethodDeferred(null, 'setKey', i, this.decodeEscape(this.currentKeys[i]))
  	},
  	
  	setActive: function(active) {
  		if (this.isReady) this.$.plugin.callPluginMethodDeferred(null, 'setActive', active);
  	},
  	
  	inject: function(command) {
  		this.$.plugin.callPluginMethodDeferred(null, 'inject', command)
  	},
  	
  	hasPassword: function(user) {
  		return parseInt(this.$.plugin.callPluginMethod('userHasPassword', user), 10)
  	},
  	
  	setPassword: function(user, password) {
  		this.$.plugin.callPluginMethod('userSetPassword', user, password)
  	},
  	
  	addToGroup: function(user, group) {
  		this.$.plugin.callPluginMethod('userAddToGroup', user, group)
  	},
  	
  	setupSU: function(enable) {
  		this.$.plugin.callPluginMethod('setupSU', enable)
  	}
  	
})