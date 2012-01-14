enyo.kind({
	
	name: 'Terminal',
	kind: enyo.Control,
		
	published: {
		width: 0,
		height: 0,
		vkb: null,
		prefs: null,
		currentColors: [],
		currentKeys: [],
	},
		
	events: {
		onPluginReady:''
	},

	initComponents: function() {
		this.warn('Creating plugin')
		this.createComponent({
			name: 'plugin',
			kind: enyo.Hybrid,
			executable: 'wterm',
			onPluginReady: 'pluginReady',
			onPluginConnected: 'pluginConnected',
			onPluginDisconnected: 'pluginDisconnected',
			width: this.width,
			height: this.height,
			params: [this.prefs.get('fontSize').toString(10)]
		})
	},
		
  	pluginReady: function(inSender, inResponse, inRequest) {
  		this.log('~~~~~ Terminal Plugin Ready ~~~~~')
		this.setColors()
		this.setKeys()
  		//this.doPluginReady()
  	},
  	pluginConnected: function(inSender, inResponse, inRequest) {
  		this.log('~~~~~ Terminal Plugin Connected ~~~~~')
  	},
  	pluginDisconnected: function(inSender, inResponse, inRequest) {
  		this.log('~~~~~ Terminal Plugin Disconnected ~~~~~')
  	},

  	pushKeyEvent: function(type,state,sym,unicode) {
  		return parseInt(this.$.plugin.callPluginMethod('pushKeyEvent',type,state,sym,unicode))
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
  	
	cancelKeyRepeat: function() {
		this.$.plugin.callPluginMethod('cancelKeyRepeat')
	},
  	getDimensions: function() {
  		return enyo.json.parse(this.$.plugin.callPluginMethod('getDimensions'))
  	},
  	
  	getFontSize: function() {
  		return parseInt(this.$.plugin.callPluginMethod('getFontSize'),10)
  	},
  	
  	setFontSize: function(fontSize) {
  		return parseInt(this.$.plugin.callPluginMethod('setFontSize', fontSize),10)
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
  		var colorScheme = this.prefs.get('colorScheme')
		var colorSchemes = this.prefs.get('colorSchemes')
		this.currentColors = colorSchemes[colorScheme]
		if (colorScheme == 'Black on Random Light')
			this.currentColors[17] = this.currentColors[19] = this.hsvToRgb(Math.floor(Math.random()*256),34,247)
  		for (i in this.currentColors)
  			this.$.plugin.callPluginMethod('setColor', i, this.currentColors[i][0], this.currentColors[i][1], this.currentColors[i][2])
  	},
  	
  	decodeEscape: function(str) {
  		return str.replace(/\\x([0-9A-Fa-f]{2})/g, function() {
	        return String.fromCharCode(parseInt(arguments[1], 16));
	    });
  	},
  	
  	setKeys: function() {
  		var inputScheme = this.prefs.get('inputScheme')
		var inputSchemes = this.prefs.get('inputSchemes')
		this.currentKeys = inputSchemes[inputScheme]
		for (i in this.currentKeys)
			this.$.plugin.callPluginMethod('setKey', i, this.decodeEscape(this.currentKeys[i]))
  	}
  	
})