enyo.kind({

	name: 'Setup',
	kind: enyo.Hybrid,

	hybridReady: function() {
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

	setupNonRoot: function() {
		this.callPluginMethod('setupNonRoot')
	},

})