var dumpObject = function(inSender, inObject) {
	for (var i in inObject) {
		try {
			inSender.log(i, inObject[i])
		} catch (e) {
			inSender.error(i, e)
		}
	}
}

enyo.kind({

	name: 'wTermLauncher',
	kind: enyo.Component,

	initComponents: function() {
		this.inherited(arguments)
		this.createComponent({kind: 'ApplicationEvents', onApplicationRelaunch: 'onRelaunch'})
	},

	create: function() {
		this.inherited(arguments);
		enyo.application.m = this;
    },

	launch: function(relaunch, setup) {
		if (enyo.windowParams.resetFirstUse)
			enyo.application.p.set('firstUse', false)
		else if (enyo.windowParams.dockMode || enyo.windowParams.windowType === 'dockModeWindow')
			enyo.windows.openWindow('dock.html', 'dock', enyo.windowParams, {window:"dockMode"});
		else
			enyo.windows.openWindow('app.html', null, enyo.windowParams, null);
	},

	onRelaunch: function() {
		this.launch(true)
	}

})
