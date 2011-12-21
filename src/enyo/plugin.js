enyo.kind({
	
	name: 'Terminal',
	kind: enyo.Control,
		
	published: {
		width: 0,
		height: 0,
		vkb: null
	},
		
	events: {
		onPluginReady:''
	},
	
	published: {
		modes: {
			wrap: true,
			newline: false,
			reverse: false,
			charset: 0,
			charsetG0: 'US',
			charsetG1: 'US',
			origin: 0,
			insert: false,
			appkeys: false,
		}
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
			height: this.height
		})
	},
		
  	pluginReady: function(inSender, inResponse, inRequest) {
  		this.log('~~~~~ Terminal Plugin Ready ~~~~~')
  		//this.doPluginReady()
  	},
  	pluginConnected: function(inSender, inResponse, inRequest) {
  		this.log('~~~~~ Terminal Plugin Connected ~~~~~')
  	},
  	pluginDisconnected: function(inSender, inResponse, inRequest) {
  		this.log('~~~~~ Terminal Plugin Disconnected ~~~~~')
  	},
  	
  	write: function(buffer) {
  		this.$.plugin.callPluginMethod('write', buffer)
  	},
  	writeKeycode: function(keycode) {
  		this.$.plugin.callPluginMethod('write_keycode', keycode)
  	}
  	
})