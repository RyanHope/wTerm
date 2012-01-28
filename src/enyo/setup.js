enyo.kind({
	
	name: 'Setup',
	kind: enyo.Control,
		
	events: {
		onPluginReady:''
	},

	initComponents: function() {
		this.createComponent({
			name: 'plugin',
			kind: enyo.Hybrid,
			executable: 'setup',
			onPluginReady: 'pluginReady',
			onPluginConnected: 'pluginConnected',
			onPluginDisconnected: 'pluginDisconnected',
		})
	},
		
  	pluginReady: function(inSender, inResponse, inRequest) {
  		this.doPluginReady()
  	},
  	pluginConnected: function(inSender, inResponse, inRequest) {
  	},
  	pluginDisconnected: function(inSender, inResponse, inRequest) {
  		this.error('Setup Plugin Disconnected')
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
  	},
  	
  	setupNonRoot: function() {
  		this.$.plugin.callPluginMethod('setupNonRoot')
  	},
  	
})