if (typeof PREFS === 'undefined')
	var PREFS = new Prefs();

enyo.kind({

	name: 'wTermLauncher',
	kind: enyo.Component,
	
	components: [
		{
			kind: 'ApplicationEvents',
			onApplicationRelaunch: 'onRelaunch',
		}
	],
		
	create: function () {
		this.inherited(arguments);
		enyo.application.m = this;
    },

	launch: function (relaunch) {
		if (enyo.windowParams.dockMode || enyo.windowParams.windowType === 'dockModeWindow')
			enyo.windows.openWindow('dock.html', 'dock', enyo.windowParams, {window:"dockMode"});
		else
			enyo.windows.openWindow('app.html', null, enyo.windowParams, null);
	},
	
	onRelaunch: function () {
		this.launch(true)
	},
	
})
